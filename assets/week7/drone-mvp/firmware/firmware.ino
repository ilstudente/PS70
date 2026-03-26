/*
 * Drone MVP — 4-Servo Controller
 * Board  : ESP32-C3 (XIAO variant)
 * Libs   : ESP32Servo, WebSockets (by Markus Sattler)
 *
 * Wiring (XIAO ESP32-C3):
 *   Servo 0 (Front-Left)  → D2 / GPIO 4
 *   Servo 1 (Front-Right) → D3 / GPIO 5
 *   Servo 2 (Rear-Left)   → D4 / GPIO 6
 *   Servo 3 (Rear-Right)  → D5 / GPIO 7
 *   All servo power rails → external 5V BEC (NOT the XIAO 3.3V pin)
 *   Common GND between XIAO and servo power supply
 */

#include "ServoController.h"
#include "DroneServer.h"

// ---------------------------------------------------------------------------
// Config
// ---------------------------------------------------------------------------

const int     SERVO_PINS[ServoController::COUNT] = {4, 5, 6, 7};
const int     PWM_MIN_US  = 500;
const int     PWM_MAX_US  = 2400;
const char*   AP_SSID     = "DroneControl";
const char*   AP_PASSWORD = "drone1234";
const IPAddress AP_IP(192, 168, 4, 1);

// ---------------------------------------------------------------------------
// Instances
// ---------------------------------------------------------------------------

ServoController servos(SERVO_PINS, PWM_MIN_US, PWM_MAX_US);
DroneServer     drone(AP_SSID, AP_PASSWORD, AP_IP, servos);

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  servos.begin();
  drone.begin();
}

void loop() {
  drone.loop();
}
