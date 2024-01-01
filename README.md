# M5DialExchange
M5Dial Exchange Display

# Deps
M5Dial(M5GFX,M5Unified)
PubSubClient
ArduionoJson

# Usage
1. Setup M5Dial Wi-Fi by using Expressif Esptouch (iOS)
2. Setup MQTT server (e.g., Mosquitto)
3. Publish exchange json data to topic:kabusapi/exchange

  $ python3 ./file2mqtt.py --topic exchange -f ./logs/exchange-20231228.log --loop
