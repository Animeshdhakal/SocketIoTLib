CURRENT_DIR = $(shell pwd)
ESP_ROOT = $(CURRENT_DIR)/build/esp8266
SKETCH = $(CURRENT_DIR)/src/main.cpp
LIBS = $(CURRENT_DIR)/SocketIoT
MONITOR_SPEED = 115200
BOARD = nodemcuv2

include $(CURRENT_DIR)/build/makeEspArduino/makeEspArduino.mk
