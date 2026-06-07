#pragma once
#include "theme.h"
#include <pebble.h>

// Weather icon drawing functions
// All icons are ~18x18 pixels, white on black background
// (cx, cy) is the center point of the icon

// Sun: circle with rays
static void draw_icon_sun(GContext *ctx, int cx, int cy) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_stroke_color(ctx, theme->weather_sun);
  graphics_context_set_fill_color(ctx, theme->weather_sun);
  // filled circle (sun body)
  graphics_fill_circle(ctx, GPoint(cx, cy), 5);
  // rays (top, bottom, left, right)
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, GPoint(cx, cy - 9), GPoint(cx, cy - 7));
  graphics_draw_line(ctx, GPoint(cx, cy + 7), GPoint(cx, cy + 9));
  graphics_draw_line(ctx, GPoint(cx - 9, cy), GPoint(cx - 7, cy));
  graphics_draw_line(ctx, GPoint(cx + 7, cy), GPoint(cx + 9, cy));
  // diagonal rays
  graphics_draw_line(ctx, GPoint(cx - 6, cy - 6), GPoint(cx - 5, cy - 5));
  graphics_draw_line(ctx, GPoint(cx + 5, cy - 5), GPoint(cx + 6, cy - 6));
  graphics_draw_line(ctx, GPoint(cx - 6, cy + 6), GPoint(cx - 5, cy + 5));
  graphics_draw_line(ctx, GPoint(cx + 5, cy + 5), GPoint(cx + 6, cy + 6));
}

// Cloud: filled bumps with visible separation
static void draw_icon_cloud(GContext *ctx, int cx, int cy) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_fill_color(ctx, theme->weather_cloud);
  // fill all bumps
  graphics_fill_circle(ctx, GPoint(cx - 5, cy + 1), 4);
  graphics_fill_circle(ctx, GPoint(cx + 5, cy + 1), 4);
  graphics_fill_circle(ctx, GPoint(cx, cy - 3), 5);
  // flat bottom
  graphics_fill_rect(ctx, GRect(cx - 8, cy + 2, 16, 4), 0, GCornerNone);
}

static void draw_icon_fog(GContext *ctx, int cx, int cy) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_stroke_color(ctx, theme->weather_fog);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(cx - 8, cy - 5), GPoint(cx + 8, cy - 5));
  graphics_draw_line(ctx, GPoint(cx - 6, cy), GPoint(cx + 6, cy));
  graphics_draw_line(ctx, GPoint(cx - 8, cy + 5), GPoint(cx + 8, cy + 5));
}

static void draw_icon_partly_cloudy_day(GContext *ctx, int cx, int cy) {
  draw_icon_sun(ctx, cx - 4, cy - 3);
  draw_icon_cloud(ctx, cx + 2, cy + 3);
}

// Reusable droplet: draws a teardrop at (dx, dy) with given radius
static void draw_droplet(GContext *ctx, int dx, int dy, int radius) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_fill_color(ctx, theme->weather_rain);
  graphics_context_set_stroke_color(ctx, theme->weather_rain);
  // circle body at bottom
  graphics_fill_circle(ctx, GPoint(dx, dy + radius), radius);
  // filled triangle pointing up
  int tip_y = dy - (radius * 2);
  for (int row = tip_y; row <= dy; row++) {
    int half_width = (radius * (row - tip_y)) / (dy - tip_y);
    graphics_draw_line(ctx, GPoint(dx - half_width, row), GPoint(dx + half_width, row));
  }
}

// Light rain: one big droplet
static void draw_icon_light_rain(GContext *ctx, int cx, int cy) {
  draw_droplet(ctx, cx, cy, 3);
}

// Heavy rain: two big droplets, slightly overlapping
static void draw_icon_heavy_rain(GContext *ctx, int cx, int cy) {
  draw_droplet(ctx, cx - 3, cy + 1, 3);
  draw_droplet(ctx, cx + 3, cy - 1, 3);
}

// ⛈ Thunderstorm: just a lightning bolt
static void draw_icon_thunderstorm(GContext *ctx, int cx, int cy) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_fill_color(ctx, theme->weather_storm);
  graphics_context_set_stroke_color(ctx, theme->weather_storm);
  graphics_context_set_stroke_width(ctx, 2);
  // lightning bolt shape: top-right to middle-left, jog right, then down-left
  graphics_draw_line(ctx, GPoint(cx + 2, cy - 8), GPoint(cx - 3, cy - 1));
  graphics_draw_line(ctx, GPoint(cx - 3, cy - 1), GPoint(cx + 2, cy - 1));
  graphics_draw_line(ctx, GPoint(cx + 2, cy - 1), GPoint(cx - 2, cy + 8));
}

// Snowflake: three crossing lines
static void draw_icon_snowflake(GContext *ctx, int cx, int cy) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_stroke_color(ctx, theme->weather_snow);
  graphics_context_set_stroke_width(ctx, 1);
  // vertical line
  graphics_draw_line(ctx, GPoint(cx, cy - 6), GPoint(cx, cy + 6));
  // diagonal lines
  graphics_draw_line(ctx, GPoint(cx - 5, cy - 3), GPoint(cx + 5, cy + 3));
  graphics_draw_line(ctx, GPoint(cx - 5, cy + 3), GPoint(cx + 5, cy - 3));
}

// Moon: crescent shape using overlapping circles
static void draw_icon_moon(GContext *ctx, int cx, int cy) {
  const AppTheme *theme = app_theme_current();
  graphics_context_set_fill_color(ctx, theme->weather_moon);
  // draw full moon circle
  graphics_fill_circle(ctx, GPoint(cx, cy), 6);
  // cut out a circle offset to the right to create crescent
  graphics_context_set_fill_color(ctx, theme->background);
  graphics_fill_circle(ctx, GPoint(cx + 4, cy - 2), 5);
}

static void draw_icon_partly_cloudy_night(GContext *ctx, int cx, int cy) {
  draw_icon_moon(ctx, cx - 4, cy - 3);
  draw_icon_cloud(ctx, cx + 2, cy + 3);
}
