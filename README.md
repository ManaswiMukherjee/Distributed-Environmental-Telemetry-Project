# Distributed-Environmental-Telemetry-Project

A Distributed Environmental Telemetry System, which collects temperature and pressure data and logs them to a webserver on a local network. There is an old pc, acting as broker, runs mosquitto mqtt and influxdb docker containers on it. The project features two microcontrollers, one is the RPi pico W with a bmp280 and an OLED screen, which will be kept indoors and powered from a wall adapter. second is the esp8266 which will be powered by a 18650 li-ion battery and will be kept outside, it will have a bmp280 and a Neo-6M GPS module. 

## What's New
Check out the [CHANGELOG](./CHANGELOG.md) for a full list of recent features, bug fixes, and breaking changes.

For detailed documentation see the [docs](./docs/) folder.

[References and helpful resources](REFERENCES.md)
