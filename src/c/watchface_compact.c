#include "watchface_compact.h"
#include "battery.h"
#include "layout.h"
#include "theme.h"

static void watchface_compact_draw_header(GRect rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  const AppTheme *theme = app_theme_current();

  // first split into two columns: left for date, battery etc.; right for large time
  static LayoutBox header;
  header = LayoutBox(rect, 0);
  static GRect cols[2];
  static EntryConfig col_configs[] = {{42}, {0}};
  split_into_columns(header, col_configs, cols, 2);
  static GRect left_rect;
  static GRect right_rect;
  left_rect = cols[0];
  right_rect = cols[1];

  // display battery, date, bluetooth in left column
  static GRect meta_rows[2];
  static EntryConfig meta_row_configs[] = {{16}, {0}};
  split_into_rows(LayoutBox(left_rect, 0), meta_row_configs, meta_rows, 2);
  static GRect battery_rect;
  static GRect date_rect;
  battery_rect = meta_rows[0];
  date_rect = meta_rows[1];

  // battery
  draw_battery(draw_ctx->battery_charge_percent,
               GRect(battery_rect.origin.x, battery_rect.origin.y, 18, 10),
               ctx);

  // bluetooth
  if (!draw_ctx->bluetooth_connected) {
    static GRect bluetooth_rect;
    bluetooth_rect = GRect(battery_rect.origin.x + 30, battery_rect.origin.y + 2, 12, 12);
    watchface_draw_bt_strike_through(bluetooth_rect, ctx);
  }

  // date
  graphics_context_set_text_color(ctx, theme->primary_text);
  static char date_buf[12];
  strftime(date_buf, sizeof(date_buf), "%a %d", draw_ctx->tick_time);
  graphics_draw_text(ctx, date_buf, draw_ctx->date_font, date_rect, GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft, NULL);

  // display time in right column
  watchface_draw_time(right_rect, draw_ctx, draw_ctx->time_font, -4, GTextAlignmentCenter, ctx);
}

void watchface_compact_draw(GRect bounds, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  static LayoutBox root;

  // compact watchface contains three main rows
  // 1st row: date, time, battery, bluetooth
  // 2nd row: tomorrow's weather summary
  // 3rd row: weather plot
  root = LayoutBox(bounds, 2);
  static GRect rows[3];
  static EntryConfig row_configs[] = {{32}, {15}, {0}};
  split_into_rows(root, row_configs, rows, 3);

  watchface_compact_draw_header(rows[0], draw_ctx, ctx);
  draw_weather_summary(rows[1], "Tom.", draw_ctx->tomorrow_weather_code, draw_ctx->tomorrow_temp_max,
                       draw_ctx->plot_font, ctx);
  watchface_draw_weather_section(rows[2], draw_ctx, ctx);
}
