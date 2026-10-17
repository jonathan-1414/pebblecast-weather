#include "watchface_full.h"
#include "battery.h"
#include "layout.h"
#include "theme.h"

#define INFO_LINE_HEIGHT 15
#define INFO_ICON_WIDTH 11

static void draw_text_line(const char *text, GRect rect, GFont font, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_text_color(ctx, theme->primary_text);
  graphics_draw_text(ctx, text, font, rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

// small sun on a horizon line with a chevron above it (up = sunrise, down = sunset)
static void draw_sun_icon(GRect rect, bool rise, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  int cx = rect.origin.x + 5;
  int horizon_y = rect.origin.y + 12;
  graphics_context_set_fill_color(ctx, theme->weather_sun);
  graphics_fill_circle(ctx, GPoint(cx, horizon_y), 4);
  // cut off the lower half below the horizon
  graphics_context_set_fill_color(ctx, theme->background);
  graphics_fill_rect(ctx, GRect(cx - 5, horizon_y + 1, 11, 4), 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, theme->primary_text);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(cx - 5, horizon_y), GPoint(cx + 5, horizon_y));
  int tip_y = rise ? rect.origin.y + 2 : rect.origin.y + 5;
  int base_y = rise ? rect.origin.y + 5 : rect.origin.y + 2;
  graphics_draw_line(ctx, GPoint(cx - 3, base_y), GPoint(cx, tip_y));
  graphics_draw_line(ctx, GPoint(cx, tip_y), GPoint(cx + 3, base_y));
}

static void draw_drop_icon(GRect rect, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  int cx = rect.origin.x + 5;
  int cy = rect.origin.y + 10;
  graphics_context_set_fill_color(ctx, theme->weather_rain);
  graphics_context_set_stroke_color(ctx, theme->weather_rain);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_fill_circle(ctx, GPoint(cx, cy), 3);
  for (int dy = 1; dy <= 4; dy++) {
    int half_w = (4 - dy) * 3 / 4;
    graphics_draw_line(ctx, GPoint(cx - half_w, cy - 2 - dy), GPoint(cx + half_w, cy - 2 - dy));
  }
}

static void draw_mountain_icon(GRect rect, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  int x = rect.origin.x;
  int base_y = rect.origin.y + 12;
  graphics_context_set_stroke_color(ctx, theme->primary_text);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(x, base_y), GPoint(x + 4, base_y - 7));
  graphics_draw_line(ctx, GPoint(x + 4, base_y - 7), GPoint(x + 6, base_y - 4));
  graphics_draw_line(ctx, GPoint(x + 6, base_y - 4), GPoint(x + 7, base_y - 5));
  graphics_draw_line(ctx, GPoint(x + 7, base_y - 5), GPoint(x + 10, base_y));
  graphics_draw_line(ctx, GPoint(x, base_y), GPoint(x + 10, base_y));
}

// two shoe soles
static void draw_steps_icon(GRect rect, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  int x = rect.origin.x;
  int y = rect.origin.y + 3;
  graphics_context_set_fill_color(ctx, theme->primary_text);
  graphics_fill_rect(ctx, GRect(x, y + 3, 4, 6), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect(x, y + 10, 4, 2), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(x + 6, y, 4, 6), 1, GCornersAll);
  graphics_fill_rect(ctx, GRect(x + 6, y + 7, 4, 2), 0, GCornerNone);
}

static GRect info_line_rect(GRect column, int line) {
  int top = column.origin.y + (column.size.h - 3 * INFO_LINE_HEIGHT) / 2;
  return GRect(column.origin.x, top + line * INFO_LINE_HEIGHT, column.size.w, INFO_LINE_HEIGHT);
}

static GRect info_text_rect(GRect line_rect) {
  // custom font renders a few pixels below the rect origin
  return GRect(line_rect.origin.x + INFO_ICON_WIDTH, line_rect.origin.y - 2, line_rect.size.w - INFO_ICON_WIDTH,
               line_rect.size.h + 2);
}

// left of the time: sunrise, sunset, steps
static void draw_left_info(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  GFont font = draw_ctx->plot_font;
  if (draw_ctx->sunrise_time[0] != '\0') {
    GRect line = info_line_rect(rect, 0);
    draw_sun_icon(line, true, ctx);
    draw_text_line(draw_ctx->sunrise_time, info_text_rect(line), font, ctx);
    line = info_line_rect(rect, 1);
    draw_sun_icon(line, false, ctx);
    draw_text_line(draw_ctx->sunset_time, info_text_rect(line), font, ctx);
  }
  if (draw_ctx->steps >= 0) {
    static char steps_buf[12];
    snprintf(steps_buf, sizeof(steps_buf), "%ld", (long)draw_ctx->steps);
    GRect line = info_line_rect(rect, 2);
    draw_steps_icon(line, ctx);
    draw_text_line(steps_buf, info_text_rect(line), font, ctx);
  }
}

// right of the time: UV index, humidity, elevation
static void draw_right_info(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  if (!draw_ctx->has_conditions) {
    return;
  }
  GFont font = draw_ctx->plot_font;
  static char uv_buf[12];
  static char humidity_buf[12];
  static char elevation_buf[12];
  snprintf(uv_buf, sizeof(uv_buf), "UV %d", draw_ctx->uv_index);
  snprintf(humidity_buf, sizeof(humidity_buf), "%d%%", draw_ctx->humidity);
  snprintf(elevation_buf, sizeof(elevation_buf), "%d", draw_ctx->elevation);

  GRect line = info_line_rect(rect, 0);
  draw_text_line(uv_buf, GRect(line.origin.x, line.origin.y - 2, line.size.w, line.size.h + 2), font, ctx);
  line = info_line_rect(rect, 1);
  draw_drop_icon(line, ctx);
  draw_text_line(humidity_buf, info_text_rect(line), font, ctx);
  line = info_line_rect(rect, 2);
  draw_mountain_icon(line, ctx);
  draw_text_line(elevation_buf, info_text_rect(line), font, ctx);
}

static void watchface_full_draw_top_row(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static LayoutBox top_row;
  top_row = LayoutBox(rect, 0);
  static EntryConfig col_configs[] = {{40}, {0}, {40}};
  static GRect cols[3];
  split_into_columns(top_row, col_configs, cols, 3);

  // battery as percentage
  static char battery_buf[8];
  snprintf(battery_buf, sizeof(battery_buf), "%d%%", draw_ctx->battery_charge_percent);
  graphics_context_set_text_color(ctx, draw_ctx->battery_charge_percent <= 20 ? theme->battery_low
                                                                                 : theme->primary_text);
  graphics_draw_text(ctx, battery_buf, draw_ctx->plot_font, cols[0], GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, NULL);

  // draw date centered across the whole row, not just the middle column
  watchface_draw_date(rect, draw_ctx, draw_ctx->date_font, GTextAlignmentCenter, ctx);

  if (!draw_ctx->bluetooth_connected) {
    static GRect bluetooth_rect;
    bluetooth_rect = GRect(cols[2].origin.x + cols[2].size.w - 14, cols[2].origin.y, 14, 14);
    watchface_draw_bt_strike_through(bluetooth_rect, ctx);
  }
}

static void watchface_full_draw_time_row(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  static LayoutBox time_row;
  time_row = LayoutBox(rect, 0);
  static EntryConfig col_configs[] = {{52}, {0}, {52}};
  static GRect cols[3];
  split_into_columns(time_row, col_configs, cols, 3);

  draw_left_info(cols[0], draw_ctx, ctx);
  watchface_draw_time(rect, draw_ctx, draw_ctx->time_font, -6, GTextAlignmentCenter, ctx);
  draw_right_info(cols[2], draw_ctx, ctx);
}

void watchface_full_draw(GRect bounds, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static LayoutBox root;
  root = LayoutBox(bounds, 2);
  // 3rd row is an empty gap that makes the weather section 15 px smaller,
  // so tomorrow's summary sits directly above the divider
  static GRect rows[5];
  static EntryConfig row_configs[] = {{20}, {52}, {15}, {25}, {0}};
  split_into_rows(root, row_configs, rows, 5);
  // dbg_draw_outlines(rows, 5, ctx);
  static GRect top_row_rect;
  static GRect time_rect;
  static GRect summary_rect;
  static GRect weather_rect;
  top_row_rect = rows[0];
  time_rect = rows[1];
  summary_rect = rows[3];
  weather_rect = rows[4];

  // top row for battery percentage, date, bluetooth
  watchface_full_draw_top_row(top_row_rect, draw_ctx, ctx);

  // time, framed by sun times / steps and UV / humidity / elevation
  watchface_full_draw_time_row(time_rect, draw_ctx, ctx);

  // weather summary for tomorrow
  draw_weather_summary(summary_rect, "Morgen", draw_ctx->tomorrow_weather_code, draw_ctx->tomorrow_temp_max,
                       draw_ctx->plot_font, ctx);

  // draw divider
  graphics_context_set_stroke_color(ctx, theme->divider);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(weather_rect.origin.x + 20, weather_rect.origin.y),
                     GPoint(weather_rect.origin.x + weather_rect.size.w - 20, weather_rect.origin.y));

  // weather plot
  watchface_draw_weather_section(weather_rect, draw_ctx, ctx);
}
