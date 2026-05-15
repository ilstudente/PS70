#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

#define I2C_SDA 22
#define I2C_SCL 21
TwoWire I2C_screen = TwoWire(0);

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_screen, OLED_RESET);

// --- Ball ---
float ballX = 64, ballY = 32;
float ballVX = 2.0, ballVY = 1.2;
#define BALL_R    3
#define KICK_DIST 7
#define MAX_SPEED 4.5f
#define MIN_SPEED 1.5f

// --- Goals ---
#define GOAL_DEPTH  5
#define GOAL_HEIGHT 22
#define GOAL_TOP    ((SCREEN_HEIGHT - GOAL_HEIGHT) / 2)
#define GOAL_BOT    (GOAL_TOP + GOAL_HEIGHT)

// --- Score ---
int scoreLeft = 0, scoreRight = 0;

// --- Players ---
#define NUM_PLAYERS  6
#define PLAYER_SPEED 0.9f

struct Player {
  float x, y;
  float targetX, targetY;
  int   team;
  unsigned long nextThink;
};

Player players[NUM_PLAYERS];

void initPlayers() {
  for (int i = 0; i < 3; i++)
    players[i] = { (float)random(10, 55), (float)random(5, 58), 64, 32, 0, 0 };
  for (int i = 3; i < 6; i++)
    players[i] = { (float)random(73, 118), (float)random(5, 58), 64, 32, 1, 0 };
}

float playerBallDist(int i) {
  float dx = players[i].x - ballX;
  float dy = players[i].y - ballY;
  return sqrt(dx * dx + dy * dy);
}

int nearestToBall(int team) {
  int   best = team * 3;
  float minD = playerBallDist(best);
  for (int i = team * 3 + 1; i < team * 3 + 3; i++) {
    float d = playerBallDist(i);
    if (d < minD) { minD = d; best = i; }
  }
  return best;
}

void updatePlayers() {
  unsigned long now = millis();
  int nearLeft  = nearestToBall(0);
  int nearRight = nearestToBall(1);
  float distL = playerBallDist(nearLeft);
  float distR = playerBallDist(nearRight);
  int kicker = (distL <= distR) ? nearLeft : nearRight;

  for (int i = 0; i < NUM_PLAYERS; i++) {
    Player &p = players[i];
    if (now >= p.nextThink) {
      bool chases = (i == nearLeft || i == nearRight);
      if (chases) {
        p.targetX = ballX; p.targetY = ballY; p.nextThink = now + 100;
      } else {
        if (p.team == 0) { p.targetX = random(8,  85); p.targetY = random(5, 58); }
        else             { p.targetX = random(43, 120); p.targetY = random(5, 58); }
        p.nextThink = now + random(400, 900);
      }
    }
    float dx = p.targetX - p.x, dy = p.targetY - p.y;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist > 1.0f) { p.x += (dx / dist) * PLAYER_SPEED; p.y += (dy / dist) * PLAYER_SPEED; }
    p.x = constrain(p.x, 3, SCREEN_WIDTH - 4);
    p.y = constrain(p.y, 3, SCREEN_HEIGHT - 4);
  }

  if (playerBallDist(kicker) < KICK_DIST) {
    float power  = 2.2f + random(0, 18) / 10.0f;
    float spread = (random(0, 2) ? 1 : -1) * (0.4f + random(0, 14) / 10.0f);
    ballVX = (players[kicker].team == 0) ? power : -power;
    ballVY = spread;
  }
}

void drawPlayers() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    int px = (int)players[i].x, py = (int)players[i].y;
    if (players[i].team == 0) {
      display.fillCircle(px, py, 2, SSD1306_WHITE);
    } else {
      display.drawFastHLine(px - 2, py,     5, SSD1306_WHITE);
      display.drawFastVLine(px,     py - 2, 5, SSD1306_WHITE);
    }
  }
}

void drawField() {
  display.drawFastHLine(0, 0,               SCREEN_WIDTH,  SSD1306_WHITE);
  display.drawFastHLine(0, SCREEN_HEIGHT-1, SCREEN_WIDTH,  SSD1306_WHITE);
  display.drawFastVLine(0, 0, GOAL_TOP, SSD1306_WHITE);
  display.drawFastVLine(0, GOAL_BOT, SCREEN_HEIGHT - GOAL_BOT, SSD1306_WHITE);
  display.drawFastVLine(SCREEN_WIDTH-1, 0, GOAL_TOP, SSD1306_WHITE);
  display.drawFastVLine(SCREEN_WIDTH-1, GOAL_BOT, SCREEN_HEIGHT - GOAL_BOT, SSD1306_WHITE);
  display.drawFastHLine(0, GOAL_TOP, GOAL_DEPTH, SSD1306_WHITE);
  display.drawFastHLine(0, GOAL_BOT, GOAL_DEPTH, SSD1306_WHITE);
  display.drawFastVLine(GOAL_DEPTH, GOAL_TOP, GOAL_HEIGHT, SSD1306_WHITE);
  display.drawFastHLine(SCREEN_WIDTH - GOAL_DEPTH, GOAL_TOP, GOAL_DEPTH, SSD1306_WHITE);
  display.drawFastHLine(SCREEN_WIDTH - GOAL_DEPTH, GOAL_BOT, GOAL_DEPTH, SSD1306_WHITE);
  display.drawFastVLine(SCREEN_WIDTH-1-GOAL_DEPTH, GOAL_TOP, GOAL_HEIGHT, SSD1306_WHITE);
  for (int y = 1; y < SCREEN_HEIGHT-1; y += 4)
    display.drawFastVLine(SCREEN_WIDTH/2, y, 2, SSD1306_WHITE);
  display.drawCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 10, SSD1306_WHITE);
}

void drawScore() {
  display.setTextSize(1); display.setTextColor(SSD1306_WHITE);
  display.setCursor(52, 3);
  display.print(scoreLeft); display.print(F("-")); display.print(scoreRight);
}

void goalFlash(bool rightScored) {
  for (int f = 0; f < 3; f++) {
    display.clearDisplay();
    display.setTextSize(3); display.setTextColor(SSD1306_WHITE);
    display.setCursor(14, 5); display.print(F("GOAL!"));
    display.setTextSize(2); display.setCursor(30, 42);
    display.print(scoreLeft); display.print(F(" - ")); display.print(scoreRight);
    display.display(); delay(400);
    display.clearDisplay(); display.display(); delay(200);
  }
  delay(500);
  ballX = SCREEN_WIDTH / 2; ballY = SCREEN_HEIGHT / 2;
  ballVX = rightScored ? -2.0f : 2.0f;
  ballVY = (random(0,2) ? 1 : -1) * (0.8f + random(0,12) / 10.0f);
  initPlayers();
}

void setup() {
  Serial.begin(115200);
  I2C_screen.begin(I2C_SDA, I2C_SCL, 400000);
  delay(500);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay(); display.display();
  randomSeed(analogRead(0));
  initPlayers();
}

void loop() {
  updatePlayers();
  display.clearDisplay();
  ballX += ballVX; ballY += ballVY;

  float speed = sqrt(ballVX * ballVX + ballVY * ballVY);
  if (speed > MAX_SPEED) { ballVX = ballVX/speed*MAX_SPEED; ballVY = ballVY/speed*MAX_SPEED; }
  if (speed < MIN_SPEED && speed > 0) { ballVX = ballVX/speed*MIN_SPEED; ballVY = ballVY/speed*MIN_SPEED; }

  if (ballY - BALL_R <= 1)               { ballY = 1 + BALL_R;               ballVY =  abs(ballVY); }
  if (ballY + BALL_R >= SCREEN_HEIGHT-2) { ballY = SCREEN_HEIGHT-2 - BALL_R; ballVY = -abs(ballVY); }

  if (ballX - BALL_R <= 1) {
    if (ballY > GOAL_TOP && ballY < GOAL_BOT) { scoreRight++; goalFlash(true); return; }
    ballX = 1 + BALL_R; ballVX = abs(ballVX);
  }
  if (ballX + BALL_R >= SCREEN_WIDTH-2) {
    if (ballY > GOAL_TOP && ballY < GOAL_BOT) { scoreLeft++; goalFlash(false); return; }
    ballX = SCREEN_WIDTH-2 - BALL_R; ballVX = -abs(ballVX);
  }

  drawField(); drawScore(); drawPlayers();
  display.drawCircle((int)ballX, (int)ballY, BALL_R, SSD1306_WHITE);
  display.drawPixel((int)ballX,  (int)ballY,          SSD1306_WHITE);
  display.display();
  delay(16);
}