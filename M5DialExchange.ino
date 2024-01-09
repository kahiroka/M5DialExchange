#include <M5Dial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char *MQTT_ADDR = "192.168.128.139";
const int MQTT_PORT = 1883;
const char *MQTT_CID = "M5Dial";
const char *MQTT_TOPIC = "kabusapi/exchange";

WiFiClient client;
PubSubClient mqttClient(client);

long g_dial_offset = 0;

M5Canvas canvas(&(M5Dial.Display));

portMUX_TYPE mutex_draw = portMUX_INITIALIZER_UNLOCKED;

void setUpDownColor(int old_val, int new_val) {
  if (old_val > new_val) {
    canvas.setTextColor(CYAN, BLACK);
  } else if (old_val < new_val) {
    canvas.setTextColor(RED, BLACK);
  } else {
    canvas.setTextColor(WHITE, BLACK);
  }
}

void updateExchange(double BidPrice, double AskPrice, double Spread) {
    static int old_bid = 0;
    static int old_ask = 0;
    int bid1000 = int(BidPrice*1000);
    int bid0 = bid1000/1000;
    int bid1 = bid1000%1000/10;
    int bid2 = bid1000%10;
    int ask1000 = int(AskPrice*1000);
    int ask0 = ask1000/1000;
    int ask1 = ask1000%1000/10;
    int ask2 = ask1000%10;

    M5Dial.Display.startWrite();
    M5Dial.Display.setTextSize(0.5);
    M5Dial.Display.setTextColor(LIGHTGREY, BLACK); // WHITE
    M5Dial.Display.drawString("BID  USD/JPY  ASK", M5Dial.Display.width() / 2, M5Dial.Display.height() * 2.5 / 10);
    M5Dial.Display.endWrite();

    setUpDownColor(old_bid, bid1000);
    old_bid = bid1000;

    M5Dial.Display.startWrite();
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextSize(1.5);
    canvas.drawString(String(bid0 + g_dial_offset), 60, 25);
    canvas.pushSprite(0,70);
    M5Dial.Display.endWrite();

    M5Dial.Display.startWrite();
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextSize(1.5);
    canvas.drawString(String(bid1), 48, 25);
    canvas.setTextSize(0.75);
    canvas.drawString(String(bid2), 102, 36);
    canvas.pushSprite(0,120);
    M5Dial.Display.endWrite();

    setUpDownColor(old_ask, ask1000);
    old_ask = ask1000;

    M5Dial.Display.startWrite();
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextSize(1.5);
    canvas.drawString(String(ask0 + g_dial_offset), 60, 25);
    canvas.pushSprite(120,70);
    M5Dial.Display.endWrite();

    M5Dial.Display.startWrite();
    canvas.fillSprite(TFT_BLACK);
    canvas.setTextSize(1.5);
    canvas.drawString(String(ask1), 48, 25);
    canvas.setTextSize(0.75);
    canvas.drawString(String(ask2), 102, 36);
    canvas.pushSprite(120,120);
    M5Dial.Display.endWrite();

    M5Dial.Display.startWrite();
    M5Dial.Display.setTextSize(0.5);
    M5Dial.Display.setTextColor(LIGHTGREY, BLACK); // WHITE
    M5Dial.Display.drawString(" SPREAD " + String(Spread) + " ", M5Dial.Display.width() / 2, M5Dial.Display.height() * 7.5 / 10);
    M5Dial.Display.endWrite();
}

StaticJsonDocument<200> doc;
double g_BidPrice = 0.0;
double g_AskPrice = 0.0;
double g_Spread = 0.0;
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  DeserializationError err = deserializeJson(doc, payload);
  if (!err) {
    double BidPrice = (double)doc["BidPrice"];
    double AskPrice = (double)doc["AskPrice"];
    double Spread = (double)doc["Spread"];
    portENTER_CRITICAL(&mutex_draw);
    g_BidPrice = BidPrice;
    g_AskPrice = AskPrice;
    g_Spread = Spread;
    updateExchange(BidPrice, AskPrice, Spread);
    portEXIT_CRITICAL(&mutex_draw);
  }
}

void printStatus(String message) {
  M5Dial.Display.startWrite();
  M5Dial.Display.clearDisplay(TFT_BLACK);
  M5Dial.Display.setTextColor(WHITE);
  M5Dial.Display.setTextSize(0.5);
  M5Dial.Display.drawString(message, M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
  M5Dial.Display.endWrite();
}

void setupWiFi() {
  if (!M5Dial.BtnA.wasPressed()) {
    WiFi.begin();
  } else { // force setup by pressing the button
    while (!M5Dial.BtnA.wasReleased()) {
      printStatus("Release the button");
      M5Dial.update();
      delay(500);
    }
    //WiFi.eraseAP();
    WiFi.mode(WIFI_STA);
    WiFi.beginSmartConfig();
    while (!WiFi.smartConfigDone()) {
      printStatus("Waiting Esp Config...");
      delay(500);
    }
    printStatus("Esp Configured");
  }
  while(WiFi.status() != WL_CONNECTED){
    M5Dial.update();
    printStatus("Wi-Fi Connecting...");
    delay(500);
  }
  printStatus("Wi-Fi Connected");
}

void connectMqtt(){
  printStatus("MQTT Connecting...");
  while(!mqttClient.connected()){
    if(mqttClient.connect(MQTT_CID)){
      mqttClient.subscribe(MQTT_TOPIC);
      mqttClient.setCallback(mqttCallback);
      printStatus("MQTT Connected");
    }else{
      delay(500);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
  M5Dial.Encoder.write(0);

  canvas.createSprite(120, 50);
  canvas.setTextDatum(middle_center);
  canvas.setTextFont(&fonts::Orbitron_Light_32);

  setupWiFi();
  mqttClient.setServer(MQTT_ADDR, MQTT_PORT);
}

void loop() {
  static long old_dial_offset = 0;
  // put your main code here, to run repeatedly:
  if(!mqttClient.connected()){
    connectMqtt();
    M5Dial.Display.clearDisplay(TFT_BLACK);
  }
  mqttClient.loop();

  M5Dial.update();
  g_dial_offset = M5Dial.Encoder.read();
  if (old_dial_offset != g_dial_offset) {
    portENTER_CRITICAL(&mutex_draw);
    double BidPrice = g_BidPrice;
    double AskPrice = g_AskPrice;
    double Spread = g_Spread;
    updateExchange(BidPrice, AskPrice, Spread);
    portEXIT_CRITICAL(&mutex_draw);
    old_dial_offset = g_dial_offset;
  }
  if (M5Dial.BtnA.wasPressed()) {
    M5Dial.Encoder.write(0);
  }
}
