module.exports = [
    {
        "type": "heading",
        "defaultValue": "Pebblecast Weather Settings"
    },
    {
        "type": "section",
        "items": [
            {
                "type": "heading",
                "defaultValue": "Appearance"
            },
            {
                "type": "select",
                "messageKey": "ThemeId",
                "defaultValue": 0,
                "label": "Theme",
                "options": [
                    {
                        "label": "Dark",
                        "value": 0
                    },
                    {
                        "label": "Light",
                        "value": 1
                    }
                ]
            },
            {
                "type": "select",
                "messageKey": "TemperatureUnit",
                "defaultValue": "celsius",
                "label": "Temperature Unit",
                "options": [
                    {
                        "label": "Celsius",
                        "value": "celsius"
                    },
                    {
                        "label": "Fahrenheit",
                        "value": "fahrenheit"
                    }
                ]
            }
        ]
    },
    {
        "type": "submit",
        "defaultValue": "Save Settings"
    }
];
