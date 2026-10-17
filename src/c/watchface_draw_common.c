#include "watchface_draw_common.h"
#include "theme.h"

void watchface_draw_date(GRect rect, const WatchfaceDrawContext *draw_ctx, GFont font, GTextAlignment alignment,
                         GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static char date_buffer[16];
  static const char *monate[] = {"Jan", "Feb", "Mrz", "Apr", "Mai", "Jun",
                                 "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};
  snprintf(date_buffer, sizeof(date_buffer), "%d %s",
           draw_ctx->tick_time->tm_mday, monate[draw_ctx->tick_time->tm_mon]);
  graphics_context_set_text_color(ctx, theme->primary_text);
  graphics_draw_text(ctx, date_buffer, font, rect, GTextOverflowModeTrailingEllipsis, alignment, NULL);
}

void watchface_draw_time(GRect rect, const WatchfaceDrawContext *draw_ctx, GFont font, int y_offset,
                         GTextAlignment alignment, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static char time_buffer[12];
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H %M" : "%I %M", draw_ctx->tick_time);

  // Custom fonts need a small vertical adjustment to align visually within their row.
  rect.origin.y += y_offset;

  graphics_context_set_text_color(ctx, theme->primary_text);
  graphics_draw_text(ctx, time_buffer, font, rect, GTextOverflowModeTrailingEllipsis, alignment, NULL);
}

void watchface_draw_bt_strike_through(GRect rect, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static int x;
  static int y;
  static int half_h;
  static int half_w;
  x = rect.origin.x + rect.size.w / 2;
  y = rect.origin.y + rect.size.h / 2;
  half_h = rect.size.h / 2;
  half_w = rect.size.w / 2;

  graphics_context_set_stroke_color(ctx, theme->primary_text);
  graphics_context_set_stroke_width(ctx, 1);

  graphics_draw_line(ctx, GPoint(x, y - half_h), GPoint(x, y + half_h));
  graphics_draw_line(ctx, GPoint(x, y - half_h), GPoint(x + half_w, y - half_h / 2));
  graphics_draw_line(ctx, GPoint(x + half_w, y - half_h / 2), GPoint(x, y));
  graphics_draw_line(ctx, GPoint(x, y), GPoint(x + half_w, y + half_h / 2));
  graphics_draw_line(ctx, GPoint(x + half_w, y + half_h / 2), GPoint(x, y + half_h));
  graphics_draw_line(ctx, GPoint(x - half_w, y - half_h / 2), GPoint(x + half_w, y + half_h / 2));
  graphics_draw_line(ctx, GPoint(x - half_w, y + half_h / 2), GPoint(x + half_w, y - half_h / 2));

  // strike through
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(rect.origin.x, rect.origin.y),
                     GPoint(rect.origin.x + rect.size.w - 1, rect.origin.y + rect.size.h - 1));
}

void watchface_draw_weather_section(GRect weather_rect, const WatchfaceDrawContext *draw_ctx, GContext *ctx) {
  draw_weather(weather_rect, draw_ctx->weather_data, draw_ctx->plot_font, ctx);
}
