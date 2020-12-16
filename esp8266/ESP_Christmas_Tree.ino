
#include <ESPAsyncTCP.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WS2812FX.h>
#include <WiFiManager.h>
#include <string.h>
#include "FS.h"
#include <ESP8266mDNS.h>


extern const char index_html[];
extern const char main_js[];
extern const char wifi_html[];
extern const char reset_html[];

// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define LED_PIN3 5
#define LED_COUNT3 24

#define LED_PIN2 4
#define LED_COUNT2 12

#define LED_PIN1 2
#define LED_COUNT1 7

#define HTTP_PORT 80

#define DEFAULT_COLOR 0x0B5394
#define DEFAULT_BRIGHTNESS 180
#define DEFAULT_SPEED 1000
#define DEFAULT_MODE FX_MODE_BREATH

bool spiffsActive = false;

String modes = "";
uint8_t myModes[] = {}; // *** optionally create a custom list of effect/mode numbers
boolean auto_cycle = false;
int auto_last_change;
WS2812FX ws2812fx = WS2812FX(LED_COUNT1, LED_PIN1, NEO_GRB + NEO_KHZ800);
WS2812FX ws2812fx2 = WS2812FX(LED_COUNT2, LED_PIN2, NEO_GRB + NEO_KHZ800);
WS2812FX ws2812fx3 = WS2812FX(LED_COUNT3, LED_PIN3, NEO_GRB + NEO_KHZ800);
WiFiManager wifiManager;
ESP8266WebServer server(HTTP_PORT);

void setup(){
  Serial.begin(115200);
  delay(500);

wifiManager.autoConnect("Tree-NET");
// Start filing subsystem
  if (SPIFFS.begin()) {
      Serial.println("SPIFFS Active");
      Serial.println();
      spiffsActive = true;
      delay(1000);
      
  } else {
      Serial.println("Unable to activate SPIFFS");
  }
  delay(1000);

  Serial.println("\n\nStarting...");

  modes.reserve(5000);
  modes_setup();

  Serial.println("WS2812FX setup");
  ws2812fx.init();
  ws2812fx.setMode(DEFAULT_MODE);
  ws2812fx.setColor(DEFAULT_COLOR);
  ws2812fx.setSpeed(DEFAULT_SPEED);
  ws2812fx.setBrightness(DEFAULT_BRIGHTNESS);
  ws2812fx.start();

  ws2812fx2.init();
  ws2812fx2.setMode(DEFAULT_MODE);
  ws2812fx2.setColor(DEFAULT_COLOR);
  ws2812fx2.setSpeed(DEFAULT_SPEED);
  ws2812fx2.setBrightness(DEFAULT_BRIGHTNESS);
  ws2812fx2.start();

  
  ws2812fx3.init();
  ws2812fx3.setMode(DEFAULT_MODE);
  ws2812fx3.setColor(DEFAULT_COLOR);
  ws2812fx3.setSpeed(DEFAULT_SPEED);
  ws2812fx3.setBrightness(DEFAULT_BRIGHTNESS);
  ws2812fx3.start();
  server.begin();

  delay(1000);
 
  Serial.println("HTTP server setup");
  server.on("/", srv_handle_index_html);
  server.on("/main.js", srv_handle_main_js);
  server.on("/modes", srv_handle_modes);
  server.on("/reset", srv_handle_reset);
  server.on("/set", srv_handle_set);
  server.onNotFound(srv_handle_not_found);
  server.begin();
  Serial.println("HTTP server started.");
  if(MDNS.begin("tree")) {
    Serial.println("MDNS responder started");
  }
  MDNS.addService("http", "tcp", 80);
  Serial.println("ready!");
}


void loop() {

  unsigned long now = millis();
  MDNS.update();
  server.handleClient();
  ws2812fx.service();
  ws2812fx2.service();
  ws2812fx3.service();
  delay(10);
  if(auto_cycle && (now - auto_last_change > 10000)) { // cycle effect mode every 10 seconds
    uint8_t next_mode = (ws2812fx.getMode() + 1) % ws2812fx.getModeCount();
    if(sizeof(myModes) > 0) { // if custom list of modes exists
      for(uint8_t i=0; i < sizeof(myModes); i++) {
        if(myModes[i] == ws2812fx.getMode()) {
          next_mode = ((i + 1) < sizeof(myModes)) ? myModes[i + 1] : myModes[0];
          break;
        }
      }
    }
    ws2812fx.setMode(next_mode);
    ws2812fx2.setMode(next_mode);
    ws2812fx3.setMode(next_mode);
    
    Serial.print("mode is "); Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
    auto_last_change = now;
  }
}


/*
 * Build <li> string for all modes.
 */
void modes_setup() {
  modes = "";
  uint8_t num_modes = sizeof(myModes) > 0 ? sizeof(myModes) : ws2812fx.getModeCount();
  for(uint8_t i=0; i < num_modes; i++) {
    uint8_t m = sizeof(myModes) > 0 ? myModes[i] : i;
    modes += "<li><a href='#'>";
    modes += ws2812fx.getModeName(m);
    modes += "</a></li>";
  }
}

/* #####################################################
#  Webserver Functions
##################################################### */

void srv_handle_not_found() {
  
}

void srv_handle_index_html() {
  server.send_P(200,"text/html", index_html);
}

void srv_handle_main_js() {
  server.send_P(200,"application/javascript", main_js);
}

void srv_handle_modes() {
  server.send(200,"text/plain", modes);
}

void srv_handle_reset() {
  if (server.hasArg("reset")){
    handleWifiReset();
  } else {
  server.send(200,"text/html", reset_html);
}
}

void handleWifiReset(){
 String response="<p>Your network settings have been Cleared.... Restarting. ";
 server.send(200, "text/html", response);
 wifiManager.resetSettings();
 delay(500);
 ESP.restart();

}


void srv_handle_set() {
  for (uint8_t i=0; i < server.args(); i++){
    if(server.argName(i) == "c") {
      uint32_t tmp = (uint32_t) strtol(server.arg(i).c_str(), NULL, 10);
      if(tmp >= 0x000000 && tmp <= 0xFFFFFF) {
        ws2812fx.setColor(tmp);
        ws2812fx2.setColor(tmp);
        ws2812fx3.setColor(tmp);
      }
    }

    if(server.argName(i) == "m") {
      uint8_t tmp = (uint8_t) strtol(server.arg(i).c_str(), NULL, 10);
      ws2812fx.setMode(tmp % ws2812fx.getModeCount());
      ws2812fx2.setMode(tmp % ws2812fx.getModeCount());
      ws2812fx3.setMode(tmp % ws2812fx.getModeCount());
      Serial.print("mode is "); Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
    }

    if(server.argName(i) == "b") {
      if(server.arg(i)[0] == '-') {
        ws2812fx.setBrightness(ws2812fx.getBrightness() * 0.8);
        ws2812fx2.setBrightness(ws2812fx.getBrightness() * 0.8);
        ws2812fx3.setBrightness(ws2812fx.getBrightness() * 0.8);
      } else if(server.arg(i)[0] == ' ') {
        ws2812fx.setBrightness(min(max(ws2812fx.getBrightness(), 5) * 1.2, 255));
        ws2812fx2.setBrightness(min(max(ws2812fx.getBrightness(), 5) * 1.2, 255));
        ws2812fx3.setBrightness(min(max(ws2812fx.getBrightness(), 5) * 1.2, 255));
      } else { // set brightness directly
        uint8_t tmp = (uint8_t) strtol(server.arg(i).c_str(), NULL, 10);
        ws2812fx.setBrightness(tmp);
        ws2812fx2.setBrightness(tmp);
        ws2812fx3.setBrightness(tmp);
      }
      Serial.print("brightness is "); Serial.println(ws2812fx.getBrightness());
    }

    if(server.argName(i) == "s") {
      if(server.arg(i)[0] == '-') {
        ws2812fx.setSpeed(max(ws2812fx.getSpeed(), 5) * 1.2);
        ws2812fx2.setSpeed(max(ws2812fx.getSpeed(), 5) * 1.2);
        ws2812fx3.setSpeed(max(ws2812fx.getSpeed(), 5) * 1.2);
      } else if(server.arg(i)[0] == ' ') {
        ws2812fx.setSpeed(ws2812fx.getSpeed() * 0.8);
        ws2812fx2.setSpeed(ws2812fx.getSpeed() * 0.8);
        ws2812fx3.setSpeed(ws2812fx.getSpeed() * 0.8);
      } else {
        uint16_t tmp = (uint16_t) strtol(server.arg(i).c_str(), NULL, 10);
        ws2812fx.setSpeed(tmp);
        ws2812fx2.setSpeed(tmp);
        ws2812fx3.setSpeed(tmp);
      }
      Serial.print("speed is "); Serial.println(ws2812fx.getSpeed());
    }

    if(server.argName(i) == "a") {
      if(server.arg(i)[0] == '-') {
        auto_cycle = false;
      } else {
        auto_cycle = true;
        auto_last_change = 0;
      }
    }
  }
  server.send(200, "text/plain", "OK");
}
