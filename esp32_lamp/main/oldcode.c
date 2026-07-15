#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
// =====================================================
// OLED SETTINGS
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
// =====================================================
// PIN SETTINGS
// =====================================================
const int START_BUTTON_PIN = 25;
const int RESET_BUTTON_PIN = 33;
const int BUZZER_PIN = 26;   // optional active buzzer
 
// =====================================================
// TIMER SETTINGS
// =====================================================
// Demo values for presentation.
// Later you can change them to 25 * 60 and 5 * 60.
const int FOCUS_SECONDS = 60;
const int BREAK_SECONDS = 20;
 
// =====================================================
// TASK DATA
// =====================================================
String currentTask = "Finish Slides";
 
// =====================================================
// WIFI WEB SERVER SETTINGS
// =====================================================
const char* ssid = "SmartLamp_AP";
const char* password = "12345678";
 
WebServer server(80);
 
// =====================================================
// TIMER STATE MACHINE
// =====================================================
enum TimerState {
  IDLE,
  FOCUS,
  PAUSED,
  BREAK_TIME,
  DONE
};
 
TimerState state = IDLE;
TimerState previousRunningState = FOCUS;
 
int remainingSeconds = FOCUS_SECONDS;
 
// =====================================================
// TIMING VARIABLES
// =====================================================
unsigned long lastTickTime = 0;
unsigned long lastDisplayUpdate = 0;
 
// BUTTON DEBOUNCE
const unsigned long debounceDelay = 50;
 
// Start/Pause button
bool lastStartReading = HIGH;
bool stableStartState = HIGH;
unsigned long lastStartChange = 0;
 
// Reset button
bool lastResetReading = HIGH;
bool stableResetState = HIGH;
unsigned long lastResetChange = 0;
 
// SETUP
void setup() {
  Serial.begin(115200);
 
  // Buttons
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
 
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
 
  // OLED I2C
  Wire.begin(21, 22);
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found. Try OLED address 0x3D.");
    while (true);
  }
 
  display.clearDisplay();
  display.display();
 
  // Wi-Fi AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
 
  Serial.println("Wi-Fi Access Point started");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
 
  // Web routes
  server.on("/", handleRoot);
  server.on("/setTask", handleSetTask);
  server.on("/reset", handleWebReset);
 
  server.begin();
  Serial.println("Web server started");
 
  showScreen();
}
 
// =====================================================
// LOOP
// =====================================================
void loop() {
  server.handleClient();
 
  handleStartButton();
  handleResetButton();
 
  updateTimer();
  showScreen();
}
 
// =====================================================
// WEB PAGE: MAIN PAGE
// =====================================================
void handleRoot() {
  String html = "";
 
  html += "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Smart Study Lamp</title>";
 
  html += "<style>";
  html += "body { font-family: Arial; background:#f5f5f5; padding:20px; }";
  html += ".card { background:white; padding:20px; border-radius:12px; max-width:420px; margin:auto; box-shadow:0 2px 8px rgba(0,0,0,0.15); }";
  html += "h2 { margin-top:0; }";
  html += "input[type=text] { width:100%; padding:12px; font-size:16px; margin-top:8px; margin-bottom:12px; box-sizing:border-box; }";
  html += "input[type=submit], button { width:100%; padding:12px; font-size:16px; border:0; border-radius:8px; background:#2563eb; color:white; }";
  html += ".info { background:#eef2ff; padding:10px; border-radius:8px; margin:10px 0; }";
  html += ".small { color:#555; font-size:14px; }";
  html += "</style>";
 
  html += "</head>";
  html += "<body>";
  html += "<div class='card'>";
 
  html += "<h2>Smart Study Lamp</h2>";
 
  html += "<div class='info'>";
  html += "<b>Current Task:</b><br>";
  html += currentTask;
  html += "</div>";
 
  html += "<div class='info'>";
  html += "<b>Mode:</b> ";
  html += getStateText();
  html += "<br><b>Time:</b> ";
  html += formatTime(remainingSeconds);
  html += "</div>";
 
  html += "<form action='/setTask' method='GET'>";
  html += "<label>Enter task before session:</label>";
  html += "<input type='text' name='task' maxlength='24' placeholder='e.g. Finish ELEC3117 slides'>";
  html += "<input type='submit' value='Update Task'>";
  html += "</form>";
 
  html += "<br>";
  html += "<form action='/reset' method='GET'>";
  html += "<button type='submit'>Reset Timer</button>";
  html += "</form>";
 
  html += "<p class='small'>";
  html += "Phone is used only for pre-session task input. Timer control is local using physical buttons.";
  html += "</p>";
 
  html += "</div>";
  html += "</body>";
  html += "</html>";
 
  server.send(200, "text/html", html);
}
 
// =====================================================
// WEB PAGE: SET TASK
// =====================================================
void handleSetTask() {
  if (server.hasArg("task")) {
    String newTask = server.arg("task");
    newTask.trim();
 
    if (newTask.length() == 0) {
      newTask = "No Task";
    }
 
    // Keep task short for OLED readability
    if (newTask.length() > 24) {
      newTask = newTask.substring(0, 24);
    }
 
    currentTask = newTask;
 
    Serial.print("Task updated from phone: ");
    Serial.println(currentTask);
 
    // After setting task, keep timer in IDLE ready state
    state = IDLE;
    previousRunningState = FOCUS;
    remainingSeconds = FOCUS_SECONDS;
  }
 
  server.sendHeader("Location", "/");
  server.send(303);
}
 
// =====================================================
// WEB PAGE: RESET
// =====================================================
void handleWebReset() {
  resetTimer();
  server.sendHeader("Location", "/");
  server.send(303);
}
 
// =====================================================
// START / PAUSE BUTTON
// =====================================================
void handleStartButton() {
  bool reading = digitalRead(START_BUTTON_PIN);
 
  if (reading != lastStartReading) {
    lastStartChange = millis();
  }
 
  if ((millis() - lastStartChange) > debounceDelay) {
    if (reading != stableStartState) {
      stableStartState = reading;
 
      // INPUT_PULLUP: LOW = pressed
      if (stableStartState == LOW) {
        onStartButtonPressed();
      }
    }
  }
 
  lastStartReading = reading;
}
 
void onStartButtonPressed() {
  Serial.println("Start/Pause button pressed");
 
  if (state == IDLE) {
    state = FOCUS;
    previousRunningState = FOCUS;
    remainingSeconds = FOCUS_SECONDS;
    lastTickTime = millis();
  }
  else if (state == FOCUS || state == BREAK_TIME) {
    previousRunningState = state;
    state = PAUSED;
  }
  else if (state == PAUSED) {
    state = previousRunningState;
    lastTickTime = millis();
  }
  else if (state == DONE) {
    state = IDLE;
    previousRunningState = FOCUS;
    remainingSeconds = FOCUS_SECONDS;
  }
}
 
// =====================================================
// RESET BUTTON
// =====================================================
void handleResetButton() {
  bool reading = digitalRead(RESET_BUTTON_PIN);
 
  if (reading != lastResetReading) {
    lastResetChange = millis();
  }
 
  if ((millis() - lastResetChange) > debounceDelay) {
    if (reading != stableResetState) {
      stableResetState = reading;
 
      // INPUT_PULLUP: LOW = pressed
      if (stableResetState == LOW) {
        resetTimer();
      }
    }
  }
 
  lastResetReading = reading;
}
 
void resetTimer() {
  Serial.println("Reset triggered");
 
  state = IDLE;
  previousRunningState = FOCUS;
  remainingSeconds = FOCUS_SECONDS;
  lastTickTime = millis();
 
  shortBeep();
}
 
// =====================================================
// TIMER UPDATE
// =====================================================
void updateTimer() {
  if (state != FOCUS && state != BREAK_TIME) {
    return;
  }
 
  if (millis() - lastTickTime >= 1000) {
    lastTickTime += 1000;
 
    if (remainingSeconds > 0) {
      remainingSeconds--;
    }
 
    Serial.print("Mode: ");
    Serial.print(getStateText());
    Serial.print(" | Task: ");
    Serial.print(currentTask);
    Serial.print(" | Time left: ");
    Serial.println(formatTime(remainingSeconds));
 
    if (remainingSeconds <= 0) {
      if (state == FOCUS) {
        alertBeep();
 
        // Mandatory break after each focus task
        state = BREAK_TIME;
        previousRunningState = BREAK_TIME;
        remainingSeconds = BREAK_SECONDS;
        lastTickTime = millis();
      }
      else if (state == BREAK_TIME) {
        alertBeep();
        state = DONE;
      }
    }
  }
}
 
// =====================================================
// OLED DISPLAY
// =====================================================
void showScreen() {
  // Limit OLED refresh rate
  if (millis() - lastDisplayUpdate < 200) {
    return;
  }
 
  lastDisplayUpdate = millis();
 
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
 
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SMART STUDY LAMP");
 
  // Mode
  display.setCursor(0, 12);
  display.print("Mode: ");
  display.println(getStateText());
 
  // Task
  display.setCursor(0, 24);
  display.print("Task:");
  display.setCursor(0, 34);
  display.println(currentTask);
 
  // Timer
  display.setTextSize(2);
  display.setCursor(20, 48);
  display.println(formatTime(remainingSeconds));
 
  display.display();
}
 
// =====================================================
// HELPER FUNCTIONS
// =====================================================
String getStateText() {
  switch (state) {
    case IDLE:
      return "IDLE";
    case FOCUS:
      return "FOCUS";
    case PAUSED:
      return "PAUSED";
    case BREAK_TIME:
      return "BREAK";
    case DONE:
      return "DONE";
    default:
      return "UNKNOWN";
  }
}
 
String formatTime(int totalSeconds) {
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
 
  String result = "";
 
  if (minutes < 10) result += "0";
  result += String(minutes);
  result += ":";
 
  if (seconds < 10) result += "0";
  result += String(seconds);
 
  return result;
}
 
// =====================================================
// BUZZER FUNCTIONS
// =====================================================
void shortBeep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(80);
  digitalWrite(BUZZER_PIN, LOW);
}
 
void alertBeep() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}
 