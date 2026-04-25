const DEFAULT_STEP_GOAL = 10000;
const MIN_STEP_GOAL = 1000;
const MAX_STEP_GOAL = 99999;
const DEFAULT_RING_COLOR = "ffffff";
const STEP_GOAL_STORAGE_KEY = "walkmate.stepGoal";
const RING_COLOR_STORAGE_KEY = "walkmate.ringColor";
const DEFAULT_WEATHER_UPDATE_INTERVAL = 30;
const MIN_WEATHER_UPDATE_INTERVAL = 5;
const MAX_WEATHER_UPDATE_INTERVAL = 180;
const WEATHER_UPDATE_INTERVAL_STORAGE_KEY = "walkmate.weatherUpdateInterval";
const DEFAULT_TEMPERATURE_DISPLAY_MAX = 40;
const DEFAULT_TEMPERATURE_DISPLAY_MIN = -10;
const MIN_TEMPERATURE_DISPLAY = -50;
const MAX_TEMPERATURE_DISPLAY = 60;
const TEMPERATURE_DISPLAY_MAX_STORAGE_KEY = "walkmate.temperatureDisplayMax";
const TEMPERATURE_DISPLAY_MIN_STORAGE_KEY = "walkmate.temperatureDisplayMin";
const WEATHER_API_BASE_URL = "https://api.open-meteo.com/v1/forecast";
const COLOR_RING_COLORS = [
  { name: "White", value: "ffffff" },
  { name: "Blue", value: "55aaff" },
  { name: "Green", value: "55ff55" },
  { name: "Yellow", value: "ffff55" },
  { name: "Orange", value: "ffaa55" },
  { name: "Red", value: "ff5555" },
  { name: "Purple", value: "aa55ff" },
  { name: "Pink", value: "ff55aa" }
];
const GRAYSCALE_RING_COLORS = [
  { name: "White", value: "ffffff" },
  { name: "Grey", value: "aaaaaa" }
];
const MONOCHROME_PLATFORMS = ["aplite", "diorite"];
const MONOCHROME_MODELS = ["aplite", "diorite", "pebble2", "pebble_2"];

function sendWeatherData(temperature, temperatureMax, temperatureMin) {
  Pebble.sendAppMessage(
    {
      WEATHER_TEMPERATURE: temperature,
      WEATHER_TEMPERATURE_MAX: temperatureMax,
      WEATHER_TEMPERATURE_MIN: temperatureMin,
    },
    function() {
      console.log("Updated weather", temperature, temperatureMax, temperatureMin);
    },
    function(error) {
      console.log("Failed to send weather", error);
    }
  );
}

function requestWeather() {
  navigator.geolocation.getCurrentPosition(
    function(position) {
      const latitude = position.coords.latitude;
      const longitude = position.coords.longitude;
      const url =
        WEATHER_API_BASE_URL +
        "?latitude=" + encodeURIComponent(String(latitude)) +
        "&longitude=" + encodeURIComponent(String(longitude)) +
        "&current=temperature_2m" +
        "&daily=temperature_2m_max,temperature_2m_min" +
        "&forecast_days=1" +
        "&timezone=auto";
      const request = new XMLHttpRequest();
      request.onload = function() {
        if (request.status < 200 || request.status >= 300) {
          console.log("Weather request failed with status", request.status);
          return;
        }

        try {
          const data = JSON.parse(request.responseText);
          const current = data && data.current ? data.current.temperature_2m : null;
          const daily = data && data.daily ? data.daily : null;
          const max = daily && Array.isArray(daily.temperature_2m_max) ? daily.temperature_2m_max[0] : null;
          const min = daily && Array.isArray(daily.temperature_2m_min) ? daily.temperature_2m_min[0] : null;

          if (![current, max, min].every(Number.isFinite)) {
            throw new Error("Weather payload missing temperature data");
          }

          sendWeatherData(Math.round(current), Math.round(max), Math.round(min));
        } catch (error) {
          console.log("Failed to parse weather response", error);
        }
      };
      request.onerror = function(error) {
        console.log("Failed to fetch weather", error);
      };
      request.open("GET", url);
      request.send();
    },
    function(error) {
      console.log("Failed to get current position", error);
    },
    {
      enableHighAccuracy: false,
      maximumAge: 10 * 60 * 1000,
      timeout: 15 * 1000,
    }
  );
}

function getInitialStepGoal() {
  const storedValue = parseInt(localStorage.getItem(STEP_GOAL_STORAGE_KEY), 10);
  if (Number.isFinite(storedValue) && storedValue >= MIN_STEP_GOAL && storedValue <= MAX_STEP_GOAL) {
    return storedValue;
  }

  return DEFAULT_STEP_GOAL;
}

function getInitialWeatherUpdateInterval() {
  const storedValue = parseInt(localStorage.getItem(WEATHER_UPDATE_INTERVAL_STORAGE_KEY), 10);
  if (
    Number.isFinite(storedValue) &&
    storedValue >= MIN_WEATHER_UPDATE_INTERVAL &&
    storedValue <= MAX_WEATHER_UPDATE_INTERVAL
  ) {
    return storedValue;
  }

  return DEFAULT_WEATHER_UPDATE_INTERVAL;
}

function isValidTemperatureDisplayRange(displayMax, displayMin) {
  return Number.isFinite(displayMax) &&
    Number.isFinite(displayMin) &&
    displayMin >= MIN_TEMPERATURE_DISPLAY &&
    displayMax <= MAX_TEMPERATURE_DISPLAY &&
    displayMin < displayMax;
}

function getInitialTemperatureDisplayRange() {
  const storedMax = parseInt(localStorage.getItem(TEMPERATURE_DISPLAY_MAX_STORAGE_KEY), 10);
  const storedMin = parseInt(localStorage.getItem(TEMPERATURE_DISPLAY_MIN_STORAGE_KEY), 10);
  if (isValidTemperatureDisplayRange(storedMax, storedMin)) {
    return {
      max: storedMax,
      min: storedMin
    };
  }

  return {
    max: DEFAULT_TEMPERATURE_DISPLAY_MAX,
    min: DEFAULT_TEMPERATURE_DISPLAY_MIN
  };
}

function getActiveWatchInfo() {
  if (typeof Pebble.getActiveWatchInfo !== "function") {
    return {};
  }

  return Pebble.getActiveWatchInfo() || {};
}

function isMonochromeWatchInfo(watchInfo) {
  const platform = String(watchInfo.platform || "").toLowerCase();
  const model = String(watchInfo.model || "").toLowerCase();

  if (MONOCHROME_PLATFORMS.indexOf(platform) !== -1) {
    return true;
  }

  return MONOCHROME_MODELS.some(function(modelName) {
    return model.indexOf(modelName) !== -1;
  });
}

function getRingColorOptions() {
  return isMonochromeWatchInfo(getActiveWatchInfo()) ? GRAYSCALE_RING_COLORS : COLOR_RING_COLORS;
}

function normalizeRingColor(value, options) {
  const color = String(value || "").replace(/^#/, "").toLowerCase();
  return options.some(function(option) {
    return option.value === color;
  }) ? color : DEFAULT_RING_COLOR;
}

function buildConfigUrl() {
  const ringColorOptions = getRingColorOptions();
  const initialStepGoal = getInitialStepGoal();
  const initialRingColor = normalizeRingColor(localStorage.getItem(RING_COLOR_STORAGE_KEY), ringColorOptions);
  const initialWeatherUpdateInterval = getInitialWeatherUpdateInterval();
  const initialTemperatureDisplayRange = getInitialTemperatureDisplayRange();
  const colorOptions = ringColorOptions.map(function(option) {
    const checked = option.value === initialRingColor ? " checked" : "";
    return `
      <label class="swatch-option" style="--swatch-color: #${option.value};">
        <input type="radio" name="ring-color" value="${option.value}"${checked}>
        <span class="swatch"></span>
        <span class="swatch-label">${option.name}</span>
      </label>`;
  }).join("");
  const html = `
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Walkmate Settings</title>
    <style>
      body {
        margin: 0;
        padding: 24px 20px;
        background: #000000;
        color: #ffffff;
        font-family: -apple-system, BlinkMacSystemFont, sans-serif;
      }
      h1 {
        margin: 0 0 12px;
        font-size: 24px;
      }
      p {
        margin: 0 0 16px;
        color: #c7c7c7;
        line-height: 1.4;
      }
      label {
        display: block;
        margin-bottom: 8px;
        font-size: 14px;
      }
      input {
        box-sizing: border-box;
        width: 100%;
        padding: 14px 12px;
        border: 1px solid #444444;
        border-radius: 10px;
        background: #111111;
        color: #ffffff;
        font-size: 18px;
      }
      .section {
        margin-top: 24px;
      }
      .temperature-range {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 12px;
      }
      .swatches {
        display: grid;
        grid-template-columns: repeat(4, 1fr);
        gap: 10px;
        margin-top: 10px;
      }
      .swatch-option {
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 6px;
        margin: 0;
        color: #c7c7c7;
        font-size: 12px;
      }
      .swatch-option input {
        position: absolute;
        opacity: 0;
        pointer-events: none;
      }
      .swatch {
        box-sizing: border-box;
        width: 40px;
        height: 40px;
        border: 2px solid #444444;
        border-radius: 50%;
        background: var(--swatch-color);
      }
      .swatch-option input:checked + .swatch {
        border-color: #ffffff;
        box-shadow: 0 0 0 3px #555555;
      }
      .swatch-label {
        line-height: 1.2;
      }
      button {
        width: 100%;
        margin-top: 16px;
        padding: 14px 12px;
        border: 0;
        border-radius: 10px;
        background: #ffffff;
        color: #000000;
        font-size: 16px;
        font-weight: 600;
      }
      .error {
        min-height: 20px;
        margin-top: 12px;
        color: #ff7b7b;
        font-size: 14px;
      }
    </style>
  </head>
  <body>
    <h1>Walkmate Settings</h1>
    <div class="section">
      <p>Set a goal between ${MIN_STEP_GOAL} and ${MAX_STEP_GOAL} steps.</p>
      <label for="step-goal">Daily Step Goal</label>
      <input id="step-goal" type="number" inputmode="numeric" min="${MIN_STEP_GOAL}" max="${MAX_STEP_GOAL}" value="${initialStepGoal}">
    </div>
    <div class="section">
      <label>Ring Color</label>
      <div class="swatches">${colorOptions}</div>
    </div>
    <div class="section">
      <p>Choose how often weather data should refresh, in minutes.</p>
      <label for="weather-update-interval">Weather Update Interval</label>
      <input id="weather-update-interval" type="number" inputmode="numeric" min="${MIN_WEATHER_UPDATE_INTERVAL}" max="${MAX_WEATHER_UPDATE_INTERVAL}" value="${initialWeatherUpdateInterval}">
    </div>
    <div class="section">
      <p>Set the temperature range used by the weather gauge.</p>
      <div class="temperature-range">
        <div>
          <label for="temperature-display-min">Lowest Temperature (C)</label>
          <input id="temperature-display-min" type="number" min="${MIN_TEMPERATURE_DISPLAY}" max="${MAX_TEMPERATURE_DISPLAY}" value="${initialTemperatureDisplayRange.min}">
        </div>
        <div>
          <label for="temperature-display-max">Highest Temperature (C)</label>
          <input id="temperature-display-max" type="number" min="${MIN_TEMPERATURE_DISPLAY}" max="${MAX_TEMPERATURE_DISPLAY}" value="${initialTemperatureDisplayRange.max}">
        </div>
      </div>
    </div>
    <button id="save" type="button">Save</button>
    <div id="error" class="error"></div>
    <script>
      (function () {
        var stepMin = ${MIN_STEP_GOAL};
        var stepMax = ${MAX_STEP_GOAL};
        var weatherMin = ${MIN_WEATHER_UPDATE_INTERVAL};
        var weatherMax = ${MAX_WEATHER_UPDATE_INTERVAL};
        var temperatureMin = ${MIN_TEMPERATURE_DISPLAY};
        var temperatureMax = ${MAX_TEMPERATURE_DISPLAY};
        var stepInput = document.getElementById('step-goal');
        var weatherInput = document.getElementById('weather-update-interval');
        var temperatureDisplayMinInput = document.getElementById('temperature-display-min');
        var temperatureDisplayMaxInput = document.getElementById('temperature-display-max');
        var error = document.getElementById('error');
        document.getElementById('save').addEventListener('click', function () {
          var stepGoal = parseInt(stepInput.value, 10);
          var weatherUpdateInterval = parseInt(weatherInput.value, 10);
          var temperatureDisplayMin = parseInt(temperatureDisplayMinInput.value, 10);
          var temperatureDisplayMax = parseInt(temperatureDisplayMaxInput.value, 10);
          var colorInput = document.querySelector('input[name="ring-color"]:checked');
          if (!Number.isFinite(stepGoal) || stepGoal < stepMin || stepGoal > stepMax) {
            error.textContent = 'Step goal must be between ' + stepMin + ' and ' + stepMax + '.';
            return;
          }
          if (!Number.isFinite(weatherUpdateInterval) || weatherUpdateInterval < weatherMin || weatherUpdateInterval > weatherMax) {
            error.textContent = 'Weather update interval must be between ' + weatherMin + ' and ' + weatherMax + ' minutes.';
            return;
          }
          if (!Number.isFinite(temperatureDisplayMin) || !Number.isFinite(temperatureDisplayMax) || temperatureDisplayMin < temperatureMin || temperatureDisplayMax > temperatureMax || temperatureDisplayMin >= temperatureDisplayMax) {
            error.textContent = 'Temperature range must be between ' + temperatureMin + ' and ' + temperatureMax + ', with lowest below highest.';
            return;
          }
          var payload = encodeURIComponent(JSON.stringify({
            stepGoal: stepGoal,
            ringColor: colorInput ? colorInput.value : '${DEFAULT_RING_COLOR}',
            weatherUpdateInterval: weatherUpdateInterval,
            temperatureDisplayMax: temperatureDisplayMax,
            temperatureDisplayMin: temperatureDisplayMin
          }));
          document.location = 'pebblejs://close#' + payload;
        });
      }());
    </script>
  </body>
</html>`;

  return "data:text/html;charset=utf-8," + encodeURIComponent(html);
}

Pebble.addEventListener("showConfiguration", function() {
  Pebble.openURL(buildConfigUrl());
});

Pebble.addEventListener("appmessage", function(event) {
  if (event.payload && event.payload.WEATHER_REQUEST) {
    requestWeather();
  }
});

Pebble.addEventListener("webviewclosed", function(event) {
  if (!event.response) {
    return;
  }

  let config;
  try {
    config = JSON.parse(decodeURIComponent(event.response));
  } catch (error) {
    console.log("Failed to parse configuration", error);
    return;
  }

  const stepGoal = parseInt(config.stepGoal, 10);
  const ringColor = normalizeRingColor(config.ringColor, getRingColorOptions());
  const weatherUpdateInterval = parseInt(config.weatherUpdateInterval, 10);
  const temperatureDisplayMax = parseInt(config.temperatureDisplayMax, 10);
  const temperatureDisplayMin = parseInt(config.temperatureDisplayMin, 10);
  if (!Number.isFinite(stepGoal) || stepGoal < MIN_STEP_GOAL || stepGoal > MAX_STEP_GOAL) {
    return;
  }
  if (
    !Number.isFinite(weatherUpdateInterval) ||
    weatherUpdateInterval < MIN_WEATHER_UPDATE_INTERVAL ||
    weatherUpdateInterval > MAX_WEATHER_UPDATE_INTERVAL
  ) {
    return;
  }
  if (!isValidTemperatureDisplayRange(temperatureDisplayMax, temperatureDisplayMin)) {
    return;
  }

  localStorage.setItem(STEP_GOAL_STORAGE_KEY, String(stepGoal));
  localStorage.setItem(RING_COLOR_STORAGE_KEY, ringColor);
  localStorage.setItem(WEATHER_UPDATE_INTERVAL_STORAGE_KEY, String(weatherUpdateInterval));
  localStorage.setItem(TEMPERATURE_DISPLAY_MAX_STORAGE_KEY, String(temperatureDisplayMax));
  localStorage.setItem(TEMPERATURE_DISPLAY_MIN_STORAGE_KEY, String(temperatureDisplayMin));

  Pebble.sendAppMessage(
    {
      STEP_GOAL: stepGoal,
      RING_COLOR: parseInt(ringColor, 16),
      WEATHER_UPDATE_INTERVAL: weatherUpdateInterval,
      TEMPERATURE_DISPLAY_MAX: temperatureDisplayMax,
      TEMPERATURE_DISPLAY_MIN: temperatureDisplayMin,
    },
    function() {
      console.log("Updated settings", stepGoal, ringColor, weatherUpdateInterval, temperatureDisplayMax, temperatureDisplayMin);
    },
    function(error) {
      console.log("Failed to send settings", error);
    }
  );
});
