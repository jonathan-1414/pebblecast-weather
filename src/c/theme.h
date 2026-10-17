#pragma once
#include <pebble.h>

typedef enum {
  APP_THEME_SETTING_DARK = 0,
  APP_THEME_SETTING_LIGHT = 1,
} AppThemeSetting;

typedef enum {
  APP_THEME_DARK = 0,
  APP_THEME_LIGHT = 1,
  APP_THEME_COUNT = 2,
} AppThemeId;

typedef struct {
  GColor8 background;
  GColor8 primary_text;
  GColor8 divider;
  GColor8 battery_low;
  GColor8 weather_sun;
  GColor8 weather_cloud;
  GColor8 weather_rain;
  GColor8 weather_storm;
  GColor8 weather_snow;
  GColor8 weather_fog;
  GColor8 weather_moon;
  GColor8 weather_moon_cutout;
  GColor8 plot_axis;
  GColor8 plot_line;
  GColor8 plot_label;
} AppTheme;

const AppTheme *app_theme_current(void);
AppThemeSetting app_theme_get_setting(void);
void app_theme_set_setting(AppThemeSetting setting);
AppThemeSetting app_theme_setting_from_int(int value);
