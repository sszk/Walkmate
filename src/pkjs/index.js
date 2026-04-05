const DEFAULT_STEP_GOAL = 10000;
const MIN_STEP_GOAL = 1000;
const MAX_STEP_GOAL = 99999;
const STEP_GOAL_STORAGE_KEY = "walkmate.stepGoal";

function getInitialStepGoal() {
  const storedValue = parseInt(localStorage.getItem(STEP_GOAL_STORAGE_KEY), 10);
  if (Number.isFinite(storedValue) && storedValue >= MIN_STEP_GOAL && storedValue <= MAX_STEP_GOAL) {
    return storedValue;
  }

  return DEFAULT_STEP_GOAL;
}

function buildConfigUrl() {
  const initialStepGoal = getInitialStepGoal();
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
    <h1>Daily Step Goal</h1>
    <p>Set a goal between ${MIN_STEP_GOAL} and ${MAX_STEP_GOAL} steps.</p>
    <label for="step-goal">Goal</label>
    <input id="step-goal" type="number" inputmode="numeric" min="${MIN_STEP_GOAL}" max="${MAX_STEP_GOAL}" value="${initialStepGoal}">
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
          if (!Number.isFinite(value) || value < min || value > max) {
            error.textContent = 'Enter a number between ' + min + ' and ' + max + '.';
            return;
          }
          var payload = encodeURIComponent(JSON.stringify({ stepGoal: value }));
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
  if (!Number.isFinite(stepGoal) || stepGoal < MIN_STEP_GOAL || stepGoal > MAX_STEP_GOAL) {
    return;
  }

  localStorage.setItem(STEP_GOAL_STORAGE_KEY, String(stepGoal));

  Pebble.sendAppMessage(
    { STEP_GOAL: stepGoal },
    function() {
      console.log("Updated step goal to", stepGoal);
    },
    function(error) {
      console.log("Failed to send step goal", error);
    }
  );
});
