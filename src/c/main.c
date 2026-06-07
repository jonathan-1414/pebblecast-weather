#include "theme.h"
#include "watchface_compact.h"
#include "watchface_full.h"
#include "weather.h"
#include <pebble.h>

static Window *s_window;
static Layer *window_layer;

// fonts
static GFont s_time_font;
static GFont s_date_font;
static GFont s_plot_font;

static uint8_t battery_charge_percent;
static bool bluetooth_connected;

// current temps
static int temps_count;
static int8_t temps[24];
static int8_t hours[24];
static int8_t weather_codes[24];
static char city_name[32];
static int8_t today_temp_max;
static int8_t today_temp_min;

// tomorrow's summary
static int8_t tomorrow_temp_max;
static int8_t tomorrow_weather_code;

enum {
  PERSIST_TEMPS_COUNT = 0,
  PERSIST_TEMPS = 1,
  PERSIST_HOURS = 2,
  PERSIST_WEATHER_CODES = 3,
  PERSIST_TOMORROW_TEMP_MAX = 4,
  PERSIST_TOMORROW_WEATHER_CODE = 5,
  PERSIST_CITY_NAME = 6,
  PERSIST_TODAY_TEMP_MAX = 7,
  PERSIST_TODAY_TEMP_MIN = 8,
  PERSIST_THEME_ID = 9,
};

static void battery_change_handler(BatteryChargeState charge) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "battery_change_handle called");
  battery_charge_percent = charge.charge_percent;
  layer_mark_dirty(window_layer);
}

static void bluetooth_callback(bool connected) {
  bluetooth_connected = connected;
  if (!connected) {
    vibes_double_pulse();
  }
  layer_mark_dirty(window_layer);
}

static void log_memory() {
  int used_kb = heap_bytes_used() / 1024;
  int free_kb = heap_bytes_free() / 1024;
  int used_kb_after_comma = (int)(((heap_bytes_used() % 1024) / 1024.0) * 100);
  int free_kb_after_comma = (int)(((heap_bytes_free() % 1024) / 1024.0) * 100);
  static char buf[64];
  snprintf(buf, sizeof(buf), "Memory: %d.%d/%d.%d KB", used_kb, used_kb_after_comma, free_kb, free_kb_after_comma);
  APP_LOG(APP_LOG_LEVEL_DEBUG, buf);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  log_memory();

  layer_mark_dirty(window_layer);

  // request weather every 15 mins
  if (time(NULL) % (60 * 15) == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox received");
  Tuple *temp_data = dict_find(iterator, MESSAGE_KEY_TEMP_DATA);
  Tuple *hour_data = dict_find(iterator, MESSAGE_KEY_HOUR_DATA);
  Tuple *temp_count_data = dict_find(iterator, MESSAGE_KEY_TEMP_COUNT);
  Tuple *weather_code_data = dict_find(iterator, MESSAGE_KEY_WEATHER_CODE_DATA);
  Tuple *today_temp_max_data = dict_find(iterator, MESSAGE_KEY_TODAY_TEMP_MAX);
  Tuple *today_temp_min_data = dict_find(iterator, MESSAGE_KEY_TODAY_TEMP_MIN);
  Tuple *tomorrow_temp = dict_find(iterator, MESSAGE_KEY_TOMORROW_TEMP_MAX);
  Tuple *tomorrow_code = dict_find(iterator, MESSAGE_KEY_TOMORROW_WEATHER_CODE);
  Tuple *city_name_data = dict_find(iterator, MESSAGE_KEY_CITY_NAME);
  Tuple *theme_id_data = dict_find(iterator, MESSAGE_KEY_ThemeId);

  if (theme_id_data) {
    int theme_id =
        theme_id_data->type == TUPLE_CSTRING ? atoi(theme_id_data->value->cstring) : theme_id_data->value->int32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ThemeId received: %d", theme_id);
    app_theme_set_setting(app_theme_setting_from_int(theme_id));
    persist_write_int(PERSIST_THEME_ID, app_theme_get_setting());
    layer_mark_dirty(window_layer);
  }

  if (!temp_data || !hour_data || !temp_count_data || !weather_code_data) {
    return;
  }

  temps_count = temp_count_data->value->int8;
  if (temps_count > 24) {
    temps_count = 24;
  }

  memcpy(temps, temp_data->value->data, temps_count);
  memcpy(hours, hour_data->value->data, temps_count);
  memcpy(weather_codes, weather_code_data->value->data, temps_count);

  if (today_temp_max_data && today_temp_min_data) {
    today_temp_max = today_temp_max_data->value->int8;
    today_temp_min = today_temp_min_data->value->int8;
    persist_write_int(PERSIST_TODAY_TEMP_MAX, today_temp_max);
    persist_write_int(PERSIST_TODAY_TEMP_MIN, today_temp_min);
  }

  if (tomorrow_temp && tomorrow_code) {
    tomorrow_temp_max = tomorrow_temp->value->int8;
    tomorrow_weather_code = tomorrow_code->value->int8;
    persist_write_int(PERSIST_TOMORROW_TEMP_MAX, tomorrow_temp_max);
    persist_write_int(PERSIST_TOMORROW_WEATHER_CODE, tomorrow_weather_code);
  }

  if (city_name_data) {
    snprintf(city_name, sizeof(city_name), "%s", city_name_data->value->cstring);
    persist_write_string(PERSIST_CITY_NAME, city_name);
  }

  // persist weather data
  persist_write_int(PERSIST_TEMPS_COUNT, temps_count);
  persist_write_data(PERSIST_TEMPS, temps, temps_count);
  persist_write_data(PERSIST_HOURS, hours, temps_count);
  persist_write_data(PERSIST_WEATHER_CODES, weather_codes, temps_count);

  layer_mark_dirty(window_layer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void draw_watchface(struct Layer *layer, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  GRect bounds = layer_get_bounds(layer);
  time_t current_time = time(NULL);
  struct tm *tick_time = localtime(&current_time);

  graphics_context_set_fill_color(ctx, theme->background);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  static WatchfaceDrawContext draw_ctx;
  draw_ctx.time_font = s_time_font;
  draw_ctx.date_font = s_date_font;
  draw_ctx.plot_font = s_plot_font;
  draw_ctx.battery_charge_percent = battery_charge_percent;
  draw_ctx.bluetooth_connected = bluetooth_connected;
  draw_ctx.tick_time = tick_time;
  draw_ctx.weather_data =
      WeatherData(temps_count, temps, hours, weather_codes, city_name, today_temp_max, today_temp_min);
  draw_ctx.tomorrow_temp_max = tomorrow_temp_max;
  draw_ctx.tomorrow_weather_code = tomorrow_weather_code;

  if (watchface_is_compact_bounds(bounds)) {
    watchface_compact_draw(bounds, &draw_ctx, ctx);
  } else {
    watchface_full_draw(bounds, &draw_ctx, ctx);
  }
}

static void prv_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "prv_window_load called");
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  uint32_t time_font_resource =
      watchface_is_compact_bounds(bounds) ? RESOURCE_ID_FONT_RIGHTEOUS_TIME_32 : RESOURCE_ID_FONT_RIGHTEOUS_TIME_64;
  s_time_font = fonts_load_custom_font(resource_get_handle(time_font_resource));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RIGHTEOUS_DATE_12));
  s_plot_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_RIGHTEOUS_WEATHER_12));

  layer_set_update_proc(window_layer, draw_watchface);

  // load persisted weather data
  if (persist_exists(PERSIST_TEMPS_COUNT)) {
    temps_count = persist_read_int(PERSIST_TEMPS_COUNT);
    persist_read_data(PERSIST_TEMPS, temps, temps_count);
    persist_read_data(PERSIST_HOURS, hours, temps_count);
    persist_read_data(PERSIST_WEATHER_CODES, weather_codes, temps_count);
  }
  if (persist_exists(PERSIST_TOMORROW_TEMP_MAX)) {
    tomorrow_temp_max = persist_read_int(PERSIST_TOMORROW_TEMP_MAX);
    tomorrow_weather_code = persist_read_int(PERSIST_TOMORROW_WEATHER_CODE);
  }
  if (persist_exists(PERSIST_CITY_NAME)) {
    persist_read_string(PERSIST_CITY_NAME, city_name, sizeof(city_name));
  }
  if (persist_exists(PERSIST_TODAY_TEMP_MAX) && persist_exists(PERSIST_TODAY_TEMP_MIN)) {
    today_temp_max = persist_read_int(PERSIST_TODAY_TEMP_MAX);
    today_temp_min = persist_read_int(PERSIST_TODAY_TEMP_MIN);
  }
  if (persist_exists(PERSIST_THEME_ID)) {
    app_theme_set_setting(app_theme_setting_from_int(persist_read_int(PERSIST_THEME_ID)));
  } else {
    app_theme_set_setting(APP_THEME_SETTING_DARK);
  }

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  connection_service_subscribe((ConnectionHandlers){.pebble_app_connection_handler = bluetooth_callback});
  bluetooth_connected = connection_service_peek_pebble_app_connection();

  battery_state_service_subscribe(battery_change_handler);
  battery_change_handler(battery_state_service_peek());
}

static void prv_window_unload(Window *window) {
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_plot_font);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
                                           .load = prv_window_load,
                                           .unload = prv_window_unload,
                                       });
  const bool animated = false;
  window_stack_push(s_window, animated);

  // AppMessage callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_outbox_failed(outbox_failed_callback);

  const int inbox_size = 256;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void prv_deinit(void) { window_destroy(s_window); }

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
