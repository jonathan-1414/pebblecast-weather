#include "theme.h"

static const AppTheme APP_THEMES[APP_THEME_COUNT] = {
    [APP_THEME_DARK] =
        {
            .background = GColorBlack,
            .primary_text = GColorWhite,
            .divider = GColorWhite,
            .battery_low = GColorChromeYellow,
            .weather_sun = GColorChromeYellow,
            .weather_cloud = GColorLightGray,
            .weather_rain = GColorPictonBlue,
            .weather_storm = GColorIcterine,
            .weather_snow = GColorWhite,
            .weather_fog = GColorLightGray,
            .weather_moon = GColorWhite,
            .weather_moon_cutout = GColorBlack,
            .plot_axis = GColorWhite,
            .plot_line = GColorRed,
            .plot_label = GColorWhite,
        },
    [APP_THEME_LIGHT] =
        {
            .background = GColorWhite,
            .primary_text = GColorBlack,
            .divider = GColorBlack,
            .battery_low = GColorOrange,
            .weather_sun = GColorOrange,
            .weather_cloud = GColorDarkGray,
            .weather_rain = GColorBlue,
            .weather_storm = GColorOrange,
            .weather_snow = GColorDarkGray,
            .weather_fog = GColorDarkGray,
            .weather_moon = GColorBlack,
            .weather_moon_cutout = GColorWhite,
            .plot_axis = GColorBlack,
            .plot_line = GColorRed,
            .plot_label = GColorBlack,
        },
};

static AppThemeSetting s_theme_setting = APP_THEME_SETTING_DARK;
static AppThemeId s_theme_id = APP_THEME_DARK;

static AppThemeId app_theme_resolve(AppThemeSetting setting) {
  return setting == APP_THEME_SETTING_LIGHT ? APP_THEME_LIGHT : APP_THEME_DARK;
}

const AppTheme *app_theme_current(void) { return &APP_THEMES[s_theme_id]; }

AppThemeSetting app_theme_get_setting(void) { return s_theme_setting; }

void app_theme_set_setting(AppThemeSetting setting) {
  s_theme_setting = app_theme_setting_from_int(setting);
  s_theme_id = app_theme_resolve(s_theme_setting);
}

AppThemeSetting app_theme_setting_from_int(int value) {
  if (value == APP_THEME_SETTING_LIGHT) {
    return APP_THEME_SETTING_LIGHT;
  }
  return APP_THEME_SETTING_DARK;
}
