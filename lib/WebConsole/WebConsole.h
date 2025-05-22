#pragma once
#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiServer.h>


namespace WebConsole {
    void begin(uint8_t baud);
    void print(const char* str);
    void printf(const char* format, ...);
    void println(const char* str);
}

