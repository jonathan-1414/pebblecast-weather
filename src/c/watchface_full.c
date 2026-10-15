#include "watchface_full.h"
#include "battery.h"
#include "layout.h"
#include "theme.h"

static void watchface_full_draw_top_row(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  static LayoutBox top_row;
  top_row = LayoutBox(rect, 0);
  static EntryConfig col_configs[] = {{30}, {0}, {54}};
  static GRect cols[3];
  split_into_columns(top_row, col_configs, cols, 3);

  // draw_battery(draw_ctx->battery_charge_percent, cols[0], ctx);
  draw_battery(draw_ctx->battery_charge_percent, GRect(0, 2, 26, 12), ctx);
  // watchface_draw_date(cols[1], draw_ctx, draw_ctx->date_font, GTextAlignmentCenter, ctx);
  // draw date centered across the whole row, not just the middle column
  watchface_draw_date(rect, draw_ctx, draw_ctx->date_font, GTextAlignmentCenter, ctx);

  if (!draw_ctx->bluetooth_connected) {
    static GRect bluetooth_rect;
    bluetooth_rect = GRect(cols[2].origin.x + 8, cols[2].origin.y, 14, 14);
    watchface_draw_bt_strike_through(bluetooth_rect, ctx);
  } else {
      // next sunrise or sunset
      watchface_draw_sun_event(cols[2], draw_ctx, draw_ctx->plot_font, ctx);
  }
}

void watchface_full_draw(GRect bounds, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static LayoutBox root;
  root = LayoutBox(bounds, 2);
  static GRect rows[4];
  static EntryConfig row_configs[] = {{20}, {52}, {25}, {0}};
  split_into_rows(root, row_configs, rows, 4);
  // dbg_draw_outlines(rows, 4, ctx);
  static GRect top_row_rect;
  static GRect time_rect;
  static GRect summary_rect;
  static GRect weather_rect;
  top_row_rect = rows[0];
  time_rect = rows[1];
  summary_rect = rows[2];
  weather_rect = rows[3];

  // top row for battery, date, bluetooth
  watchface_full_draw_top_row(top_row_rect, draw_ctx, ctx);

  // time
  watchface_draw_time(time_rect, draw_ctx, draw_ctx->time_font, -10, GTextAlignmentCenter, ctx);

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
