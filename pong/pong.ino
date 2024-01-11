#include <OneWire.h>

#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <EEPROM.h>

/*
  A simple Pong game.
  https://notabug.org/Maverick/WokwiPong
  
  Based on Arduino Pong by eholk
  https://github.com/eholk/Arduino-Pong
*/
// SCL: D22
// SDA: D21

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>

#define SD_PIN 5

#define B0 31
#define C1 33
#define CS1 35
#define D1 37
#define DS1 39
#define E1 41
#define F1 44
#define FS1 46
#define G1 49
#define GS1 52
#define A1 55
#define AS1 58
#define B1 62
#define C2 65
#define CS2 69
#define D2 73
#define DS2 78
#define E2 82
#define F2 87
#define FS2 93
#define G2 98
#define GS2 104
#define A2 110
#define AS2 117
#define B2 123
#define C3 131
#define CS3 139
#define D3 147
#define DS3 156
#define E3 165
#define F3 175
#define FS3 185
#define G3 196
#define GS3 208
#define A3 220
#define AS3 233
#define B3 247
#define C4 262
#define CS4 277
#define D4 294
#define DS4 311
#define E4 330
#define F4 349
#define FS4 370
#define G4 392
#define GS4 415
#define A4 440
#define AS4 466
#define B4 494
#define C5 523
#define CS5 554
#define D5 587
#define DS5 622
#define E5 659
#define F5 698
#define FS5 740
#define G5 784
#define GS5 831
#define A5 880
#define AS5 932
#define B5 988
#define C6 1047
#define CS6 1109
#define D6 1175
#define DS6 1245
#define E6 1319
#define F6 1397
#define FS6 1480
#define G6 1568
#define GS6 1661
#define A6 1760
#define AS6 1865
#define B6 1976
#define C7 2093
#define CS7 2217
#define D7 2349
#define DS7 2489
#define E7 2637
#define F7 2794
#define FS7 2960
#define G7 3136
#define GS7 3322
#define A7 3520
#define AS7 3729
#define B7 3951
#define C8 4186
#define CS8 4435
#define D8 4699
#define DS8 4978

#define UP_BUTTON 26
#define DOWN_BUTTON 27

#define UP_BUTTON_MCU 12
#define DOWN_BUTTON_MCU 13

#define H 175
#define B H * 2
#define W B * 2

const unsigned long PADDLE_RATE = 64;

const unsigned long BALL_RATE = 10;
const uint8_t PADDLE_HEIGHT = 12;
const uint8_t SCORE_LIMIT = 2;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

bool game_over, win;

uint8_t player_score, mcu_score;
uint8_t ball_x = 53, ball_y = 26;
uint8_t ball_dir_x = 1, ball_dir_y = 1;

unsigned long ball_update;
unsigned long paddle_player_update, paddle_mcu_update;

const uint8_t MCU_X = 12;
uint8_t mcu_y = 16;

const uint8_t PLAYER_X = 115;
uint8_t player_y = 16;

// notes in the melody:
int winning_melody[] = {
  D4, G4, G4, A4, G4, FS4, E4, E4, E4, A4, A4, B4, A4, G4, FS4, D4, D4, B4, B4, C5, B4, A4, G4, E4, D4, D4, E4, A4, FS4, G4
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int winning_noteDurations[] = {
  B, B, H, H, H, H, B, B, B, B, H, H, H, H, B, B, B, B, H, H, H, H, B, B, H, H, B, B, B, W
};

void changetune(int melody[], int duration[]) {
  for (int i = 0; i < 30; ++i) {
    winning_melody[i] = melody[i];
    winning_noteDurations[i] = duration[i];
  }
}

bool readFile(const char *filePath) {
  File file = SD.open(filePath);
  int maxSize = 30;

  if (file) {
    Serial.println(filePath);
    for (int i = 0; i < maxSize; ++i) {
      winning_melody[i] = file.parseInt();
    }
    for (int i = 0; i < maxSize; ++i) {
      winning_noteDurations[i] = file.parseInt();
    }
    file.close();
    return true;
  } else {
    Serial.println("Error opening file.");
    return false;
  }
}

void listFiles(const char *dirPath) {
  Serial.println("Listing files in directory: " + String(dirPath));
  File dir = SD.open(dirPath);

  if (dir) {
    while (File entry = dir.openNextFile()) {
      Serial.print("  ");
      Serial.println(entry.name());
      entry.close();
    }
    dir.close();
  } else {
    Serial.println("Error opening directory.");
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(SD_PIN, OUTPUT);

  if (!SD.begin(SD_PIN)) {
    Serial.println("SD card initialization failed. Make sure the card is properly connected and formatted.");
    return;
  } else {
    Serial.println("SD card init success");
  }

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // Display the splash screen (we're legally required to do so)
  display.display();

  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);

  pinMode(UP_BUTTON_MCU, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_MCU, INPUT_PULLUP);

  select_tune();



  display.clearDisplay();
  drawCourt();

  display.display();

  ball_update = millis();
  paddle_player_update = ball_update;
  paddle_mcu_update = ball_update;
}


int start_press_u = 0, start_press_d = 0, start_press_umcu = 0, start_press_dmcu = 0;

void select_tune() {
  display.clearDisplay();
  display.setCursor(20, 14);
  String text = "loading...";
  display.print(text);
  display.display();
  delay(5000);
  display.clearDisplay();
  display.setCursor(20, 14);
  text = "Press button to select tune!";
  display.print(text);
  display.display();
  int max_tune = 0;

  File dir = SD.open("/");
  if (!dir) return;
  while (File entry = dir.openNextFile()) {
    Serial.print("  ");
    Serial.println(entry.name());
    entry.close();
    max_tune += 1;
  }
  max_tune -= 1;
  dir.close();

  int tune_id = 1;

  display.setTextColor(WHITE, BLACK);

  display.clearDisplay();
  display.setCursor(20, 14);
  text = "Select tune: " + String(tune_id);
  display.print(text);
  display.display();
  while (true) {
    static unsigned long lastDebounceTimeUp = 0;
    static unsigned long lastDebounceTimeDown = 0;
    static const int debounceDelay = 50;  // milliseconds

    bool button_state_up = (digitalRead(UP_BUTTON) == LOW);
    bool button_state_down = (digitalRead(DOWN_BUTTON) == LOW);

    // Debounce UP_BUTTON
    if (button_state_up && (millis() - lastDebounceTimeUp) > debounceDelay) {
      tune_id = (tune_id % max_tune) + 1;
      display.setTextColor(WHITE, BLACK);

      display.clearDisplay();
      display.setCursor(20, 14);
      String text = "Select tune: " + String(tune_id);
      display.print(text);
      display.display();

      lastDebounceTimeUp = millis();
    }

    // Debounce DOWN_BUTTON
    if (button_state_down && (millis() - lastDebounceTimeDown) > debounceDelay) {
      char id[4] = "123";  // Assuming tune_id won't be larger than 999
      char path[10] = "/";
      itoa(tune_id, id, 10);
      strcat(id, ".txt");
      strcat(path, id);
      readFile(path);
      winTone();
      display.clearDisplay();
      break;

      lastDebounceTimeDown = millis();
    }

    delay(10);
  }
}
void loop() {
  bool update_needed = false;

  unsigned long time = millis();

  static bool up_state = false;
  static bool down_state = false;

  static bool up_state_mcu = false;
  static bool down_state_mcu = false;

  up_state |= (digitalRead(UP_BUTTON) == LOW);
  down_state |= (digitalRead(DOWN_BUTTON) == LOW);

  up_state_mcu |= (digitalRead(UP_BUTTON_MCU) == LOW);
  down_state_mcu |= (digitalRead(DOWN_BUTTON_MCU) == LOW);

  if (!up_state) start_press_u = time;
  if (!down_state) start_press_d = time;
  if (!up_state_mcu) start_press_umcu = time;
  if (!down_state_mcu) start_press_dmcu = time;

  if (time - start_press_u > 2000 && time - start_press_d > 2000 && time - start_press_umcu > 2000 && time - start_press_dmcu > 2000) {

    select_tune();

    drawCourt();
    start_press_u = time;
    start_press_d = time;
    start_press_umcu = time;
    start_press_dmcu = time;
  }

  if (time > ball_update) {
    uint8_t new_x = ball_x + ball_dir_x;
    uint8_t new_y = ball_y + ball_dir_y;

    // Check if we hit the vertical walls
    if (new_x == 0 || new_x == 127) {
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;

      if (new_x < 64) {
        player_scoreTone();
        player_score++;
      } else {
        mcu_scoreTone();
        mcu_score++;
      }

      if (player_score == SCORE_LIMIT || mcu_score == SCORE_LIMIT) {
        win = player_score > mcu_score;
        game_over = true;
      }
    }

    // Check if we hit the horizontal walls.
    if (new_y == 0 || new_y == 53) {
      wallTone();
      ball_dir_y = -ball_dir_y;
      new_y += ball_dir_y + ball_dir_y;
    }

    // Check if we hit the CPU paddle
    if (new_x == MCU_X && new_y >= mcu_y && new_y <= mcu_y + PADDLE_HEIGHT) {
      mcuPaddleTone();
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;
    }

    // Check if we hit the player paddle
    if (new_x == PLAYER_X && new_y >= player_y && new_y <= player_y + PADDLE_HEIGHT) {
      playerPaddleTone();
      ball_dir_x = -ball_dir_x;
      new_x += ball_dir_x + ball_dir_x;
    }

    display.drawPixel(ball_x, ball_y, BLACK);
    display.drawPixel(new_x, new_y, WHITE);
    ball_x = new_x;
    ball_y = new_y;

    ball_update += BALL_RATE;

    update_needed = true;
  }


  if (time > paddle_mcu_update) {
    paddle_mcu_update += PADDLE_RATE;
    display.drawFastVLine(MCU_X, mcu_y, PADDLE_HEIGHT, BLACK);

    if (up_state_mcu)
      mcu_y -= 1;

    if (down_state_mcu)
      mcu_y += 1;

    up_state_mcu = false;
    down_state_mcu = false;

    if (mcu_y < 1)
      mcu_y = 1;

    if (mcu_y + PADDLE_HEIGHT > 53)
      mcu_y = 53 - PADDLE_HEIGHT;

    display.drawFastVLine(MCU_X, mcu_y, PADDLE_HEIGHT, WHITE);
    update_needed = true;
  }

  if (time > paddle_player_update) {
    paddle_player_update += PADDLE_RATE;
    display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, BLACK);

    if (up_state)
      player_y -= 1;

    if (down_state)
      player_y += 1;

    up_state = false;
    down_state = false;

    if (player_y < 1)
      player_y = 1;

    if (player_y + PADDLE_HEIGHT > 53)
      player_y = 53 - PADDLE_HEIGHT;

    display.drawFastVLine(PLAYER_X, player_y, PADDLE_HEIGHT, WHITE);
    update_needed = true;
  }


  if (update_needed) {
    if (game_over) {
      String text;
      if (win) {
        text = "Player 2 Won\n\n        Score " + String(mcu_score) + " - " + String(player_score);
      } else {
        text = "Player 1 Won\n\n        Score " + String(mcu_score) + " - " + String(player_score);
      }
      // const char* text = win ? "YOU WIN!!" : "YOU LOSE!";
      display.clearDisplay();
      display.setCursor(40, 28);
      display.print(text);
      display.display();

      winTone();

      display.clearDisplay();
      ball_x = 53;
      ball_y = 26;
      ball_dir_x = 1;
      ball_dir_y = 1;
      mcu_y = 16;
      player_y = 16;
      mcu_score = 0;
      player_score = 0;
      game_over = false;
      drawCourt();
    }

    display.setTextColor(WHITE, BLACK);
    display.setCursor(0, 56);
    display.print(mcu_score);
    display.setCursor(110, 56);
    display.print(player_score);
    display.display();
  }
}

void playerPaddleTone() {
  tone(2, 250, 25);
  delay(25);
  noTone(2);
}

void mcuPaddleTone() {
  tone(2, 225, 25);
  delay(25);
  noTone(2);
}

void wallTone() {
  tone(2, 200, 25);
  delay(25);
  noTone(2);
}

void player_scoreTone() {
  tone(2, 200, 25);
  delay(50);
  noTone(2);
  delay(25);
  tone(2, 250, 25);
  delay(25);
  noTone(2);
}

void mcu_scoreTone() {
  tone(2, 250, 25);
  delay(25);
  noTone(2);
  delay(25);
  tone(2, 200, 25);
  delay(25);
  noTone(2);
}

void drawCourt() {
  display.drawRect(0, 0, 128, 54, WHITE);
}

void winTone() {
  for (int thisNote = 0; thisNote < 30; thisNote++) {
    tone(2, winning_melody[thisNote], winning_noteDurations[thisNote]);
    delay(winning_noteDurations[thisNote]);
    noTone(2);
    delay(5);
  }
}
