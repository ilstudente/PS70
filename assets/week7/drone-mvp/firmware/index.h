#pragma once

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Drone Control</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }

    body {
      font-family: 'Segoe UI', sans-serif;
      background: #0f0f0f;
      color: #e0e0e0;
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: 100vh;
      padding: 2rem 1rem;
      gap: 1.5rem;
    }

    h1 {
      font-size: 1.6rem;
      letter-spacing: 0.1em;
      color: #fff;
    }

    #status {
      font-size: 0.85rem;
      color: #888;
    }
    #status.connected    { color: #4caf50; }
    #status.disconnected { color: #f44336; }

    /* FPV */
    .fpv-wrap {
      width: 100%;
      max-width: 640px;
      display: flex;
      flex-direction: column;
      gap: 0.6rem;
    }

    .fpv-screen {
      width: 100%;
      aspect-ratio: 16/9;
      background: #111;
      border: 1px solid #2a2a2a;
      border-radius: 12px;
      overflow: hidden;
      display: flex;
      align-items: center;
      justify-content: center;
      position: relative;
    }

    .fpv-screen img {
      width: 100%;
      height: 100%;
      object-fit: cover;
      display: none;
    }

    .fpv-no-signal {
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 0.5rem;
      color: #333;
      font-size: 0.9rem;
      letter-spacing: 0.1em;
    }

    .fpv-no-signal svg { opacity: 0.3; }

    .fpv-url-row {
      display: flex;
      gap: 0.5rem;
    }

    .fpv-url-row input {
      flex: 1;
      background: #1c1c1c;
      border: 1px solid #2a2a2a;
      border-radius: 8px;
      padding: 0.5rem 0.8rem;
      color: #e0e0e0;
      font-size: 0.85rem;
      outline: none;
    }

    .fpv-url-row input:focus { border-color: #2196f3; }

    .fpv-url-row button {
      padding: 0.5rem 1rem;
      background: #1c1c1c;
      border: 1px solid #2a2a2a;
      border-radius: 8px;
      color: #aaa;
      font-size: 0.85rem;
      cursor: pointer;
      transition: background 0.2s, color 0.2s;
    }

    .fpv-url-row button:hover { background: #333; color: #fff; }

    /* Master slider */
    .master-card {
      width: 100%;
      max-width: 640px;
      background: #1c1c1c;
      border: 1px solid #2196f3;
      border-radius: 12px;
      padding: 1.2rem 1.5rem;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 0.8rem;
    }

    .master-card h2 {
      font-size: 0.85rem;
      letter-spacing: 0.12em;
      color: #2196f3;
    }

    /* Servo grid */
    .grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 1.5rem;
      width: 100%;
      max-width: 640px;
    }

    .card {
      background: #1c1c1c;
      border: 1px solid #2a2a2a;
      border-radius: 12px;
      padding: 1.5rem;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 1rem;
    }

    .card h2 {
      font-size: 1rem;
      color: #aaa;
      letter-spacing: 0.05em;
    }

    .angle-display {
      font-size: 2.4rem;
      font-weight: 700;
      color: #fff;
      min-width: 4ch;
      text-align: center;
    }

    .angle-display span { font-size: 1rem; color: #666; }

    input[type="range"] {
      -webkit-appearance: none;
      width: 100%;
      height: 6px;
      border-radius: 3px;
      background: #333;
      outline: none;
      cursor: pointer;
    }

    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 20px;
      height: 20px;
      border-radius: 50%;
      background: #2196f3;
      cursor: pointer;
      transition: background 0.2s;
    }

    input[type="range"]::-webkit-slider-thumb:hover { background: #42a5f5; }

    .btn-center {
      padding: 0.4rem 1rem;
      font-size: 0.8rem;
      border: 1px solid #333;
      border-radius: 6px;
      background: #252525;
      color: #aaa;
      cursor: pointer;
      transition: background 0.2s;
    }

    .btn-center:hover { background: #333; color: #fff; }

    /* Action buttons row */
    .btn-row {
      display: flex;
      gap: 1rem;
    }

    .btn-action {
      padding: 0.7rem 2rem;
      font-size: 0.95rem;
      border-radius: 8px;
      cursor: pointer;
      letter-spacing: 0.05em;
      transition: background 0.2s, color 0.2s;
    }

    .btn-hover {
      border: 1px solid #4caf50;
      background: transparent;
      color: #4caf50;
    }

    .btn-hover:hover, .btn-hover.active {
      background: #4caf50;
      color: #fff;
    }

    .btn-center-all {
      border: 1px solid #2196f3;
      background: transparent;
      color: #2196f3;
    }

    .btn-center-all:hover { background: #2196f3; color: #fff; }
  </style>
</head>
<body>
  <h1>DRONE CONTROL</h1>
  <div id="status" class="disconnected">Connecting...</div>

  <!-- FPV -->
  <div class="fpv-wrap">
    <div class="fpv-screen">
      <img id="fpv-img" alt="FPV stream" />
      <div class="fpv-no-signal" id="fpv-placeholder">
        <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
          <path d="M15 10l4.553-2.07A1 1 0 0121 8.87v6.26a1 1 0 01-1.447.9L15 14M3 8h12v8H3z" stroke-linecap="round" stroke-linejoin="round"/>
        </svg>
        NO SIGNAL
      </div>
    </div>
    <div class="fpv-url-row">
      <input type="text" id="fpv-url" placeholder="Stream URL — e.g. http://192.168.4.2/stream" />
      <button onclick="connectStream()">Connect</button>
      <button onclick="disconnectStream()">Clear</button>
    </div>
  </div>

  <!-- Master -->
  <div class="master-card">
    <h2>ALL SERVOS</h2>
    <div class="angle-display" id="val-master">90<span>&deg;</span></div>
    <input type="range" min="0" max="180" value="90" id="slider-master" />
  </div>

  <!-- Individual servos -->
  <div class="grid" id="grid"></div>

  <!-- Action buttons -->
  <div class="btn-row">
    <button class="btn-action btn-hover" id="hover-btn" onclick="toggleHover()">Hover</button>
    <button class="btn-action btn-center-all" onclick="centerAll()">Center All</button>
  </div>

  <script>
    const SERVO_LABELS = ['Front-Left', 'Front-Right', 'Rear-Left', 'Rear-Right'];
    const NUM_SERVOS = 4;
    const sliders = [];
    const displays = [];

    // Build servo cards
    const grid = document.getElementById('grid');
    for (let i = 0; i < NUM_SERVOS; i++) {
      const card = document.createElement('div');
      card.className = 'card';
      card.innerHTML = `
        <h2>${SERVO_LABELS[i]}</h2>
        <div class="angle-display" id="val${i}">90<span>&deg;</span></div>
        <input type="range" min="0" max="180" value="90" id="slider${i}" />
        <button class="btn-center" onclick="centerServo(${i})">Center</button>
      `;
      grid.appendChild(card);

      const slider = document.getElementById(`slider${i}`);
      const display = document.getElementById(`val${i}`);
      sliders.push(slider);
      displays.push(display);

      slider.addEventListener('input', () => {
        displays[i].innerHTML = slider.value + '<span>&deg;</span>';
        sendServo(i, parseInt(slider.value));
      });
    }

    // Master slider — moves all servos together
    const masterSlider  = document.getElementById('slider-master');
    const masterDisplay = document.getElementById('val-master');

    masterSlider.addEventListener('input', () => {
      const angle = parseInt(masterSlider.value);
      masterDisplay.innerHTML = angle + '<span>&deg;</span>';
      for (let i = 0; i < NUM_SERVOS; i++) {
        sliders[i].value = angle;
        displays[i].innerHTML = angle + '<span>&deg;</span>';
        sendServo(i, angle);
      }
    });

    // WebSocket
    const ws = new WebSocket(`ws://${location.hostname}:81`);
    const statusEl = document.getElementById('status');

    ws.onopen  = () => { statusEl.textContent = 'Connected';                      statusEl.className = 'connected'; };
    ws.onclose = () => { statusEl.textContent = 'Disconnected — reload to reconnect'; statusEl.className = 'disconnected'; };
    ws.onerror = () => { statusEl.textContent = 'Connection error';               statusEl.className = 'disconnected'; };

    function sendServo(index, angle) {
      if (ws.readyState === WebSocket.OPEN) ws.send(`${index}:${angle}`);
    }

    function setAngle(i, angle) {
      sliders[i].value = angle;
      displays[i].innerHTML = angle + '<span>&deg;</span>';
      sendServo(i, angle);
    }

    function centerServo(i) { setAngle(i, 90); }

    function centerAll() {
      masterSlider.value = 90;
      masterDisplay.innerHTML = '90<span>&deg;</span>';
      for (let i = 0; i < NUM_SERVOS; i++) centerServo(i);
    }

    // Hover — oscillates all servos between 60° and 120° using a sine wave
    let hoverTimer = null;
    let hoverPhase = 0;

    function toggleHover() {
      const btn = document.getElementById('hover-btn');
      if (hoverTimer) {
        clearInterval(hoverTimer);
        hoverTimer = null;
        btn.textContent = 'Hover';
        btn.classList.remove('active');
        centerAll();
      } else {
        hoverPhase = 0;
        btn.textContent = 'Stop Hover';
        btn.classList.add('active');
        hoverTimer = setInterval(() => {
          hoverPhase += 0.06;
          const angle = Math.round(90 + 30 * Math.sin(hoverPhase));
          masterSlider.value = angle;
          masterDisplay.innerHTML = angle + '<span>&deg;</span>';
          for (let i = 0; i < NUM_SERVOS; i++) setAngle(i, angle);
        }, 50);
      }
    }

    // FPV stream
    function connectStream() {
      const url = document.getElementById('fpv-url').value.trim();
      if (!url) return;
      const img = document.getElementById('fpv-img');
      const placeholder = document.getElementById('fpv-placeholder');
      img.src = url;
      img.style.display = 'block';
      placeholder.style.display = 'none';
    }

    function disconnectStream() {
      const img = document.getElementById('fpv-img');
      const placeholder = document.getElementById('fpv-placeholder');
      img.src = '';
      img.style.display = 'none';
      placeholder.style.display = 'flex';
      document.getElementById('fpv-url').value = '';
    }
  </script>
</body>
</html>
)rawliteral";
