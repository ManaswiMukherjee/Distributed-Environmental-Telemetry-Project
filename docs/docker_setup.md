# Setup Guide

This document explains how setup the alpine webserver.

---

# Prerequisites


- preconfigured alpine server
(for help visit [wiki.alpine.org](https://wiki.alpinelinux.org))


---

# Project Structure

```text
project/
├── mosquitto/
|   ├── config/
|   ├── data/
|   ├── log/
├── secrets
|   ├── influxdb2-admin-username
|   ├── influxdb2-admin-password
|   ├── influxdb2-admin-token
├── .env
└── docker-compose.yaml
```

---
# 1. Install docker and docker-compose

```bash
doas apk add docker docker-compose-cli
```

# 2. Configure the Influxdb server
- Start services
``` docker-compose up```
- Open port 8086 on the server ip to access web ui.
- Create InfluxDB credentials
- Store the token and credentials in the  ```secret``` folders.

# 3. Configure Telegraf in InfluxDb web UI
- Go to the telegraf in the left hand side portion to create a new configuration(arrow logo)
- Follow through the steps and ensure ```["tcp://127.0.0.1:1883"]``` is changed to ```["tcp://192.168.1.31:1883"]``` in the input mqtt plugin part.
- Edit the MQTT username and password.
- Store Telegraf token in ```.env``` file and edit the below portion in docker-compose.yaml
```      
    http://192.168.1.31:8086/api/v2/telegrafs/[string provided during telegraf configuration]
```

# 3. Restart docker-compose

```docker-compose up```