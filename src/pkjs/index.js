var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

function getTemperatureUnit() {
    try {
        var settings = JSON.parse(localStorage.getItem('clay-settings')) || {};
        if (settings.TemperatureUnit === 'fahrenheit') {
            return 'fahrenheit';
        }
    } catch (e) {
    }
    return 'celsius';
}

function xhrRequest(url, type) {
    return new Promise(function (resolve, reject) {
        var xhr = new XMLHttpRequest();
        xhr.onload = function () {
            resolve(this.responseText);
        };
        xhr.onerror = function () {
            reject();
        };
        xhr.open(type, url);
        xhr.send();
    });
}

function getCityName(latitude, longitude) {
    var url = `https://api.bigdatacloud.net/data/reverse-geocode-client` +
        `?latitude=${latitude}&longitude=${longitude}&localityLanguage=en`;

    return xhrRequest(url, 'GET').then(function (responseText) {
        var json = JSON.parse(responseText);
        return json.city || json.locality || json.principalSubdivision || '';
    }).catch(function () {
        console.log('Error requesting city!');
        return '';
    });
}

// returns the next upcoming time ("HH:MM") out of today's and tomorrow's entries
function nextTime(times) {
    var now = new Date();
    for (var i = 0; i < times.length; i++) {
        if (now < new Date(times[i])) {
            return times[i].substring(11, 16);
        }
    }
    return times[times.length - 1].substring(11, 16);
}

function getWeatherData(latitude, longitude) {
    var url = `https://api.open-meteo.com/v1/forecast` +
        `?latitude=${latitude}&longitude=${longitude}&hourly=temperature_2m,weather_code&forecast_hours=13&current=relative_humidity_2m&daily=weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset,uv_index_max&forecast_days=2&temperature_unit=${getTemperatureUnit()}&timezone=auto`;

    return xhrRequest(url, 'GET').then(function (responseText) {
        return JSON.parse(responseText);
    });
}

function locationSuccess(position) {
    var latitude = position.coords.latitude;
    var longitude = position.coords.longitude;
    console.log(`position: ${JSON.stringify(position)}`);
    Promise.all([
        getWeatherData(latitude, longitude),
        getCityName(latitude, longitude)
    ]).then(function (results) {
        var weatherJson = results[0];
        var cityName = results[1];
        var hours = weatherJson.hourly.time.map(function (t) {
            return new Date(t).getHours();
        });
        var temps = weatherJson.hourly.temperature_2m.map(function (t) {
            return Math.round(t);
        });
        var temps_bytes = new Int8Array(temps);
        var hours_bytes = new Int8Array(hours);
        var weather_code_bytes = new Int8Array(weatherJson.hourly.weather_code);
        var today_temp_max = Math.round(weatherJson.daily.temperature_2m_max[0]);
        var today_temp_min = Math.round(weatherJson.daily.temperature_2m_min[0]);
        var tomorrow_temp_max = Math.round(weatherJson.daily.temperature_2m_max[1]);
        var tomorrow_weather_code = weatherJson.daily.weather_code[1];
        Pebble.sendAppMessage({
            'TEMP_COUNT': temps.length,
            'TEMP_DATA': Array.from(temps_bytes),
            'HOUR_DATA': Array.from(hours_bytes),
            'WEATHER_CODE_DATA': Array.from(weather_code_bytes),
            'TODAY_TEMP_MAX': today_temp_max,
            'TODAY_TEMP_MIN': today_temp_min,
            'TOMORROW_TEMP_MAX': tomorrow_temp_max,
            'TOMORROW_WEATHER_CODE': tomorrow_weather_code,
            'CITY_NAME': cityName.substring(0, 31),
            'SUNRISE_TIME': nextTime(weatherJson.daily.sunrise),
            'SUNSET_TIME': nextTime(weatherJson.daily.sunset),
            'UV_INDEX': Math.round(weatherJson.daily.uv_index_max[0] || 0),
            'HUMIDITY': Math.round(weatherJson.current.relative_humidity_2m),
            'ELEVATION': Math.round(weatherJson.elevation)
        });
    }).catch(function () {
        console.log('Error requesting weather!');
    });
}

function locationError(positionError) {
    console.log('Error requesting location!');
}

function getWeather() {
    navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
        { enableHighAccuracy: true, timeout: 15000, maximumAge: 60000 });
}

Pebble.addEventListener('ready',
    function (e) {
        console.log('PebbleKit JS ready!');
        getWeather();
    }
);

Pebble.addEventListener('webviewclosed', function (e) {
    if (e && e.response) {
        // upon saving settings, query the weather APIs, since there could have been a switch of the temperature unit
        getWeather();
    }
});

Pebble.addEventListener('appmessage',
    function (e) {
        console.log('AppMessage received!');
        if (e.payload['REQUEST_WEATHER']) {
            getWeather();
        }
    }
);
