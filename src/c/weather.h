#pragma once
#include "theme.h"
#include <pebble.h>

typedef struct {
  int temps_count;
  int8_t *temps;
  int8_t *hours;
  int8_t *weather_codes;
  const char *city_name;
  int8_t today_temp_max;
  int8_t today_temp_min;
} WeatherData;

#define WeatherData(temps_count, temps, hours, weather_codes, city_name, today_temp_max, today_temp_min) \
  ((WeatherData){temps_count, temps, hours, weather_codes, city_name, today_temp_max, today_temp_min})

void draw_weather(GRect rect, WeatherData data, GFont font, GContext *ctx);
void draw_weather_summary(GRect rect, const char *label, int8_t weather_code, int8_t temp_max, GFont font,
                          GContext *ctx);
void draw_arrow_up(GContext *ctx, GRect rect);
void draw_arrow_down(GContext *ctx, GRect rect);
