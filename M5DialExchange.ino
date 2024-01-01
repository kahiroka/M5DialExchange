#include <M5Dial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char *MQTT_ADDR = "192.168.100.5";
const int MQTT_PORT = 1883;
const char *MQTT_CID = "M5Dial";
const char *MQTT_TOPIC = "kabusapi/exchange";

WiFiClient client;
PubSubClient mqttClient(client);

long g_dial_offset = 0;

void setUpDownColor(int old_val, int new_val) {
  if (old_val > new_val) {
    M5Dial.Display.setTextColor(CYAN);
  } else if (old_val < new_val) {
    M5Dial.Display.setTextColor(RED);
  } else {
    M5Dial.Display.setTextColor(WHITE);
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

    M5Dial.Display.clearDisplay(TFT_BLACK);

    M5Dial.Display.setTextSize(0.5);
    M5Dial.Display.setTextColor(WHITE);
    M5Dial.Display.drawString("BID  USD/JPY  ASK", M5Dial.Display.width() / 2, M5Dial.Display.height() * 2.5 / 10);

    setUpDownColor(old_bid, bid1000);
    old_bid = bid1000;
    M5Dial.Display.setTextSize(1.5);
    M5Dial.Display.drawString(String(bid0 + g_dial_offset), M5Dial.Display.width() / 4, M5Dial.Display.height() * 4 / 10);
    M5Dial.Display.drawString(String(bid1), M5Dial.Display.width() * 5 / 24, M5Dial.Display.height() * 6 / 10);
    M5Dial.Display.setTextSize(0.75);
    M5Dial.Display.drawString(String(bid2), M5Dial.Display.width() * 10.5 / 24, M5Dial.Display.height() * 6.5 / 10);

    setUpDownColor(old_ask, ask1000);
    old_ask = ask1000;
    M5Dial.Display.setTextSize(1.5);
    M5Dial.Display.drawString(String(ask0 + g_dial_offset), M5Dial.Display.width() * 3 / 4, M5Dial.Display.height() * 4 / 10);
    M5Dial.Display.drawString(String(ask1), M5Dial.Display.width() * 17 / 24, M5Dial.Display.height() * 6 / 10);
    M5Dial.Display.setTextSize(0.75);
    M5Dial.Display.drawString(String(ask2), M5Dial.Display.width() * 22.5 / 24, M5Dial.Display.height() * 6.5 / 10);

    M5Dial.Display.setTextSize(0.5);
    M5Dial.Display.setTextColor(WHITE);
    M5Dial.Display.drawString("SPREAD " + String(Spread), M5Dial.Display.width() / 2, M5Dial.Display.height() * 7.5 / 10);
}

StaticJsonDocument<200> doc;
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  DeserializationError err = deserializeJson(doc, payload);
  if (!err) {
    double BidPrice = (double)doc["BidPrice"];
    double AskPrice = (double)doc["AskPrice"];
    double Spread = (double)doc["Spread"];
    updateExchange(BidPrice, AskPrice, Spread);
  }
}

void printStatus(String message) {
  M5Dial.Display.clearDisplay(TFT_BLACK);
  M5Dial.Display.setTextColor(WHITE);
  M5Dial.Display.setTextSize(0.5);
  M5Dial.Display.drawString(message, M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
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

  setupWiFi();
  mqttClient.setServer(MQTT_ADDR, MQTT_PORT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!mqttClient.connected()){
    connectMqtt();
  }
  mqttClient.loop();

  M5Dial.update();
  g_dial_offset = M5Dial.Encoder.read();
  if (M5Dial.BtnA.wasPressed()) {
    M5Dial.Encoder.write(0);
  }
}
