#include "theme.h"

static const AppTheme APP_THEMES[APP_THEME_COUNT] = {
    [APP_THEME_DARK] =
        {
            .background = GColorBlack,
            .primary_text = GColorWhite,
            .divider = GColorWhite,
            .battery_border = GColorWhite,
            .battery_low = GColorChromeYellow,
            .battery_ok = GColorGreen,
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
            .battery_border = GColorBlack,
            .battery_low = GColorOrange,
            .battery_ok = GColorIslamicGreen,
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
    [APP_THEME_BW_DARK] =
        {
            .background = GColorBlack,
            .primary_text = GColorWhite,
            .divider = GColorWhite,
            .battery_border = GColorWhite,
            .battery_low = GColorWhite,
            .battery_ok = GColorWhite,
            .weather_sun = GColorWhite,
            .weather_cloud = GColorWhite,
            .weather_rain = GColorWhite,
            .weather_storm = GColorWhite,
            .weather_snow = GColorWhite,
            .weather_fog = GColorWhite,
            .weather_moon = GColorWhite,
            .weather_moon_cutout = GColorBlack,
            .plot_axis = GColorWhite,
            .plot_line = GColorWhite,
            .plot_label = GColorWhite,
        },
    [APP_THEME_BW_LIGHT] =
        {
            .background = GColorWhite,
            .primary_text = GColorBlack,
            .divider = GColorBlack,
            .battery_border = GColorBlack,
            .battery_low = GColorBlack,
            .battery_ok = GColorBlack,
            .weather_sun = GColorBlack,
            .weather_cloud = GColorBlack,
            .weather_rain = GColorBlack,
            .weather_storm = GColorBlack,
            .weather_snow = GColorBlack,
            .weather_fog = GColorBlack,
            .weather_moon = GColorBlack,
            .weather_moon_cutout = GColorWhite,
            .plot_axis = GColorBlack,
            .plot_line = GColorBlack,
            .plot_label = GColorBlack,
        },
};

static AppThemeSetting s_theme_setting = APP_THEME_SETTING_DARK;
static AppThemeId s_theme_id = APP_THEME_DARK;

static AppThemeId app_theme_resolve(AppThemeSetting setting) {
#if PBL_COLOR
  return setting == APP_THEME_SETTING_LIGHT ? APP_THEME_LIGHT : APP_THEME_DARK;
#else
  return setting == APP_THEME_SETTING_LIGHT ? APP_THEME_BW_LIGHT : APP_THEME_BW_DARK;
#endif
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
