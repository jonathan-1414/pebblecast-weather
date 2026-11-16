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

// each digit sits centered in its own box, so the time does not shift when a digit changes
#define TIME_DIGIT_WIDTH 32
#define TIME_DIGIT_GAP 14

void watchface_draw_time(GRect rect, const WatchfaceDrawContext *draw_ctx, GFont font, int y_offset, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  static char time_buffer[8];
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H%M" : "%I%M", draw_ctx->tick_time);

  // Custom fonts need a small vertical adjustment to align visually within their row.
  rect.origin.y += y_offset;

  graphics_context_set_text_color(ctx, theme->primary_text);
  const int total_width = 4 * TIME_DIGIT_WIDTH + TIME_DIGIT_GAP;
  int x = rect.origin.x + (rect.size.w - total_width) / 2;
  for (int i = 0; i < 4; i++) {
    if (i == 2) {
      x += TIME_DIGIT_GAP;
    }
    char digit[2] = {time_buffer[i], '\0'};
    // the text rect is wider than the box, because the widest digits are slightly wider
    // than TIME_DIGIT_WIDTH and would otherwise be cut off
    GRect digit_rect = GRect(x - 10, rect.origin.y, TIME_DIGIT_WIDTH + 20, rect.size.h);
    graphics_draw_text(ctx, digit, font, digit_rect, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    x += TIME_DIGIT_WIDTH;
  }
}

void watchface_draw_disconnected(GRect rect, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  const int x = rect.origin.x;
  const int y = rect.origin.y;

  // inverted icon: filled square in text colour, phone and strike cut out in background colour
  graphics_context_set_fill_color(ctx, theme->primary_text);
  graphics_fill_rect(ctx, GRect(x, y, 14, 14), 0, GCornerNone);

  graphics_context_set_stroke_width(ctx, 1);

  // phone: 8x13 outline with cut corners, speaker and home button
  graphics_context_set_stroke_color(ctx, theme->background);
  graphics_draw_line(ctx, GPoint(x + 4, y), GPoint(x + 9, y));
  graphics_draw_line(ctx, GPoint(x + 4, y + 12), GPoint(x + 9, y + 12));
  graphics_draw_line(ctx, GPoint(x + 3, y + 1), GPoint(x + 3, y + 11));
  graphics_draw_line(ctx, GPoint(x + 10, y + 1), GPoint(x + 10, y + 11));
  graphics_draw_line(ctx, GPoint(x + 6, y + 2), GPoint(x + 7, y + 2));
  graphics_draw_line(ctx, GPoint(x + 6, y + 11), GPoint(x + 7, y + 11));

  // gap in square colour so the strike stays separate from the phone outline
  graphics_context_set_stroke_color(ctx, theme->primary_text);
  graphics_draw_line(ctx, GPoint(x, y + 1), GPoint(x + 12, y + 13));
  graphics_draw_line(ctx, GPoint(x + 2, y), GPoint(x + 13, y + 11));

  // 2 px strike through
  graphics_context_set_stroke_color(ctx, theme->background);
  graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 13, y + 13));
  graphics_draw_line(ctx, GPoint(x + 1, y), GPoint(x + 13, y + 12));
}
