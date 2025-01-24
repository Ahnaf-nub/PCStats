#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "gif.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

#define BUFFER_SIZE 256
char usb_buffer[BUFFER_SIZE];

String current_cpu_usage = "0";
String current_ram_usage = "0";
String current_disk_usage = "0";
String current_time = "";
String current_day = "";

#define BUZZER_PIN D10
#define BUTTON_PIN D0

// Modes
#define MODE_STATS 0
#define MODE_GIF 1
#define MODE_POMODORO 2
int currentMode = MODE_STATS;

// Prevent quick switching to GIF mode
unsigned long statsDisplayStartTime = 0;
const unsigned long statsDisplayDuration = 5000; // Display stats for at least 5 seconds

unsigned long pomodoroStartTime = 0;
const unsigned long pomodoroDuration = 25 * 60 * 1000;

// GIF variables
int gifFrameIndex = 0;
unsigned long lastGifFrameTime = 0;
const unsigned char **frames = epd_bitmap_allArray;
const int frameCount = sizeof(epd_bitmap_allArray) / sizeof(epd_bitmap_allArray[0]);

void playGIFNonBlocking() {
    if (millis() - lastGifFrameTime >= 46) {
        lastGifFrameTime = millis();

        if (gifFrameIndex < frameCount) {
            display.clearDisplay();
            display.drawBitmap(0, 0, frames[gifFrameIndex], SCREEN_WIDTH, SCREEN_HEIGHT, SH110X_WHITE);
            display.display();
            gifFrameIndex++;
        } else {
            gifFrameIndex = 0;  // Restart GIF
        }
    }
}

void updateOLED() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(15, 0);
    display.println("CPU Usage: " + current_cpu_usage + " %");
    display.setCursor(15, 10);
    display.println("RAM Usage: " + current_ram_usage + " %");
    display.setCursor(15, 20);
    display.println("Disk Usage: " + current_disk_usage + " %");

    display.setCursor(5, 40);
    display.print(current_time);
    display.print(" ");
    display.println(current_time.toInt() < 12 ? "AM" : "PM");

    display.setCursor((SCREEN_WIDTH - current_day.length() * 6) / 2, 55);
    display.println(current_day);
    display.display();
}

void handleBuzzer(const String &ram_usage) {
    int ramUsageInt = ram_usage.toInt();
    if (ramUsageInt > 95) {
        tone(BUZZER_PIN, 1000, 500); // Play a tone at 1000 Hz for 500ms
    } else {
        noTone(BUZZER_PIN);
    }
}

void parseSystemStats(const char *data) {
    String received = String(data);
    Serial.print("Received Data: ");
    Serial.println(received);

    int separator[5];
    separator[0] = received.indexOf(',');
    for (int i = 1; i < 5; i++) {
        separator[i] = received.indexOf(',', separator[i - 1] + 1);
        if (separator[i] == -1) {
            Serial.println("Error: Malformed input!");
            return;
        }
    }

    current_cpu_usage = received.substring(0, separator[0]);
    current_ram_usage = received.substring(separator[1] + 1, separator[2]);
    current_disk_usage = received.substring(separator[2] + 1, separator[3]);
    current_time = received.substring(separator[3] + 1, separator[4]);
    current_day = received.substring(separator[4] + 1);

    currentMode = MODE_STATS; // Switch to stats mode
    statsDisplayStartTime = millis(); // Reset stats display timer
    updateOLED();
    handleBuzzer(current_ram_usage);
}

void handleButtonPress() {
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 200;

    if ((millis() - lastDebounceTime) > debounceDelay && digitalRead(BUTTON_PIN) == HIGH) {
        lastDebounceTime = millis();

        if (currentMode != MODE_POMODORO) {
            currentMode = MODE_POMODORO;
            pomodoroStartTime = millis();
        } else {
            currentMode = MODE_STATS;
        }
    }
}

void updatePomodoro() {
    unsigned long elapsedTime = millis() - pomodoroStartTime;
    if (elapsedTime > pomodoroDuration) elapsedTime = pomodoroDuration;

    unsigned long remainingTime = pomodoroDuration - elapsedTime;
    int minutes = remainingTime / 60000;
    int seconds = (remainingTime % 60000) / 1000;
    int progress = (elapsedTime * SCREEN_WIDTH) / pomodoroDuration;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);

    // Center Pomodoro text
    display.setCursor((SCREEN_WIDTH - 14 * 6) / 2, 0); // "Pomodoro Timer" is 14 characters
    display.println("Pomodoro Timer");

    // Center Timer
    char timeBuffer[6];
    sprintf(timeBuffer, "%02d:%02d", minutes, seconds);
    display.setCursor((SCREEN_WIDTH - strlen(timeBuffer) * 6) / 2, 20);
    display.println(timeBuffer);

    // Draw progress bar
    display.drawRect(0, 40, SCREEN_WIDTH, 10, SH110X_WHITE);
    display.fillRect(0, 40, progress, 10, SH110X_WHITE);
    display.display();

    if (remainingTime == 0) {
        tone(BUZZER_PIN, 1000, 1500);
    }
}

void setup() {
    Wire.begin();
    Serial.begin(9600);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    if (!display.begin(0x3C)) {
        Serial.println(F("OLED initialization failed!"));
        while (1);
    }

    currentMode = MODE_STATS;
    statsDisplayStartTime = millis();
}

void loop() {
    handleButtonPress();

    switch (currentMode) {
        case MODE_POMODORO:
            updatePomodoro();
            break;

        case MODE_STATS:
            if (Serial.available()) {
                int len = Serial.readBytesUntil('\n', usb_buffer, BUFFER_SIZE - 1);
                usb_buffer[len] = '\0';
                parseSystemStats(usb_buffer);
            }

            if (millis() - statsDisplayStartTime < statsDisplayDuration) {
                updateOLED();
            } else {
                currentMode = MODE_GIF; // Switch to GIF mode after showing stats for 5 seconds
            }
            break;

        case MODE_GIF:
            playGIFNonBlocking();
            break;
    }

    delay(10); // Prevent CPU overwork
}