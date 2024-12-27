/*
 * Apple MIDI code from https://github.com/lathoub/Arduino-AppleMIDI-Library/blob/master/examples/ESP8266_NoteOnOffEverySec_softAP_mDNS/ESP8266_NoteOnOffEverySec_softAP_mDNS.ino
 * WIFI code from https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/RangeExtender-NAPT/RangeExtender-NAPT.ino
*/

#ifndef STASSID
#define STASSID "<your SID>"
#define STAPSK "<your WIFI password>"
#endif

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <lwip/napt.h>
#include <lwip/dns.h>

#define SerialMon Serial
#include <AppleMIDI.h>
#include <AppleMIDI_Debug.h>

#include <Adafruit_MCP4728.h>
#include <Wire.h>

#define NAPT 1000
#define NAPT_PORT 10

#define D3 0
#define D4 2
#define D5 14
#define D6 12

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();
Adafruit_MCP4728 mcp1, mcp2;
int8_t isConnected = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 Apple-MIDI MCP4728 MIDI-to-CV");
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW); // this is also the internal LED - "LOW" means LED on
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);

  // connect MCP4728 chip #1
  if (!mcp1.begin(0x60)) {
    Serial.println("Failed to find MCP4728 chip #1");
    while (1) { // if this fails, it does not make sense to continue
      delay(1000);
    }
  } else {
    Serial.println("Found MCP4728 chip #1");
  }

  // connect MCP4728 chip #1
  if (!mcp2.begin(0x61)) {
    Serial.println("Failed to find MCP4728 chip #2");
    while (1) { // if this fails, it does not make sense to continue
      delay(1000);
    }
  } else {
    Serial.println("Found MCP4728 chip #2");
  }


  // Connect to STA so we can get a proper local DNS server
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.printf("\nSTA: %s (dns: %s / %s)\n", WiFi.localIP().toString().c_str(), WiFi.dnsIP(0).toString().c_str(), WiFi.dnsIP(1).toString().c_str());

  // By default, DNS option will point to the interface IP
  // Instead, point it to the real DNS server.
  // Notice that:
  // - DhcpServer class only supports IPv4
  // - Only a single IP can be set
  auto &server = WiFi.softAPDhcpServer();
  server.setDns(WiFi.dnsIP(0));

  WiFi.softAP(STASSID "extender", STAPSK);
  Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());

  err_t ret = ip_napt_init(NAPT, NAPT_PORT);
  Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) {
      Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n", STASSID "extender", STASSID);
    }
  }

  if (ret != ERR_OK) {
    Serial.printf("NAPT initialization failed\n");
  }

  // Set up mDNS responder
  if (!MDNS.begin(AppleMIDI.getName())) {
    Serial.printf("Error setting up MDNS responder!");
  }

  char str[128] = "";
  strcat(str, AppleMIDI.getName());
  strcat(str, ".local");
  DBG(F("mDNS responder started at:"), str);
  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());

  DBG(F(""));
  DBG(F("Start MIDI Network app on iPhone/iPad or rtpMIDI on Windows"));
  DBG(F("AppleMIDI-ESP8266 will show in the 'Directory' list (rtpMIDI) or"));
  DBG(F("under 'Found on the network' list (iOS). Select and click 'Connect'"));
  DBG(F(""));

  // only listen for events on MIDI channel 1
  MIDI.begin(1);

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *name) {
    isConnected++;
    DBG(F("Connected to session"), ssrc, name);
  });

  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc) {
    isConnected--;
    DBG(F("Disconnected"), ssrc);
  });

  MIDI.setHandleControlChange([](Channel channel, byte cc, byte value) {
    //DBG(F("ControlChange"), "MIDI channel", channel, "CC", cc, "value", value);
    uint16_t channelValue = map(value, 0, 127, 0, 4095);
    switch (cc) {
      case 0:
        mcp1.setChannelValue(MCP4728_CHANNEL_A, channelValue);
        break;
      case 1:
        mcp1.setChannelValue(MCP4728_CHANNEL_B, channelValue);
        break;
      case 2:
        mcp1.setChannelValue(MCP4728_CHANNEL_C, channelValue);
        break;
      case 3:
        mcp1.setChannelValue(MCP4728_CHANNEL_D, channelValue);
        break;
      case 4:
        mcp2.setChannelValue(MCP4728_CHANNEL_A, channelValue);
        break;
      case 5:
        mcp2.setChannelValue(MCP4728_CHANNEL_B, channelValue);
        break;
      case 6:
        mcp2.setChannelValue(MCP4728_CHANNEL_C, channelValue);
        break;
      case 7:
        mcp2.setChannelValue(MCP4728_CHANNEL_D, channelValue);
        break;
      default:
        // do nothing
        break;
    }
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    //DBG(F("NoteOn"), note, "channel", channel, "velocity", velocity);
    switch (note) {
      case 48:
        digitalWrite(D3, HIGH);
        break;
      case 49:
        digitalWrite(D4, HIGH); // this is also the internal LED - "HIGH" means LED off
        break;
      case 50:
        digitalWrite(D5, HIGH);
        break;
      case 51:
        digitalWrite(D6, HIGH);
        break;
      default:
        // do nothing
        break;
    }
  });

  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    //DBG(F("NoteOff"), note, "channel", channel, "velocity", velocity);
    switch (note) {
      case 48:
        digitalWrite(D3, LOW);
        break;
      case 49:
        digitalWrite(D4, LOW); // this is also the internal LED - "LOW" means LED on
        break;
      case 50:
        digitalWrite(D5, LOW);
        break;
      case 51:
        digitalWrite(D6, LOW);
        break;
      default:
        // do nothing
        break;
    }
  });
}

void loop() {
  // According to the documentation 'update' should be called in every 'loop'
  MDNS.update();

  // Listen to incoming notes
  MIDI.read();
}
