#include "config.hpp"

#ifdef BASELINE_WANT_SERIAL
#define MSG(x) Serial.println(x)
#else
#define MSG(x)                                                                                     \
  do {                                                                                             \
  } while (false)
#endif

#if defined(BASELINE_WANT_WIFI_AP) || defined(BASELINE_WANT_WIFI_STA)
#define BASELINE_WANT_WIFI
#include <WiFi.h>
#endif

#ifdef BASELINE_WANT_BLE_DEVICE
#include <NimBLEDevice.h>
#endif

void
setup() {
#ifdef BASELINE_WANT_SERIAL
  Serial.begin(115200);
  Serial.println();
#endif

#ifdef BASELINE_WANT_WIFI
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
#endif

#ifdef BASELINE_WANT_BLE_DEVICE
  NimBLEDevice::init("esp32");
  NimBLEDevice::setMTU(517);
  NimBLEDevice::startAdvertising();
  MSG("BLE enabled");
  delay(2000);
  NimBLEDevice::stopAdvertising();
  NimBLEDevice::deinit(true);
  MSG("BLE disabled");
#endif

#ifdef BASELINE_WANT_WIFI_AP
  WiFi.softAP("ssid", "password", 1, 0, 1);
  MSG("WiFi AP enabled");
  while (WiFi.softAPgetStationNum() == 0) {
    delay(100);
  }
  WiFi.softAPdisconnect(true);
  MSG("WiFi AP disabled");
  delay(4000);
#endif

#ifdef BASELINE_WANT_WIFI_STA
  WiFi.setSleep(false);
  WiFi.begin("uplink", "password");
  WiFi.waitForConnectResult();
  MSG(WiFi.isConnected() ? "WiFi STA connected" : "WiFi STA not connected");
#endif

  MSG("OK");
}

void
loop() {}
