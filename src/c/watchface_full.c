#include "watchface_full.h"
#include "battery.h"
#include "layout.h"
#include "theme.h"

#define INFO_LINE_HEIGHT 15
#define INFO_ICON_WIDTH 11
#define TIME_TEXT_HEIGHT 52

static void draw_text_line(const char *text, GRect rect, GFont font, GTextAlignment alignment, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_text_color(ctx, theme->primary_text);
  graphics_draw_text(ctx, text, font, rect, GTextOverflowModeTrailingEllipsis, alignment, NULL);
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

// one info line, vertically centered in its row
static GRect info_line_rect(GRect rect) {
  return GRect(rect.origin.x, rect.origin.y + (rect.size.h - INFO_LINE_HEIGHT) / 2, rect.size.w, INFO_LINE_HEIGHT);
}

static GRect info_text_rect(GRect line_rect) {
  // custom font renders a few pixels below the rect origin
  return GRect(line_rect.origin.x + INFO_ICON_WIDTH, line_rect.origin.y - 2, line_rect.size.w - INFO_ICON_WIDTH,
               line_rect.size.h + 2);
}

// minutes from now until the next occurrence of an "HH:MM" time
static int minutes_until(const char *hhmm, const struct tm *now) {
  int event = ((hhmm[0] - '0') * 10 + (hhmm[1] - '0')) * 60 + (hhmm[3] - '0') * 10 + (hhmm[4] - '0');
  int current = now->tm_hour * 60 + now->tm_min;
  return (event - current + 24 * 60) % (24 * 60);
}

// only the next sun event: sunset during the day, sunrise during the night
static void draw_next_sun_event(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  if (draw_ctx->sunrise_time[0] == '\0' || draw_ctx->sunset_time[0] == '\0') {
    return;
  }
  bool rise =
      minutes_until(draw_ctx->sunrise_time, draw_ctx->tick_time) < minutes_until(draw_ctx->sunset_time, draw_ctx->tick_time);
  const char *text = rise ? draw_ctx->sunrise_time : draw_ctx->sunset_time;
  GRect line = info_line_rect(rect);
  // center icon and text together across the row
  GSize text_size = graphics_text_layout_get_content_size(text, draw_ctx->plot_font, info_text_rect(line),
                                                          GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
  int width = INFO_ICON_WIDTH + text_size.w;
  GRect centered = GRect(line.origin.x + (line.size.w - width) / 2, line.origin.y, width, line.size.h);
  // move the icon up so its horizon line sits on the text's bottom row
  draw_sun_icon(GRect(centered.origin.x, centered.origin.y - 3, centered.size.w, centered.size.h), rise, ctx);
  draw_text_line(text, info_text_rect(centered), draw_ctx->plot_font, GTextAlignmentLeft, ctx);
}

static void draw_uv_index(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  if (!draw_ctx->has_conditions) {
    return;
  }
  static char uv_buf[12];
  snprintf(uv_buf, sizeof(uv_buf), "UV %d", draw_ctx->uv_index);
  draw_text_line(uv_buf, rect, draw_ctx->plot_font, GTextAlignmentRight, ctx);
}

static void draw_steps(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  if (draw_ctx->steps < 0) {
    return;
  }
  static char steps_buf[12];
  snprintf(steps_buf, sizeof(steps_buf), "%ld", (long)draw_ctx->steps);
  GRect line = info_line_rect(rect);
  draw_steps_icon(line, ctx);
  draw_text_line(steps_buf, info_text_rect(line), draw_ctx->plot_font, GTextAlignmentLeft, ctx);
}

static void draw_elevation(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  if (!draw_ctx->has_conditions) {
    return;
  }
  static char elevation_buf[12];
  snprintf(elevation_buf, sizeof(elevation_buf), "%d", draw_ctx->elevation);
  GRect line = info_line_rect(rect);
  GRect text_rect = info_text_rect(line);
  draw_text_line(elevation_buf, text_rect, draw_ctx->plot_font, GTextAlignmentRight, ctx);
  // the icon sits directly left of the right-aligned text
  GSize text_size = graphics_text_layout_get_content_size(elevation_buf, draw_ctx->plot_font, text_rect,
                                                          GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);
  draw_mountain_icon(GRect(line.origin.x + line.size.w - text_size.w - INFO_ICON_WIDTH, line.origin.y,
                           INFO_ICON_WIDTH, line.size.h),
                     ctx);
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

  // the corner shows the UV index, or the bluetooth icon while disconnected
  if (draw_ctx->bluetooth_connected) {
    draw_uv_index(cols[2], draw_ctx, ctx);
  } else {
    static GRect bluetooth_rect;
    bluetooth_rect = GRect(cols[2].origin.x + cols[2].size.w - 14, cols[2].origin.y, 14, 14);
    watchface_draw_bt_strike_through(bluetooth_rect, ctx);
  }
}

static void watchface_full_draw_time_row(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  // the row is lower than the font's line height, which would stop the time from being drawn
  watchface_draw_time(GRect(rect.origin.x, rect.origin.y, rect.size.w, TIME_TEXT_HEIGHT), draw_ctx,
                      draw_ctx->time_font, -6, GTextAlignmentCenter, ctx);
}

static void watchface_full_draw_info_row(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  static LayoutBox info_row;
  info_row = LayoutBox(rect, 0);
  static EntryConfig col_configs[] = {{52}, {0}, {52}};
  static GRect cols[3];
  split_into_columns(info_row, col_configs, cols, 3);

  draw_steps(cols[0], draw_ctx, ctx);
  draw_next_sun_event(rect, draw_ctx, ctx);
  draw_elevation(cols[2], draw_ctx, ctx);
}

void watchface_full_draw(GRect bounds, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static LayoutBox root;
  root = LayoutBox(bounds, 2);
  // row heights leave about 10 px between the rows of the top section and tomorrow's summary;
  // the 4th row is an empty gap, so the summary sits directly above the divider
  static GRect rows[6];
  static EntryConfig row_configs[] = {{33}, {15}, {39}, {4}, {25}, {0}};
  split_into_rows(root, row_configs, rows, 6);
  // dbg_draw_outlines(rows, 6, ctx);
  static GRect top_row_rect;
  static GRect time_rect;
  static GRect info_rect;
  static GRect summary_rect;
  static GRect weather_rect;
  top_row_rect = rows[0];
  info_rect = rows[1];
  time_rect = rows[2];
  summary_rect = rows[4];
  weather_rect = rows[5];

  // top row for battery percentage, date, UV index or bluetooth
  watchface_full_draw_top_row(top_row_rect, draw_ctx, ctx);

  // steps, next sun event and elevation
  watchface_full_draw_info_row(info_rect, draw_ctx, ctx);

  // time
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
