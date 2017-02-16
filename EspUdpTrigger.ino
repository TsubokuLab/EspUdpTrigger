#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <EEPROM.h>
#define MAX_TRIGGER 8
#define EEPROM_SIZE 512

const char* device_name = "device";
const char* path_root   = "/index.html";
// client mode
String ssid        = "ssid";
String pass        = "pass";
// server mode
String server_ssid    = "esp";
String server_pass    = "12345678";
#define BUFFER_SIZE 10000
uint8_t html[BUFFER_SIZE];
  
ESP8266WebServer server(80);
bool isServerMode;
WiFiUDP UDP;

const String css = "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'><link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'><script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js' integrity='sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa' crossorigin='anonymous'></script><script src='https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js'></script>";

struct TRIG{
  char pin[5];
  char ip[32];
  char port[8];
};
struct CONFIG {
  char deviceName[16];
  char ssid[32];
  char pass[32];
  struct TRIG triggers[MAX_TRIGGER];
};
CONFIG buf;
int _triggers_count;
struct FLAG{
  bool _12 = false;
  bool _13 = false;
  bool _14 = false;
  bool _15 = false;
  bool _16 = false;
};
FLAG pinState;

void reboot(){
  Serial.println("[ Reboot ]");
  ESP.restart();
}

void callTrigger(int _pinNum, bool _flag){
  Serial.println("[ Call Trigger ] pin : " + String(_pinNum) + " , flag : " + String(_flag));
  String message = String(device_name) + "/" + String(_pinNum) + "/" + String(_flag) + ";";
  char charArray[48];
  message.toCharArray(charArray, 48);

  uint8_t _count = 0;
  while(_count < _triggers_count){
    if(String(buf.triggers[_count].pin) == String(_pinNum)){ 
      UDP.beginPacket(buf.triggers[_count].ip, String(buf.triggers[_count].port).toInt());
      UDP.write(charArray);
      UDP.endPacket();
      Serial.println("[ UDP Send ] " + String(buf.triggers[_count].ip) + ":" + String(buf.triggers[_count].port) + " > " + message );
    }
    _count++;
  }
}

void setWiFiConfig(){
  Serial.println("set WiFi Config.");
  String _ssid = server.arg("ssid");
  String _pass = server.arg("pass");
  String _deviceName = server.arg("name");
  
  CONFIG buf;
  EEPROM.get<CONFIG>(0, buf);  
  if(_ssid != "")strcpy(buf.ssid, _ssid.c_str());
  if(_pass != "")strcpy(buf.pass, _pass.c_str());
  if(_deviceName != "")strcpy(buf.deviceName, _deviceName.c_str());

  uint8_t _arg_num = server.args();
  for(uint8_t i = 0; i < _arg_num; i++){
    Serial.println(String(i, DEC) + " : " + server.argName(i) + " = " + server.arg(i));
    delay(1);
  }

  // remove all triggers
  for(uint8_t i = 0; i < MAX_TRIGGER; i++){
    String _null = "";
    strcpy(buf.triggers[i].pin, _null.c_str());
    strcpy(buf.triggers[i].ip, _null.c_str());
    strcpy(buf.triggers[i].port, _null.c_str());
    delay(1);
  }
  Serial.println("remove all triggers");
  
  uint8_t _count = 0;
  String _ArgName = "triggers_" + String(_count, DEC) + "_pin";
  while(server.hasArg(_ArgName.c_str()) == true && _count < MAX_TRIGGER){
    String _pin = server.arg("triggers_" + String(_count, DEC) + "_pin");
    String _ip = server.arg("triggers_" + String(_count, DEC) + "_ip");
    String _port = server.arg("triggers_" + String(_count, DEC) + "_port");
    strcpy(buf.triggers[_count].pin, _pin.c_str());
    strcpy(buf.triggers[_count].ip, _ip.c_str());
    strcpy(buf.triggers[_count].port, _port.c_str());
    _count++;
    _ArgName = "triggers_" + String(_count, DEC) + "_pin";
    delay(1);
  }
  EEPROM.put<CONFIG>(0, buf);
  EEPROM.commit();

  String text = "<h1>SET</h1>";
  text += "<p>Device Name: ";
  text += buf.deviceName;
  text += "</p>";
  text += "<p>SSID: ";
  text += buf.ssid;
  text += "</p>";
  text += "<p>PASS: ";
  text += buf.pass;
  text += "</p>";
  text += "<p>Triggers</p>\n";
  text += "<ul>\n";
  for(uint8_t i = 0; i < MAX_TRIGGER; i++){
    text += "<li>\n";
    text += "Trigger_" + String(i, DEC) + "<br />\n";
    text += "pin : ";
    text += buf.triggers[i].pin;
    text += "<br />ip : ";
    text += buf.triggers[i].ip;
    text += "<br />port : ";
    text += buf.triggers[i].port;
    text += "</li>\n";
    delay(1);
  }
  text += "</ul>\n";
  text += "<a href='/'>Back</a>\n";
  server.send(200, "text/html", text);

  reboot();
}

bool setup_client() {
  Serial.println("[ Setup Client Mode ]");
  CONFIG buf;
  EEPROM.get<CONFIG>(0, buf);
  Serial.print("SSID : ");
  Serial.println(buf.ssid);
  Serial.print("PASS : ");
  Serial.println(buf.pass);

  WiFi.begin(buf.ssid, buf.pass);
  WiFi.mode(WIFI_STA);
  Serial.println("Connecting to Access Point");
  
  uint8_t _count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(_count > 20){
      Serial.println();
      return false;
    }else{
      _count++;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP()); 
  isServerMode = false;
  return true;
}

void setup_server() {
  Serial.println("[ Setup Server Mode ]");
  WiFi.mode(WIFI_AP);
  Serial.println("SSID: " + server_ssid);
  Serial.println("PASS: " + server_pass);

  WiFi.softAP(server_ssid.c_str(), server_pass.c_str());
  Serial.println("HTTP server started.");
  Serial.println("http://192.168.4.1/");
  isServerMode = true;
}

boolean showIndex(){
  Serial.println("show index.html");
  File htmlFile = SPIFFS.open(path_root, "r");
  if (!htmlFile) {
    Serial.println("Failed to open index.html");
    return false;
  }
  size_t size = htmlFile.size();
  if(size >= BUFFER_SIZE){
    Serial.print("File Size Error:");
    Serial.println((int)size);
  }else{
    Serial.print("File Size OK:");
    Serial.println((int)size);
  }

  String _html;
  htmlFile.read(html,size);
  htmlFile.close();

  CONFIG buf;
  EEPROM.get<CONFIG>(0, buf);  
  if(sizeof(buf.deviceName) > 0){
    _html = (char*)html;
    _html.replace("#VERSION#", "ESP UDP Manager v0.1.3");
    _html.replace("#DEVICE_NAME#", buf.deviceName);
    _html.replace("#SSID#", buf.ssid);
    _html.replace("#PASS#", buf.pass);
    
    uint8_t trigger_count = 0;
    String trigger_list = "";
    for(uint8_t i = 0; i < MAX_TRIGGER; i++){
      if(String(buf.triggers[i].pin) != "" && String(buf.triggers[i].ip) != "" && String(buf.triggers[i].port) != ""){
        trigger_count++; 
        trigger_list += "<tr>\r\n";
        trigger_list += "<th>" + String(i+1, DEC) + "</th>";
        trigger_list += "<td>" + String(buf.triggers[i].pin) + "<input type='hidden' name='triggers_" + String(i, DEC) + "_pin' value='" + String(buf.triggers[i].pin) + "' ></td>";
        trigger_list += "<td>" + String(buf.triggers[i].ip) + "<input type='hidden' name='triggers_" + String(i, DEC) + "_ip' value='" + String(buf.triggers[i].ip) + "' ></td>";
        trigger_list += "<td>" + String(buf.triggers[i].port) + "<input type='hidden' name='triggers_" + String(i, DEC) + "_port' value='" + String(buf.triggers[i].port) + "' ></td>";
        trigger_list += "</tr>";
      }
      delay(1);
    }
    _html.replace("#TRIGGER_LIST#", trigger_list);
    _html.replace("#TRIGGER_COUNT#", String(trigger_count, DEC));
    _html.replace("#MAX_TRIGGER#", String(MAX_TRIGGER, DEC));
    if(isServerMode){
      _html.replace("#IP#", "192.168.4.1");
      _html.replace("#MODE#", "Server Mode");
      _html.replace("#CSS#", "");
    }else{
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      _html.replace("#IP#", ipStr);
      _html.replace("#MODE#", "Client Mode");
      _html.replace("#CSS#", css);
    }
  }
  server.send(200, "text/html", _html);
  return true;
}

void initEEPROM(){
  Serial.println("[ Initialize EEPROM ]");
  for (uint8_t i = 0 ; i < EEPROM_SIZE ; i++) {
    EEPROM.write(i, 0);
  }
  
  String _deviceName = "default";
  String _ssid = "ssid";
  String _pass = "pass";

  CONFIG buf;
  strcpy(buf.deviceName, _deviceName.c_str());
  strcpy(buf.ssid, _ssid.c_str());
  strcpy(buf.pass, _pass.c_str());
  for(uint8_t i = 0; i < MAX_TRIGGER; i++){
    String _null = "";
    strcpy(buf.triggers[i].pin, _null.c_str());
    strcpy(buf.triggers[i].ip, _null.c_str());
    strcpy(buf.triggers[i].port, _null.c_str());
  }
  EEPROM.put<CONFIG>(0, buf);
  EEPROM.commit();

  String text = "<h1>Initialized!</h1>";
  text += "<p>Device Name: ";
  text += buf.deviceName;
  text += "</p>";
  text += "<p>SSID: ";
  text += buf.ssid;
  text += "</p>";
  text += "<p>PASS: ";
  text += buf.pass;
  text += "</p>";
  text += "<p>Triggers</p>\n";
  text += "<ul>\n";
  for(uint8_t i = 0; i < MAX_TRIGGER; i++){
    text += "<li>\n";
    text += "Trigger_" + String(i, DEC) + "<br />\n";
    text += "pin : ";
    text += buf.triggers[i].pin;
    text += "<br />ip : ";
    text += buf.triggers[i].ip;
    text += "<br />port : ";
    text += buf.triggers[i].port;
    text += "</li>\n";
  }
  text += "</ul>\n";
  text += "<a href='/'>Back</a>\n";
  server.send(200, "text/html", text);
  Serial.println("Initialized!");
}

void showState(){
  Serial.println("[ Show State ]");
  CONFIG buf;
  EEPROM.get<CONFIG>(0, buf);

  String text = "<h1>State</h1>";
  text += "<p>Device Name: ";
  text += buf.deviceName;
  text += "</p>";
  text += "<p>SSID: ";
  text += buf.ssid;
  text += "</p>";
  text += "<p>PASS: ";
  text += buf.pass;
  text += "</p>";
  text += "<p>Triggers</p>\n";
  text += "<ul>\n";
  for(uint8_t i = 0; i < MAX_TRIGGER; i++){
    text += "<li>\n";
    text += "Trigger_" + String(i, DEC) + "<br />\n";
    text += "pin : ";
    text += buf.triggers[i].pin;
    text += "<br />ip : ";
    text += buf.triggers[i].ip;
    text += "<br />port : ";
    text += buf.triggers[i].port;
    text += "</li>\n";
  }
  text += "</ul>\n";
  text += "<a href='/'>Back</a>\n";
  server.send(200, "text/html", text);
}

void changeValue(){
  Serial.println("[ Change Value ] " + server.arg("no") + " : " + server.arg("value"));
  String _pin = server.arg("no");
  String _value = server.arg("value");
  Serial.println(_pin.toInt());
  Serial.println(_value.toInt());
  analogWrite(_pin.toInt(), _value.toInt());
  server.send(200, "text/html", "set");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get<CONFIG>(0, buf);
  Serial.println("--------[ EEPROM ]--------");
  Serial.print("deviceName : ");
  Serial.println(buf.deviceName);
  device_name = buf.deviceName;
  Serial.print("ssid : ");
  Serial.println(buf.ssid);
  Serial.print("pass : ");
  Serial.println(buf.pass);
  _triggers_count = sizeof(buf.triggers) / sizeof(buf.triggers[0]);
  uint8_t _count = 0;
  while(_count < _triggers_count){
    Serial.println("triggers[ " + String(_count) + " ] : " + String(buf.triggers[_count].pin) + " / " + String(buf.triggers[_count].ip) + ":" + String(buf.triggers[_count].port));
    _count++;
  }
  Serial.println("--------------------------");
  
  if(!SPIFFS.begin()){
    Serial.println("SPIFES.begin faile");
    return;
  }

  // network setup
  if(!setup_client()) setup_server();

  if (MDNS.begin(device_name)) {
    Serial.println("MDNS responder started");
    Serial.print("http://");
    Serial.print(device_name);
    Serial.println(".local/");
  }
  server.on("/", showIndex);
  server.on("/pin", changeValue);
  server.on("/config", setWiFiConfig);
  server.on("/init", initEEPROM);
  server.on("/state", showState);
  server.begin();
  MDNS.addService("http", "tcp", 80); 

  // pin mode
  pinMode(0, OUTPUT);
  //pinMode(1, OUTPUT); // pin1 can't use
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  pinMode(16, INPUT);

  Serial.println("--------------------------");
}

void loop() {
  server.handleClient();
  if(pinState._12 != digitalRead(12)){
    pinState._12 = digitalRead(12);
    callTrigger(12, pinState._12);
  }
  if(pinState._13 != digitalRead(13)){
    pinState._13 = digitalRead(13);
    callTrigger(13, pinState._13);
  }
  if(pinState._14 != digitalRead(14)){
    pinState._14 = digitalRead(14);
    callTrigger(14, pinState._14);
  }
  if(pinState._15 != digitalRead(15)){
    pinState._15 = digitalRead(15);
    callTrigger(15, pinState._15);
  }
  if(pinState._16 != digitalRead(16)){
    pinState._16 = digitalRead(16);
    callTrigger(16, pinState._16);
  }
  delay(1);
}
