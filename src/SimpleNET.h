#ifndef SIMPLE_NET_H
#define SIMPLE_NET_H


#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <stdint.h>
#include <Arduino.h>
#include <vector>


#include "OdometerData.h"

//Debugging definitions
//#define DEBUG 1
//#define DEBUGWIFI 1
//#define DEBUGCSV 1
//#define DEBUGSENT 1
#define SERIAL_ENABLE 1
//#define DEBUGTIME 1
//#define DEBUGTIMESTAMP 1

//#define LIGHTWEIGHT 1




//other definitions
#define LABOR 1


//#define ENABLE_MQTT 1


#define UDP_PORT 4210


#ifdef DAHEIM
#define SSID "WLAN"
#define PASSWORD "06130705342415756002"
#define SERVER_IP "192.168.178.42"
#endif

#ifdef HOFSPANNUNG
#define SSID "HofSpannung"
#define PASSWORD "8ZfjM4GHYVgrYa"
#define SERVER_IP "192.168.1.62"
#endif

#ifdef LABOR
#define SSID "Labore-Hof AT-MT"
#define PASSWORD "laborwlan"
#define SERVER_IP "192.168.9.150"
#endif

#ifdef HOF
#define SSID "karmvc"
#define PASSWORD "jvTtFfN3wC8g4UcQ"
#define SERVER_IP "192.168.178.112"
#endif

#ifdef JOEL
#define SSID "iPhone von Joel"
#define PASSWORD "12345678"
#define SERVER_IP "172.20.10.7"
#endif



void SUDP_beginn(uint64_t u64_time);//give the time in microseconds to the function so it can sync the time to the system
void SUDP_send(odometerData_t data);
void WIFIstart();
uint64_t bytesToUint64_StringDigits(const std::vector<uint8_t>& bytes); //convert a string to a uint64_t
uint32_t accurateMillis(); //get the accurate millis from the timer
#endif