const DEFAULT_STEP_GOAL = 10000;
const MIN_STEP_GOAL = 1000;
const MAX_STEP_GOAL = 99999;
const DEFAULT_RING_COLOR = "ffffff";
const STEP_GOAL_STORAGE_KEY = "walkmate.stepGoal";
const RING_COLOR_STORAGE_KEY = "walkmate.ringColor";
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

function getInitialStepGoal() {
  const storedValue = parseInt(localStorage.getItem(STEP_GOAL_STORAGE_KEY), 10);
  if (Number.isFinite(storedValue) && storedValue >= MIN_STEP_GOAL && storedValue <= MAX_STEP_GOAL) {
    return storedValue;
  }

  return DEFAULT_STEP_GOAL;
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
    <p>Set a goal between ${MIN_STEP_GOAL} and ${MAX_STEP_GOAL} steps.</p>
    <label for="step-goal">Goal</label>
    <input id="step-goal" type="number" inputmode="numeric" min="${MIN_STEP_GOAL}" max="${MAX_STEP_GOAL}" value="${initialStepGoal}">
    <label>Ring Color</label>
    <div class="swatches">${colorOptions}</div>
    <button id="save" type="button">Save</button>
    <div id="error" class="error"></div>
    <script>
      (function () {
        var min = ${MIN_STEP_GOAL};
        var max = ${MAX_STEP_GOAL};
        var input = document.getElementById('step-goal');
        var error = document.getElementById('error');
        document.getElementById('save').addEventListener('click', function () {
          var value = parseInt(input.value, 10);
          var colorInput = document.querySelector('input[name="ring-color"]:checked');
          if (!Number.isFinite(value) || value < min || value > max) {
            error.textContent = 'Enter a number between ' + min + ' and ' + max + '.';
            return;
          }
          var payload = encodeURIComponent(JSON.stringify({
            stepGoal: value,
            ringColor: colorInput ? colorInput.value : '${DEFAULT_RING_COLOR}'
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
  if (!Number.isFinite(stepGoal) || stepGoal < MIN_STEP_GOAL || stepGoal > MAX_STEP_GOAL) {
    return;
  }

  localStorage.setItem(STEP_GOAL_STORAGE_KEY, String(stepGoal));
  localStorage.setItem(RING_COLOR_STORAGE_KEY, ringColor);

  Pebble.sendAppMessage(
    {
      STEP_GOAL: stepGoal,
      RING_COLOR: parseInt(ringColor, 16)
    },
    function() {
      console.log("Updated settings", stepGoal, ringColor);
    },
    function(error) {
      console.log("Failed to send settings", error);
    }
  );
});
