#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>
#include <gamma.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>


const char *ssid = "The Promised LAN";
const char *password = "@interwebs!";

String web_page = "";
String userTxt = "X-MAS";
int ledMode = 1;
String strPattern = "";
ESP8266WebServer server(80);

//String web_page = "Christmas Tree";

#define NUMPIXELS 64

#define DATAPIN    9
#define CLOCKPIN   5


Adafruit_DotStarMatrix strip = Adafruit_DotStarMatrix(
  8, 8, DATAPIN, CLOCKPIN,
  DS_MATRIX_TOP     + DS_MATRIX_LEFT +
  DS_MATRIX_COLUMNS + DS_MATRIX_PROGRESSIVE,
  DOTSTAR_BGR);
  
const uint16_t colors[] = {
  strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255) };
  

void params(){
  int lumens = 0;
  Serial.println(server.arg("Pattern"));
  lumens = server.arg("Brightness").toInt();
  userTxt = server.arg("userText");
  ledMode = server.arg("Pattern").toInt();
  Serial.print("Mode:");
  Serial.println(ledMode);
  Serial.println(lumens);
  if (lumens != 0){
    strip.setBrightness(lumens);
    Serial.print("Set Brightness");
  }
  Serial.println(userTxt);
  delay(10);
  
  server.send(200, "text/html", web_page);
}

void handleRoot() {
 
  web_page = "Content-Type: text/html;charset=UTF-8";
  web_page = "<HTML>\n<HEAD>";
  web_page = "</HEAD><BODY>";
  web_page = "<h1>Christmas Control interface V1.1</h1>";
  web_page += " <p>";
  web_page += " <h2>Current Pattern: " + strPattern;
  web_page += " </h2> <p>";
  web_page += " <form action=\"/led\" method=get>";
  web_page += " <select name=\"Pattern\" id=\"Pattern\">";
  web_page += " <option Selected value=\"0\">Patterns</option>";
  web_page += " <option value=\"1\">Rainbow</option>";
  web_page += " <option value=\"2\">Color Wipe</option>";
  web_page += " <option value=\"3\">Theater Chase</option>";
  web_page += " <option value=\"4\">Theater Chase Rainbow</option>";
  web_page += " <option value=\"5\">Text</option>";
  web_page += " <option value=\"6\">Kinda Twinkle</option>";
  web_page += " </select> <br />";
  web_page += " <label for=\"userText\">Text:</label> ";
  web_page += " <input type=\"text\" name=\"userText\" value=\" " + userTxt + "\" maxlength=\"8\" size=\"8\" /><br />";
  web_page += " <label for=\"Brightness\">Brightness (5-255) </label>";
  web_page += "     <input type=\"number\" name=\"Brightness\" min=\"5\" max=\"255\" step=\"25\" />";
  web_page += " <br />";
  web_page += " </p>";
  web_page += "<br><input type=\"submit\" value=\"Send\" /></form>";
  web_page += "</BODY></HTML>";

  server.send(200, "text/html", web_page);
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setTextWrap(false);
  strip.setBrightness(10);
  strip.setTextColor(colors[0]);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/led", params);
  server.begin();
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("");
  Serial.println("The Christmas Tree is Online");
}

int x    = strip.width();
int pass = 0;

void loop() {
  server.handleClient();

if (ledMode == 1){
  rainbowDemo(10);
} else if (ledMode == 2){
  colorWipe(strip.Color(255,   0,   0), 10); // Red
  colorWipe(strip.Color(  0, 255,   0), 10); // Green
  colorWipe(strip.Color(  0,   0, 255), 10); // Blue
  colorWipe(strip.Color(  204,   0, 102), 10); //pink
  colorWipe(strip.Color(  255,   255, 255), 10); //pink
  
} else if (ledMode == 3){
  theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
  theaterChase(strip.Color(127,   0,   0), 50); // Red, half brightness
  theaterChase(strip.Color( 44,   11, 192), 50); // Blue, half brightness
} else if (ledMode == 4){
  theaterChaseRainbow(50);
} else if (ledMode == 5) {
  strip.fillScreen(0);
  strip.setCursor(x, 0);
  strip.print(userTxt);
  if(--x < -36) {
    x = strip.width();
    if(++pass >= 3) pass = 0;
    strip.setTextColor(colors[pass]);
  }
  strip.show();
  delay(70);
} else if (ledMode == 6){
  theaterChase(strip.Color(128,   128,   128), 200); // Whiteish
} else {
  ledMode = 1;
}
CheckPattern();
}

void CheckPattern() {
  
  switch (ledMode) {
    case 1:
      strPattern = "Rainbow";
      break;
    case 2:    // your hand is close to the sensor
      strPattern = "Color Wipe";
      break;
    case 3:    // your hand is a few inches from the sensor
      strPattern = "Theater Chase";
      break;
    case 4:    // your hand is nowhere near the sensor
      strPattern = "Theater Chase Rainbow";
      break;
    case 5:    // your hand is nowhere near the sensor
      strPattern = "Text";
      break;
    case 6:    // your hand is nowhere near the sensor
      strPattern = "Kinda Twinkle";
      break;
  }
}

void rainbowDemo(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}


// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
