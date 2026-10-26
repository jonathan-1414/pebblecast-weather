#pragma once
#include "weather.h"
#include <pebble.h>

typedef struct {
  GFont time_font;
  GFont date_font;
  GFont plot_font;
  uint8_t battery_charge_percent;
  bool bluetooth_connected;
  // quiet time is on (checked on every redraw, i.e. at least once a minute)
  bool quiet_time_active;
  struct tm *tick_time;
  WeatherData weather_data;
  int8_t tomorrow_temp_max;
  int8_t tomorrow_weather_code;
  const char *sunrise_time;
  const char *sunset_time;
  // true once UV index and elevation have been received
  bool has_conditions;
  int uv_index;
  int elevation;
  // today's step count, -1 if not available
  int32_t steps;
} WatchfaceDrawContext;

void watchface_draw_date(GRect rect, const WatchfaceDrawContext *draw_ctx, GFont font, GTextAlignment alignment,
                         GContext *ctx);
void watchface_draw_time(GRect rect, const WatchfaceDrawContext *draw_ctx, GFont font, int y_offset,
                         GTextAlignment alignment, GContext *ctx);
void watchface_draw_bt_strike_through(GRect rect, GContext *ctx);
