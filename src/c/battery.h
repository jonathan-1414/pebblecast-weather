#pragma once
#include "layout.h"
#include "theme.h"
#include <pebble.h>

static GColor get_fill_color(uint8_t battery_charge_percent) {
  const AppTheme *theme = app_theme_current();
  if (battery_charge_percent <= 20) {
    return theme->battery_low;
  }
  return theme->battery_ok;
}

static void draw_battery(uint8_t battery_charge_percent, GRect rect, GContext *ctx) {
  const AppTheme *theme = app_theme_current();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "draw_battery called");

  graphics_context_set_stroke_color(ctx, theme->battery_border);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_round_rect(ctx, rect, 0);

  GColor fill_color = get_fill_color(battery_charge_percent);
  graphics_context_set_fill_color(ctx, fill_color);

  GRect fill_rect = get_effective_rect(LayoutBox(rect, 2));
  int fill_width = (fill_rect.size.w * battery_charge_percent / 100);
  fill_rect.size.w = fill_width;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Battery percent: %d", battery_charge_percent);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Calculated width: %d", fill_width);

  graphics_fill_rect(ctx, fill_rect, 0, GCornersAll);
}
