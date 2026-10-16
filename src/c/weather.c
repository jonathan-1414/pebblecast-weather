#include "weather.h"
#include "layout.h"
#include "weather_icons.h"
#include <pebble.h>

// The temperature axis always covers at least this many degrees, so that small changes do not look dramatic.
#define MIN_TEMP_SPAN 6

int compare(const void *a, const void *b) { return *(int8_t *)a - *(int8_t *)b; }

static const char *weather_code_description(int8_t code) {
  if (code <= 1)
    return "Sonne";
  if (code <= 3)
    return "Wolken";
  if (code <= 48)
    return "Nebel";
  if (code <= 57)
    return "Nieselregen";
  if (code <= 67)
    return "Regen";
  if (code <= 77)
    return "Schnee";
  if (code <= 82)
    return "Schauer";
  if (code <= 86)
    return "Schnee";
  return "Gewitter";
}

void draw_weather_summary(GRect rect, const char *label, int8_t weather_code, int8_t temp_max, GFont font,
                          GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static char summary[32];
  snprintf(summary, sizeof(summary), "%s: %s, %d\u00B0", label, weather_code_description(weather_code), (int)temp_max);
  static GRect inner_rect;
  inner_rect = get_effective_rect(LayoutBox(rect, 4));
  graphics_context_set_text_color(ctx, theme->plot_label);
  graphics_draw_text(ctx, summary, font, inner_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// Returns the worst (highest) weather code in a range of hours
static int8_t worst_weather_code(int start, int count, int8_t *weather_codes, int temps_count) {
  int8_t max_code = weather_codes[start];
  for (int i = start + 1; i < start + count && i < temps_count; i++) {
    if (weather_codes[i] > max_code)
      max_code = weather_codes[i];
  }
  return max_code;
}

static bool is_night_hour(int8_t hour) { return hour >= 21 || hour < 6; }

// Draws the appropriate weather icon for a WMO code
// hour is used to determine day/night for clear and partly-cloudy icons
static void draw_weather_icon(GContext *ctx, int cx, int cy, int8_t code, int8_t hour) {
  if (code >= 95) {
    draw_icon_thunderstorm(ctx, cx, cy);
  } else if (code >= 80) {
    draw_icon_heavy_rain(ctx, cx, cy);
  } else if (code >= 71) {
    draw_icon_snowflake(ctx, cx, cy);
  } else if (code >= 61) {
    draw_icon_heavy_rain(ctx, cx, cy);
  } else if (code >= 51) {
    draw_icon_light_rain(ctx, cx, cy);
  } else if (code >= 45) {
    draw_icon_fog(ctx, cx, cy);
  } else if (code == 3) {
    draw_icon_cloud(ctx, cx, cy + 3);
  } else if (code >= 1 && is_night_hour(hour)) {
    draw_icon_partly_cloudy_night(ctx, cx, cy);
  } else if (code >= 1) {
    draw_icon_partly_cloudy_day(ctx, cx, cy);
  } else if (is_night_hour(hour)) {
    draw_icon_moon(ctx, cx, cy);
  } else {
    draw_icon_sun(ctx, cx, cy);
  }
}

void draw_arrow_up(GContext *ctx, GRect rect) {
  static GPoint top;
  static GPoint bottom;
  static GPoint left;
  static GPoint right;
  top = GPoint(rect.origin.x, rect.origin.y + 2);
  bottom = GPoint(rect.origin.x, rect.origin.y + rect.size.h - 8);
  left = GPoint(top.x - 4, top.y + 4);
  right = GPoint(top.x + 4, top.y + 4);
  graphics_draw_line(ctx, top, bottom);
  graphics_draw_line(ctx, top, left);
  graphics_draw_line(ctx, top, right);
}

void draw_arrow_down(GContext *ctx, GRect rect) {
  static GPoint top;
  static GPoint bottom;
  static GPoint left;
  static GPoint right;
  top = GPoint(rect.origin.x, rect.origin.y + 2);
  bottom = GPoint(rect.origin.x, rect.origin.y + rect.size.h - 8);
  left = GPoint(top.x - 4, bottom.y - 4);
  right = GPoint(top.x + 4, bottom.y - 4);
  graphics_draw_line(ctx, top, bottom);
  graphics_draw_line(ctx, bottom, left);
  graphics_draw_line(ctx, bottom, right);
}

static void draw_top_row(GRect rect, int min_temp, int max_temp, const char *city_name, GFont font, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  LayoutBox row = LayoutBox(rect, 0);
  static GRect cols[5];
  static EntryConfig col_configs[] = {{8}, {25}, {8}, {25}, {0}};
  split_into_columns(row, col_configs, cols, 5);
  static GRect arrow_up_rect;
  static GRect max_temp_rect;
  static GRect arrow_down_rect;
  static GRect min_temp_rect;
  static GRect city_rect;
  arrow_up_rect = cols[0];
  max_temp_rect = cols[1];
  arrow_down_rect = cols[2];
  min_temp_rect = cols[3];
  city_rect = cols[4];

  graphics_context_set_stroke_color(ctx, theme->primary_text);
  graphics_context_set_stroke_width(ctx, 2);

  draw_arrow_up(ctx, arrow_up_rect);
  draw_arrow_down(ctx, arrow_down_rect);

  graphics_context_set_text_color(ctx, theme->plot_label);

  static char max_temp_text_buf[8];
  snprintf(max_temp_text_buf, 8, "%d", max_temp);
  graphics_draw_text(ctx, max_temp_text_buf, font, max_temp_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft,
                     NULL);

  static char min_temp_text_buf[8];
  snprintf(min_temp_text_buf, 8, "%d", min_temp);
  graphics_draw_text(ctx, min_temp_text_buf, font, min_temp_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft,
                     NULL);

  graphics_draw_text(ctx, city_name, font, city_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
}

void draw_weather(GRect rect, WeatherData data, GFont font, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "draw_weather called");

  if (data.temps_count < 2)
    return;

  // padding of 10 px, except for a smaller gap between the divider line and the top row
  LayoutBox weather_box = LayoutBox(GRect(rect.origin.x, rect.origin.y - 6, rect.size.w, rect.size.h + 6), 10);
  // 1st row: top row for min/max values
  // 2nd row: serves as a spacer to make room for the temperature label that is above the temperature point
  // 3rd row: temp plot
  // 4th row: weather code icons (e.g. cloud, sun)
  // 5th row: x axis with labels
  static EntryConfig row_configs[] = {{20}, {10}, {0}, {20}, {20}};
  static GRect rows[5];
  split_into_rows(weather_box, row_configs, rows, 5);

  // top row showing max/min temps
  static int8_t sorted_temps[24];
  memcpy(sorted_temps, data.temps, data.temps_count);
  qsort(sorted_temps, data.temps_count, sizeof(int8_t), compare);
  int window_min_temp = sorted_temps[0];
  int window_max_temp = sorted_temps[data.temps_count - 1];
  draw_top_row(rows[0], data.today_temp_min, data.today_temp_max, data.city_name, font, ctx);

  // row containing the plot (only temp plot, no axis, no weather code icons)
  static GRect plot_rect;
  plot_rect = rows[2];
  int x_tick_size = (int)((plot_rect.size.w) / (double)(data.temps_count - 1));

  // widen the axis around the middle if the actual range is smaller than MIN_TEMP_SPAN
  double axis_min_temp = window_min_temp;
  double temp_range = window_max_temp - window_min_temp;
  if (temp_range < MIN_TEMP_SPAN) {
    axis_min_temp = (window_min_temp + window_max_temp) / 2.0 - MIN_TEMP_SPAN / 2.0;
    temp_range = MIN_TEMP_SPAN;
  }
  int y_pixel_range = plot_rect.size.h;

  static GPoint temp_gpoints[24];

  // collect temp points
  for (int i = 0; i < data.temps_count; i++) {
    // y is treated from top when drawing, so we must invert it
    int y = y_pixel_range - (int)(((data.temps[i] - axis_min_temp) / temp_range) * y_pixel_range + 0.5);
    temp_gpoints[i] = GPoint(plot_rect.origin.x + i * x_tick_size, plot_rect.origin.y + y);
  }

  static GRect x_axis_rect;
  x_axis_rect = rows[4];
  int y_position_of_x_axis_line = x_axis_rect.origin.y;

  // draw weather icons (one per 3-hour group, using worst weather code)
  for (int i = 0; i < data.temps_count; i += 3) {
    // only draw if a full 3 hour group is ahead
    if (i + 2 < data.temps_count) {
      // draw centered in the upcoming 3 hour group
      int icon_cx = (temp_gpoints[i + 2].x + temp_gpoints[i + 1].x) / 2.0;
      int icon_cy = rows[3].origin.y + (rows[3].size.h / 2.0);
      draw_weather_icon(ctx, icon_cx, icon_cy, worst_weather_code(i, 3, data.weather_codes, data.temps_count),
                        data.hours[i + 1]);
    }
  }

  // draw temp line
  graphics_context_set_stroke_color(ctx, theme->plot_line);
  graphics_context_set_stroke_width(ctx, 2);
  for (int i = 0; i < data.temps_count - 1; i++) {
    graphics_draw_line(ctx, temp_gpoints[i], temp_gpoints[i + 1]);
  }

  // draw temps
  for (int i = 0; i < data.temps_count; i++) {

    // draw x labels
    int x_current_label = temp_gpoints[i].x;

    if (i % 3 == 0) {
      graphics_context_set_fill_color(ctx, theme->plot_line);
      graphics_fill_circle(ctx, temp_gpoints[i], 3);

      // draw hour tick marks, longer for every third hour
      graphics_context_set_stroke_color(ctx, theme->plot_axis);
      graphics_context_set_stroke_width(ctx, 1);
      graphics_draw_line(ctx, GPoint(x_current_label, y_position_of_x_axis_line),
                         GPoint(x_current_label, y_position_of_x_axis_line + 6));

      // draw hour labels
      graphics_context_set_text_color(ctx, theme->plot_label);
      static char hour_number_buf[6];
      snprintf(hour_number_buf, sizeof(hour_number_buf), "%02d", data.hours[i]);
      graphics_draw_text(ctx, hour_number_buf, font,
                         GRect(temp_gpoints[i].x - 7, y_position_of_x_axis_line + 6, 18, 18),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      // draw temp numbers
      static char temp_number_buf[6];
      snprintf(temp_number_buf, sizeof(temp_number_buf), "%d", data.temps[i]);
      graphics_draw_text(ctx, temp_number_buf, font, GRect(temp_gpoints[i].x - 7, temp_gpoints[i].y - 18, 18, 18),
                         GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    } else {
      // draw hour tick marks, shorter for all other hours
      graphics_context_set_stroke_color(ctx, theme->plot_axis);
      graphics_context_set_stroke_width(ctx, 1);
      graphics_draw_line(ctx, GPoint(x_current_label, y_position_of_x_axis_line),
                         GPoint(x_current_label, y_position_of_x_axis_line + 3));
    }
  }

  // x axis line
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_stroke_color(ctx, theme->plot_axis);
  graphics_draw_line(ctx, GPoint(x_axis_rect.origin.x, y_position_of_x_axis_line),
                     GPoint(x_axis_rect.origin.x + x_tick_size * (data.temps_count - 1), y_position_of_x_axis_line));
}
