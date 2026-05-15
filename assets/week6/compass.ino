#include <Wire.h>
#include <QMC5883LCompass.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define UPDATE_INTERVAL 100
#define DISPLAY_INTERVAL 500
#define CALIBRATION_TIME 15000

#define CX 40
#define CY 32
#define CR 28

enum class State {
  CALIBRATING,
  RUNNING
};

class CompassDisplay {
private:
  QMC5883LCompass compass;
  Adafruit_SSD1306 display;
  State state;

  unsigned long lastUpdate;
  unsigned long lastDisplayUpdate;
  unsigned long calibrationStart;

  int xMin, xMax, yMin, yMax;
  float xOffset, yOffset, xScale, yScale;

  // --- Calibration ---
  void updateCalibration() {
    compass.read();
    int x = compass.getX();
    int y = compass.getY();

    xMin = min(xMin, x);
    xMax = max(xMax, x);
    yMin = min(yMin, y);
    yMax = max(yMax, y);

    // Send raw XY as JSON for calibration scatter plot
    Serial.print("{\"mode\":\"cal\",\"x\":");
    Serial.print(x);
    Serial.print(",\"y\":");
    Serial.print(y);
    Serial.print(",\"xMin\":");
    Serial.print(xMin);
    Serial.print(",\"xMax\":");
    Serial.print(xMax);
    Serial.print(",\"yMin\":");
    Serial.print(yMin);
    Serial.print(",\"yMax\":");
    Serial.print(yMax);
    Serial.println("}");

    if (millis() - lastDisplayUpdate > DISPLAY_INTERVAL) {
      lastDisplayUpdate = millis();
      int secsLeft = (CALIBRATION_TIME - (millis() - calibrationStart)) / 1000 + 1;
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Calibrating...");
      display.println("Rotate sensor");
      display.println("in all directions");
      display.setTextSize(2);
      display.setCursor(0, 40);
      display.print(secsLeft);
      display.println("s left");
      display.display();
    }

    if (millis() - calibrationStart >= CALIBRATION_TIME) {
      finishCalibration();
    }
  }

  void finishCalibration() {
    if (xMax == xMin) xMax = xMin + 1;
    if (yMax == yMin) yMax = yMin + 1;

    xOffset = (xMax + xMin) / 2.0;
    yOffset = (yMax + yMin) / 2.0;
    xScale  = 2.0 / (xMax - xMin);
    yScale  = 2.0 / (yMax - yMin);

    // Send calibration summary
    Serial.print("{\"mode\":\"cal_done\",\"xOffset\":");
    Serial.print(xOffset);
    Serial.print(",\"yOffset\":");
    Serial.print(yOffset);
    Serial.print(",\"xScale\":");
    Serial.print(xScale);
    Serial.print(",\"yScale\":");
    Serial.print(yScale);
    Serial.println("}");

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Calibration done!");
    display.display();

    state = State::RUNNING;
    lastUpdate = millis();
  }

  // --- Running ---
  int getCalibratedHeading() {
    float x = (compass.getX() - xOffset) * xScale;
    float y = (compass.getY() - yOffset) * yScale;
    int heading = (int)(atan2(y, x) * 180.0 / PI);
    if (heading < 0) heading += 360;
    return heading;
  }

  void drawCompass(int heading) {
    display.drawCircle(CX, CY, CR, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(CX - 3, CY - CR + 2); display.print("N");
    display.setCursor(CX - 3, CY + CR - 9); display.print("S");
    display.setCursor(CX + CR - 8, CY - 4); display.print("E");
    display.setCursor(CX - CR + 2, CY - 4); display.print("W");

    for (int i = 0; i < 8; i++) {
      float angle = radians(i * 45);
      int x1 = CX + (CR - 4) * sin(angle);
      int y1 = CY - (CR - 4) * cos(angle);
      int x2 = CX + CR * sin(angle);
      int y2 = CY - CR * cos(angle);
      display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
    }

    float rad = radians(heading);
    int ax = CX + (CR - 6) * sin(rad);
    int ay = CY - (CR - 6) * cos(rad);
    int tx = CX - (CR - 10) * sin(rad);
    int ty = CY + (CR - 10) * cos(rad);
    display.drawLine(tx, ty, ax, ay, SSD1306_WHITE);

    float perpRad = rad + PI / 2.0;
    int lx = ax - 5 * sin(rad) + 3 * sin(perpRad);
    int ly = ay + 5 * cos(rad) - 3 * cos(perpRad);
    int rx2 = ax - 5 * sin(rad) - 3 * sin(perpRad);
    int ry2 = ay + 5 * cos(rad) + 3 * cos(perpRad);
    display.fillTriangle(ax, ay, lx, ly, rx2, ry2, SSD1306_WHITE);
    display.fillCircle(CX, CY, 2, SSD1306_WHITE);
  }

  void updateRunning() {
    if (millis() - lastUpdate < UPDATE_INTERVAL) return;
    lastUpdate = millis();

    compass.read();
    int rawX = compass.getX();
    int rawY = compass.getY();
    int heading = getCalibratedHeading();

    const char* directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    int index = (heading + 22) / 45 % 8;

    // Send heading data as JSON for line chart
    Serial.print("{\"mode\":\"run\",\"heading\":");
    Serial.print(heading);
    Serial.print(",\"rawX\":");
    Serial.print(rawX);
    Serial.print(",\"rawY\":");
    Serial.print(rawY);
    Serial.print(",\"direction\":\"");
    Serial.print(directions[index]);
    Serial.println("\"}");

    display.clearDisplay();
    drawCompass(heading);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(85, 2);
    display.print(heading);
    display.print((char)247);
    display.setTextSize(2);
    display.setCursor(82, 40);
    display.print(directions[index]);
    display.display();
  }

public:
  CompassDisplay()
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
      state(State::CALIBRATING),
      lastUpdate(0),
      lastDisplayUpdate(0),
      calibrationStart(0),
      xMin(32767), xMax(-32768),
      yMin(32767), yMax(-32768) {}

  void begin() {
    Serial.begin(115200);
    Wire.begin(22, 21);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
      while (1);
    }

    compass.init();
    calibrationStart = millis();

    Serial.println("{\"mode\":\"start\",\"msg\":\"Calibration started\"}");
  }

  void update() {
    switch (state) {
      case State::CALIBRATING: updateCalibration(); break;
      case State::RUNNING:     updateRunning();     break;
    }
  }
};

CompassDisplay compassDisplay;

void setup() { compassDisplay.begin(); }
void loop()  { compassDisplay.update(); }
