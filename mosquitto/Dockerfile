# 取出 telegraf 執行檔
FROM telegraf:latest AS telegraf

# 取出 mosquitto 執行檔
FROM eclipse-mosquitto:latest AS mosquitto

# 主鏡像
FROM ubuntu:22.04

# 必要 library
RUN apt-get update && \
    apt-get install -y libssl-dev && \
    rm -rf /var/lib/apt/lists/*

# 複製 telegraf 必要檔案
COPY --from=telegraf /usr/bin/telegraf /usr/bin/telegraf
COPY --from=telegraf /etc/telegraf/ /etc/telegraf/
COPY --from=telegraf /var/lib/telegraf/ /var/lib/telegraf/
COPY --from=telegraf /var/log/telegraf/ /var/log/telegraf/

# 複製 mosquitto 必要檔案
COPY --from=mosquitto /usr/sbin/mosquitto /usr/sbin/mosquitto
COPY --from=mosquitto /mosquitto/ /mosquitto/

# 建立目錄
RUN mkdir -p /mosquitto/data /mosquitto/log

# 預設 config 用 volume 掛載
VOLUME ["/etc/telegraf", "/mosquitto/config", "/mosquitto/data", "/mosquitto/log"]

# 開放 MQTT port
EXPOSE 1883

# 一起啟動 telegraf & mosquitto
CMD mosquitto -c /mosquitto/config/mosquitto.conf & \
    telegraf --config /etc/telegraf/telegraf.conf
