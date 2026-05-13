# [0.1.0] - 2026-05-02 Initial
## Features in v0.1.0:

*️⃣ Headless Alpine-linux based webserver for significantly lower resource usage.

*️⃣ Docker running containerized Mosquitto broker and time-series database InfluxDB paired with Telegraf.(500MB total RAM usage)

*️⃣ MQTT communication between node and server to perform better in low bandwidth environments.

*️⃣ First node (Raspberry Pi Pico W with a bmp280 sensor) programmed in C/C++ i.e. Arduino IDE instead of MicroPython for better performance.

---
---
# [0.2.0] - 2026-05-13

## Added: 

*️⃣ Second node: NodeMCU ESP8266 v3 with bmp280 sensor and Neo-6M GPS module. This node runs on a single 18650 li-ion cell. The voltage of the cell has been increased to 5V by using an MT3608 dc-dc step up converter.

It features a very-fast wifi connection 137ms as seen in this [blog](
https://blog.voneicken.com/2018/lp-wifi-esp8266-1/)