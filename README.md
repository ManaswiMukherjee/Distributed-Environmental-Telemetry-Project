# Distributed-Environmental-Telemetry-Project

A Distributed Environmental Telemetry System v0.1.0, which collects temperature and pressure data and logs them to a webserver on a local network.

---
# v0.1.0 :



Features in v0.1.0:

*️⃣ Alpine-linux based webserver for significantly lower resource usage.

*️⃣ Docker running containerized Mosquitto broker and time-series database InfluxDB paired with Telegraf.

*️⃣ MQTT communication between node and server to perform better in low bandwidth environments.

*️⃣ First node (Raspberry Pi Pico W with a bmp280 sensor) programmed in C/C++ i.e. Arduino IDE instead of MicroPython for better performance.
