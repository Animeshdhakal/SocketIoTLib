#ifndef SocketIoTIdentifyDevice_h
#define SocketIoTIdentifyDevice_h


#if defined(__linux__)
    #define HARDWARE_NAME "Linux"
#elif defined(ARDUINO)
    #if defined(ARDUINO_AVR_NANO)
        #define HARDWARE_NAME  "Arduino Nano"
    #elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_DUEMILANOVE)
        #define HARDWARE_NAME  "Arduino Uno"
    #elif defined(ARDUINO_AVR_YUN)
        #define HARDWARE_NAME  "Arduino Yun"
    #elif defined(ARDUINO_AVR_MINI)
        #define HARDWARE_NAME  "Arduino Mini"
    #elif defined(ARDUINO_AVR_ETHERNET)
        #define HARDWARE_NAME  "Arduino Ethernet"
    #elif defined(ARDUINO_AVR_FIO)
        #define HARDWARE_NAME  "Arduino Fio"
    #elif defined(ARDUINO_AVR_BT)
        #define HARDWARE_NAME  "Arduino BT"
    #elif defined(ARDUINO_AVR_PRO)
        #define HARDWARE_NAME  "Arduino Pro"
    #elif defined(ARDUINO_AVR_NG)
        #define HARDWARE_NAME  "Arduino NG"
    #elif defined(ARDUINO_AVR_GEMMA)
        #define HARDWARE_NAME  "Arduino Gemma"
    #elif defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
        #define HARDWARE_NAME  "Arduino Mega"
    #elif defined(ARDUINO_AVR_ADK)
        #define HARDWARE_NAME  "Arduino Mega ADK"
    #elif defined(ARDUINO_AVR_LEONARDO)
        #define HARDWARE_NAME  "Arduino Leonardo"
    #elif defined(ARDUINO_AVR_MICRO)
        #define HARDWARE_NAME  "Arduino Micro"
    #elif defined(ARDUINO_AVR_ESPLORA)
        #define HARDWARE_NAME  "Arduino Esplora"
    #elif defined(ARDUINO_AVR_LILYPAD)
        #define HARDWARE_NAME  "Lilypad"
    #elif defined(ARDUINO_AVR_LILYPAD_USB)
        #define HARDWARE_NAME  "Lilypad USB"
    #elif defined(ARDUINO_AVR_ROBOT_MOTOR)
        #define HARDWARE_NAME  "Robot Motor"
    #elif defined(ARDUINO_AVR_ROBOT_CONTROL)
        #define HARDWARE_NAME  "Robot Control"

    /* Arduino megaAVR */
    #elif defined(ARDUINO_AVR_UNO_WIFI_REV2)
        #define HARDWARE_NAME  "Arduino UNO WiFi Rev2"

    /* Arduino SAM */
    #elif defined(ARDUINO_SAM_DUE)
        #define HARDWARE_NAME  "Arduino Due"

    /* Arduino SAMD */
    #elif defined(ARDUINO_SAMD_ZERO)
        #define HARDWARE_NAME  "Arduino Zero"
    #elif defined(ARDUINO_SAMD_MKR1000)
        #define HARDWARE_NAME  "MKR1000"
    #elif defined(ARDUINO_SAMD_MKRZERO)
        #define HARDWARE_NAME  "MKRZERO"
    #elif defined(ARDUINO_SAMD_MKRNB1500)
        #define HARDWARE_NAME  "MKR NB 1500"
    #elif defined(ARDUINO_SAMD_MKRGSM1400)
        #define HARDWARE_NAME  "MKR GSM 1400"
    #elif defined(ARDUINO_SAMD_MKRWAN1300)
        #define HARDWARE_NAME  "MKR WAN 1300"
    #elif defined(ARDUINO_SAMD_MKRFox1200)
        #define HARDWARE_NAME  "MKR FOX 1200"
    #elif defined(ARDUINO_SAMD_MKRWIFI1010)
        #define HARDWARE_NAME  "MKR WiFi 1010"
    #elif defined(ARDUINO_SAMD_MKRVIDOR4000)
        #define HARDWARE_NAME  "MKR Vidor 4000"

    /* Intel */
    #elif defined(ARDUINO_GALILEO)
        #define HARDWARE_NAME  "Galileo"
    #elif defined(ARDUINO_GALILEOGEN2)
        #define HARDWARE_NAME  "Galileo Gen2"
    #elif defined(ARDUINO_EDISON)
        #define HARDWARE_NAME  "Edison"
    #elif defined(ARDUINO_ARCH_ARC32)
        #define HARDWARE_NAME  "Arduino 101"

    /* Konekt */
    #elif defined(ARDUINO_DASH)
        #define HARDWARE_NAME  "Dash"
    #elif defined(ARDUINO_DASHPRO)
        #define HARDWARE_NAME  "Dash Pro"

    /* Red Bear Lab */
    #elif defined(ARDUINO_RedBear_Duo)
        #define HARDWARE_NAME  "RedBear Duo"
    #elif defined(ARDUINO_BLEND)
        #define HARDWARE_NAME  "Blend"
    #elif defined(ARDUINO_BLEND_MICRO_8MHZ) || defined(ARDUINO_BLEND_MICRO_16MHZ)
        #define HARDWARE_NAME  "Blend Micro"
    #elif defined(ARDUINO_RBL_nRF51822)
        #define HARDWARE_NAME  "BLE Nano"

    /* ESP8266 */
    #elif defined(ARDUINO_ESP8266_NODEMCU)
        #define HARDWARE_NAME  "NodeMCU"
    #elif defined(ARDUINO_ARCH_ESP8266)
        #define HARDWARE_NAME  "ESP8266"

    /* ESP32 */
    #elif defined(ARDUINO_ESP32C3_DEV)
        #define HARDWARE_NAME  "ESP32C3"
    #elif defined(ARDUINO_ESP32S2_DEV)
        #define HARDWARE_NAME  "ESP32S2"
    #elif defined(ARDUINO_ARCH_ESP32)
        #define HARDWARE_NAME  "ESP32"

    /* STM32 */
    #elif defined(ARDUINO_ARCH_STM32F1)
        #define HARDWARE_NAME  "STM32F1"
    #elif defined(ARDUINO_ARCH_STM32F3)
        #define HARDWARE_NAME  "STM32F3"
    #elif defined(ARDUINO_ARCH_STM32F4)
        #define HARDWARE_NAME  "STM32F4"

    /* Digistump */
    #elif defined(ARDUINO_ESP8266_OAK)
        #define HARDWARE_NAME  "Oak"
    #elif defined(ARDUINO_AVR_DIGISPARK)
        #define HARDWARE_NAME  "Digispark"
    #elif defined(ARDUINO_AVR_DIGISPARKPRO)
        #define HARDWARE_NAME  "Digispark Pro"

    /* Microduino */
    #elif defined(ARDUINO_AVR_USB)
        #define HARDWARE_NAME  "CoreUSB"
    #elif defined(ARDUINO_AVR_PLUS)
        #define HARDWARE_NAME  "Core+"
    #elif defined(ARDUINO_AVR_RF)
        #define HARDWARE_NAME  "CoreRF"

    /* Wildfire */
    #elif defined(ARDUINO_WILDFIRE_V2)
        #define HARDWARE_NAME  "Wildfire V2"
    #elif defined(ARDUINO_WILDFIRE_V3)
        #define HARDWARE_NAME  "Wildfire V3"
    #elif defined(ARDUINO_WILDFIRE_V4)
        #define HARDWARE_NAME  "Wildfire V4"

     /* Seeed studio */
    #elif defined(SEEED_WIO_TERMINAL)
        #define HARDWARE_NAME "Seeed Wio Terminal"

    /* Simblee */
    #elif defined(__Simblee__)
        #define HARDWARE_NAME  "Simblee"

    /* RFduino */
    #elif defined(__RFduino__)
        #define HARDWARE_NAME  "RFduino"

    /* Nordic NRF5x */
    #elif defined(ARDUINO_ARCH_NRF5)
        #define HARDWARE_NAME  "nRF5"

    #else
        #define HARDWARE_NAME  "Arduino"
    #endif

#elif defined(TI_CC3220)
    #define HARDWARE_NAME  "TI CC3220"
#else
    #define HARDWARE_NAME  "Custom platform"
    
#endif


#endif
