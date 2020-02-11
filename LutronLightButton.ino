// include SPI, MP3 and SD libraries
#include <WiFi101.h>
#include "secrets.h"

WiFiClient client;
IPAddress lutronBridge(192,168,86,43);

// TODO: Read light level to toggle
bool lightState = false;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

unsigned long lastDebounceTime = 0;  // the last time the output was toggled
unsigned long debounceDelay = 50;    // the debounce time

const int BUTTON_PIN = A0;

void log(String msg) {
  Serial.println(msg);  
}

void setup() {
  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8,7,4,2);

  //configure pin as an input and enable the internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  connect();
}

bool connect() {
  if (client.connected())
    return true;

  // Attempt to connect to WiFi network
  if (WiFi.status() != WL_CONNECTED) {
    log(String("Trying to connect to SSID: ") + SECRET_SSID);
    int status = WiFi.begin(SECRET_SSID, SECRET_PASS);
    if (status == WL_CONNECTED) {
      log(String("Connected to WiFi, RSSI:") + WiFi.RSSI());
    } else {
      log("WiFi connect failed: " + WiFiStatus(status));
      return false;
    }
  }

  if (!client.connect(lutronBridge, 23)) {
    log("Connection failed");
    return false;
  }

  // Initial login - seems to always fail
  client.println("foo");
  client.println("bar");
  // Actual login
  client.println("lutron");
  client.println("integration");
  
  // Connection/login takes a while, don't block reading
  log("Connection complete");
  return true;
}

String WiFiStatus(int status) {
  switch(status) {
  case WL_IDLE_STATUS:
    return "WL_IDLE_STATUS";  
  case WL_NO_SSID_AVAIL:
    return "WL_NO_SSID_AVAIL";  
  case WL_CONNECTED:
    return "WL_CONNECTED";  
  case WL_CONNECT_FAILED:
    return "WL_CONNECT_FAILED";  
  case WL_CONNECTION_LOST:
    return "WL_CONNECTION_LOST";  
  case WL_DISCONNECTED:
    return "WL_DISCONNECTED";  
  default:
    return String(status);
  }
}

void lightOff() {
  if (connect()) {
    client.println("#OUTPUT,4,1,0,0:05");
  }
}

void lightOn() {
  if (connect()) {
    client.println("#OUTPUT,4,1,100,0:05");
  }  
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        log("Button pressed");
        lightState = !lightState;
        if (lightState)
          lightOn();
        else
          lightOff();
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}
