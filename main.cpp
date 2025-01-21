#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// SH1106 OLED display setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

// Buffer for incoming data
#define BUFFER_SIZE 256
char usb_buffer[BUFFER_SIZE];

// Current stats
String current_cpu_temp = "0";
String current_mem_free = "0";
String current_cpu_usage = "0";
String current_ram_usage = "0";
String current_disk_usage = "0";
String current_time = "";

// Buzzer and button setup
#define BUZZER_PIN D10
#define BUTTON_PIN D0
bool showPomodoro = false;
unsigned long pomodoroStartTime = 0;
const unsigned long pomodoroDuration = 25 * 60 * 1000; // 25 minutes

// Update OLED with stats
void updateOLED(const String &cpu_temp, const String &cpu_usage, const String &ram_usage, const String &disk_usage, const String &time) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("CPU Temp: " + cpu_temp + " C");
    display.println("CPU Usage: " + cpu_usage + " %");
    display.println("RAM Usage: " + ram_usage + " %");
    display.println("Disk Usage: " + disk_usage + " %");
    display.println("Time: " + time);
    display.display();
}

// Handle buzzer for high RAM usage
void handleBuzzer(const String &ram_usage) {
    int ramUsageInt = ram_usage.toInt();
    if (ramUsageInt > 80) { // Threshold for high RAM usage
        digitalWrite(BUZZER_PIN, HIGH);
    } else {
        digitalWrite(BUZZER_PIN, LOW);
    }
}

// Parse system stats from the PC
void parseSystemStats(const char *data) {
    String received = String(data);
    int separator1 = received.indexOf(',');
    int separator2 = received.indexOf(',', separator1 + 1);
    int separator3 = received.indexOf(',', separator2 + 1);
    int separator4 = received.indexOf(',', separator3 + 1);
    int separator5 = received.indexOf(',', separator4 + 1);
    int separator6 = received.indexOf(',', separator5 + 1);

    if (separator1 != -1 && separator2 != -1 && separator3 != -1 && separator4 != -1 && separator5 != -1 && separator6 != -1) {
        String cpu_usage = received.substring(0, separator1);
        String mem_total = received.substring(separator2 + 1, separator3);
        String ram_usage = received.substring(separator3 + 1, separator4);
        String disk_usage = received.substring(separator4 + 1, separator5);
        String time = received.substring(separator5 + 1, separator6);
        String cpu_temp = received.substring(separator6 + 1);

        // Update OLED and NeoPixel only if values change
        if (cpu_temp != current_cpu_temp || cpu_usage != current_cpu_usage || ram_usage != current_ram_usage || disk_usage != current_disk_usage || time != current_time) {
            current_cpu_temp = cpu_temp;
            current_cpu_usage = cpu_usage;
            current_ram_usage = ram_usage;
            current_disk_usage = disk_usage;
            current_time = time;

            updateOLED(cpu_temp, cpu_usage, ram_usage, disk_usage, time);
            handleBuzzer(ram_usage);
        }
    }
}

// Handle button press to switch display
void handleButtonPress() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        showPomodoro = !showPomodoro;
        if (showPomodoro) {
            pomodoroStartTime = millis();
        }
        delay(200); // Debounce delay
    }
}

// Update OLED with Pomodoro timer
void updatePomodoro() {
    unsigned long elapsedTime = millis() - pomodoroStartTime;
    unsigned long remainingTime = pomodoroDuration - elapsedTime;
    int minutes = remainingTime / 60000;
    int seconds = (remainingTime % 60000) / 1000;
    int progress = (elapsedTime * SCREEN_WIDTH) / pomodoroDuration;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("Pomodoro Timer");
    display.setCursor(0, 10);
    display.print("Time: ");
    display.print(minutes);
    display.print(":");
    if (seconds < 10) {
        display.print("0");
    }
    display.print(seconds);
    display.drawRect(0, 30, SCREEN_WIDTH, 10, SH110X_WHITE);
    display.fillRect(0, 30, progress, 10, SH110X_WHITE);
    display.display();
}

void setup() {
    Wire.begin(D4, D5);

    if (!display.begin(0x3C)) {
        while (true) {
        }
    }
    display.clearDisplay();
    display.display();
    Serial.begin(9600);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    while (!Serial) {
        display.clearDisplay();
        display.setTextSize(2); // Increase text size
        display.setTextColor(SH110X_WHITE);
        for (int i = 0; i < SCREEN_WIDTH; i += 12) { // Adjust spacing for larger text
            display.setCursor(i, SCREEN_HEIGHT / 2 - 8); // Adjust vertical position for larger text
            display.print("Welcome!");
            display.display();
            delay(100);
            display.clearDisplay();
        }
        display.setCursor((SCREEN_WIDTH - 12 * 8) / 2, SCREEN_HEIGHT / 2 - 8); // Adjust position for larger text
        display.print("Welcome!");
        display.display();
    }
}

void loop() {
    handleButtonPress();
    if (showPomodoro) {
        updatePomodoro();
    } else {
        if (Serial.available()) {
            int len = Serial.readBytesUntil('\n', usb_buffer, BUFFER_SIZE - 1);
            usb_buffer[len] = '\0'; // Null-terminate the string
            parseSystemStats(usb_buffer);
        }
    }
    delay(10); // Prevent CPU overwork
}