# Drone MVP — 4-Servo ESP32 Controller

## Hardware

| Servo | Label       | ESP32-C3 GPIO |
|-------|-------------|---------------|
| 0     | Front-Left  | 4             |
| 1     | Front-Right | 5             |
| 2     | Rear-Left   | 6             |
| 3     | Rear-Right  | 7             |

**Power**: servos need 5V from an external BEC/regulator — do NOT power them from the ESP32 3.3V pin. Share GND between the ESP32 and the servo power supply.

## Arduino Libraries (install via Library Manager)

- **ESP32Servo** by Kevin Harrington
- **WebSockets** by Markus Sattler

Board: *ESP32C3 Dev Module*.

## Flash & Use

1. Open `firmware/firmware.ino` in Arduino IDE.
2. Select your ESP32 board and port, then upload.
3. Connect your computer to WiFi **DroneControl** / password **drone1234**.
4. Open **http://192.168.4.1** in a browser.
5. Use the four sliders to control each servo independently.

## Customization

- Change `AP_SSID` / `AP_PASSWORD` in `firmware.ino`.
- Change `SERVO_PINS[]` to match your wiring.
- Adjust `PWM_MIN_US` / `PWM_MAX_US` for non-standard servos.
