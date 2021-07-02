 
#include <Wire.h>

#define TINY_GSM_MODEM_SIM7600
#define ENABLE_GPS true
#define TINY_GSM_TEST_GPRS true
#define TINY_GSM_TEST_TIME true

#define CF_OL24 &Orbitron_Light_24
#define CF_OL32 &Orbitron_Light_32

#include <Adafruit_MLX90614.h>

#include <Bounce2.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "Logo.h"
#include "tbklogo.h";
#include "FS.h"

#include "RTClib.h"
#include "Free_Fonts.h"
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <TinyGsmClient.h>
#include <Ticker.h>


#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESP32Time.h>
#include <ArduinoOTA.h>
#include <WiFi.h>

ESP32Time rtc;

Adafruit_MPU6050 mpu;

// #define TINY_GSM_MODEM_SIM7000

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT hwSerial
HardwareSerial hwSerial(1);

// Web server
WebServer server(80);
int tftMax = 160;
const char *softAP_ssid = "SOSBoxByTBK";
const char *softAP_password = "12345678";

const char *myHostname = "SOSBox32";

const long interval = 5000;  //millisecond
unsigned long previousMillis = 0;

const long interval1 = 200;  //millisecond
unsigned long previousMillis1 = 0;

const long interval2 = 210;  //millisecond
unsigned long previousMillis2 = 0;

int currentG = 0;
const int bufferSize = 100;
int circularX[bufferSize];
int circularY[bufferSize];
int circularZ[bufferSize];
int countAccele = 1;
int countDown = 0;
boolean isButton = false;

const byte DNS_PORT = 53;
DNSServer dnsServer;

Preferences preferences;

IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

char driver[70] = "";
//Phone List
char phone1[12] = "";
char phone2[12] = "";
char phone3[12] = "";
char phone4[12] = "";
char phone5[12] = "";
char group[4] = "";
char med[50] = "";
char gsensor[10] = "";
float lat, lon;
bool disconnected = 0;
int csq = 0;
String cop = "";
String oper = "";
String maxAcceleStr = "";

String message = "";
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite headerEnv = TFT_eSprite(&tft);
TFT_eSprite center = TFT_eSprite(&tft);
TFT_eSprite footerEnv = TFT_eSprite(&tft);
TFT_eSprite tftButton = TFT_eSprite(&tft);
TFT_eSprite img = TFT_eSprite(&tft);  // Declare Sprite object "spr" with pointer to "tft" object

TFT_eSprite accele = TFT_eSprite(&tft);
#define TFT_GREY 0x5AEB // New colour

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

#include <StreamDebugger.h>
StreamDebugger debugger(hwSerial, SerialMon);
TinyGsm modem(debugger);

const char apn[]  = "internet";   //apn
const char gprsUser[] = "";   //username
const char gprsPass[] = "";   //pass

Ticker tick;
sensors_event_t a, g, temp;

#define PIN_TX                  14
#define PIN_RX                  27
#define UART_BAUD               115200
#define PWR_PIN                 4
//#define LED_PIN                 12
#define POWER_PIN               23  // 
//#define IND_PIN                 36

#define BUTTON_PIN_A 25
#define BUTTON_PIN_B 26


volatile unsigned long debounceSOS;

volatile unsigned int delayTimerSOS = 10000;
unsigned long previosSOS = 0;
#define buzzer 12
boolean value1 = 0;
boolean value2 = 0;
int bTime = 1;
bool sendCancel = false;


float s1 = 0.0;
float s2 = 0.0;

float R1 = 15000.0;
float R2 = 2000.0;


// Instantiate a Bounce object
Bounce debouncerA = Bounce();

// Instantiate another Bounce object
Bounce debouncerB = Bounce();


String latStr = "";
String lonStr = "";

int shownX = 0;
int shownY = 0;
int shownZ = 0;



void splash() {
  int xpos =  0;
  int ypos = 40;
  tft.init();
  // Swap the colour byte order when rendering
  tft.setSwapBytes(true);
  tft.setRotation(1);  // landscape

  tft.fillScreen(TFT_BLACK);
  // Draw the icons

  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TC_DATUM); // Centre text on x,y position
  tft.setTextSize(1);
  tft.pushImage(tft.width() / 2 - tbklogoWidth / 2, 55, tbklogoWidth, tbklogoHeight, tbklogo);
  tft.setFreeFont(FSB9);
  xpos = tft.width() / 2; // Half the screen width
  ypos = 170;
  tft.drawString("SOSBox", xpos, ypos, GFXFF);  // Draw the text string in the selected GFX free font

  delay(3000);

  tft.setTextFont(GLCD);
  tft.setRotation(1);

  // Select the font
  ypos += tft.fontHeight(GFXFF);                      // Get the font height and move ypos down
  tft.setFreeFont(FSB9);
  //  tft.pushImage(tft.width()/2 - (Splash2Width/2)-15, 3, Splash2Width, Splash2Height, Splash2);



  delay(1200);
  tft.setTextPadding(180);
  //  tft.setTextColor(TFT_GREEN);
  tft.setTextDatum(MC_DATUM);
  tft.setFreeFont(FSS9);



  tft.fillScreen(TFT_BLACK);
  Serial.println("Start...");
  tft.pushImage(tft.width() / 2 - logoWidth / 2, 20, logoWidth, logoHeight, Logo);
  tft.setTextColor(TFT_GREEN);

  Serial.println("end");
}

void clearInbox() {
  //  AT+CMGD=0,4
  Serial.println("try to clear Inbox");
  modem.clearSMS();
}



/**********************************************  WIFI Client 注意编译时要设置此值 *********************************
   wifi client
*/
const char* ssid = "greenioGuest"; //replace "xxxxxx" with your WIFI's ssid
const char* password = "green7650"; //replace "xxxxxx" with your WIFI's password

//WiFi&OTA 参数
#define HOSTNAME "SOSBoxByTBK_BeTa"
#define PASSWORD "12345678" //the password for OTA upgrade, can set it in any char you want

/************************************************  注意编译时要设置此值 *********************************
   是否使用静态IP
*/
#define USE_STATIC_IP false
#if USE_STATIC_IP
IPAddress staticIP(192, 168, 1, 22);
IPAddress gateway(192, 168, 1, 9);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2(114, 114, 114, 114);
#endif

/*******************************************************************
   OLED Arguments
*/
//#define RST_OLED 16                     //OLED Reset引脚，需要手动Reset，否则不显示
#define OLED_UPDATE_INTERVAL 500        //OLED屏幕刷新间隔ms
//SSD1306 display(0x3C, 4, 15);           //引脚4，15是绑定在Kit 32的主板上的，不能做其它用


/********************************************************************
   OTA升级配置
*/
void setupOTA()
{
  //Port defaults to 8266
  //ArduinoOTA.setPort(8266);

  //Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(HOSTNAME);

  //No authentication by default
  ArduinoOTA.setPassword(PASSWORD);

  //Password can be set with it's md5 value as well
  //MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  //ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]()
  {
    //    Heltec.display->clear();
    //    Heltec.display->setFont(ArialMT_Plain_10);        //设置字体大小
    //    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);//设置字体对齐方式
    //    Heltec.display->drawString(0, 0, "Start Updating....");

    Serial.printf("Start Updating....Type:%s\n", (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem");
  });

  ArduinoOTA.onEnd([]()
  {
    //    Heltec.display->clear();
    //    Heltec.display->drawString(0, 0, "Update Complete!");
    Serial.println("Update Complete!");

    ESP.restart();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    String pro = String(progress / (total / 100)) + "%";
    int progressbar = (progress / (total / 100));
    //int progressbar = (progress / 5) % 100;
    //int pro = progress / (total / 100);

    //    Heltec.display->clear();
    //    Heltec.display->drawProgressBar(0, 32, 120, 10, progressbar);    // draw the progress bar
    //    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);          // draw the percentage as String
    //    Heltec.display->drawString(64, 15, pro);
    //    Heltec.display->display();
    String temp = "Start update OTA...";
    temp.concat(pro);
    drawStatus(pro);
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error)
  {
    Serial.printf("Error[%u]: ", error);
    String info = "Error Info:";
    switch (error)
    {
      case OTA_AUTH_ERROR:
        info += "Auth Failed";
        Serial.println("Auth Failed");
        break;

      case OTA_BEGIN_ERROR:
        info += "Begin Failed";
        Serial.println("Begin Failed");
        break;

      case OTA_CONNECT_ERROR:
        info += "Connect Failed";
        Serial.println("Connect Failed");
        break;

      case OTA_RECEIVE_ERROR:
        info += "Receive Failed";
        Serial.println("Receive Failed");
        break;

      case OTA_END_ERROR:
        info += "End Failed";
        Serial.println("End Failed");
        break;
    }

    //    Heltec.display->clear();
    //    Heltec.display->drawString(0, 0, info);
    ESP.restart();
  });

  ArduinoOTA.begin();
}


/*********************************************************************
   setup wifi
*/
void setupWIFI()
{
  //  Heltec.display->clear();
  //  Heltec.display->drawString(0, 0, "Connecting...");
  //  Heltec.display->drawString(0, 10, String(ssid));
  //  Heltec.display->display();
  //
  Serial.print("Connecting...");
  Serial.println(String(ssid));
  //连接WiFi，删除旧的配置，关闭WIFI，准备重新配置
  WiFi.disconnect(true);
  delay(1000);

  WiFi.mode(WIFI_STA);
  //WiFi.onEvent(WiFiEvent);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);    //断开WiFi后自动重新连接,ESP32不可用
  //WiFi.setHostname(HOSTNAME);
  WiFi.begin(ssid, password);
#if USE_STATIC_IP
  WiFi.config(staticIP, gateway, subnet);
#endif

  //等待5000ms，如果没有连接上，就继续往下
  //不然基本功能不可用
  byte count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    delay(500);
    Serial.print(".");
  }

  //  Heltec.display->clear();
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("Connecting...OK.");
  else
    Serial.println("Connecting...Failed");

}

void setup()
{
  // Set console baud rate
  SerialMon.begin(115200);
  tft.fillScreen(TFT_BLACK);
  // TFT
  splash();
  // MLX
  mlx.begin();


  Serial.begin(115200);
  Serial.println(F("Start: Light Weight depounce blink without delay code"));
  pinMode(BUTTON_PIN_A, INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncerA.attach(BUTTON_PIN_A);
  debouncerA.interval(15); // interval in ms

  // Setup the second button with an internal pull-up :
  pinMode(BUTTON_PIN_B, INPUT_PULLUP);
  // After setting up the button, setup the Bounce instance :
  debouncerB.attach(BUTTON_PIN_B);
  debouncerB.interval(15); // interval in ms


  pinMode(buzzer, OUTPUT);

  // POWER_PIN : This pin controls the power supply of the SIM7600
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);

  clearInbox();

  DBG("Wait...");

  Serial.println();
  preferences.begin("CapPortAdv", false);
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */

  WiFi.softAP(softAP_ssid, softAP_password);

  delay(1500); // Without delay I've seen the IP address blank
  WiFi.softAPConfig(apIP, apIP, netMsk);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", handleRoot);
  server.on("/setting", handleSetting);
  server.on("/save_setting", handleSettingSave);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.onNotFound ( handleNotFound );
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  loadCredentials(); // Load WLAN credentials from network
  Serial.println(disconnected);
  //  while (disconnected == 0) { //&& (strcmp(phone1,"") == 0 || strcmp(group,"") == 0 || strcmp(gsensor,"") == 0)) {
  for (int i = 0; i <= 600; i++) {
    delay(100);
    dnsServer.processNextRequest();
    //  HTTP
    Serial.println(i);
    server.handleClient();
//    int sec = i%600000;
//    countDown = 90 - i;
    String temp = "WiFi Setting... ";
//    temp.concat(countDown);
//    temp.concat(" s");
    tft.fillRect(0, 200, 320, 25, TFT_BLACK);
    tft.drawString(temp, tft.width() / 2, 210);
    Serial.println(temp);
    if (disconnected > 0) break;
 
    debouncerA.update();

    // Get the updated value :
    value1 = debouncerA.read();
    
    if (  value1 == LOW ) {
      Serial.print("A:");  Serial.println(value1);

      break;
    }
    
  }
  WiFi.softAPdisconnect(false);
  WiFi.enableAP(false);
  // Acc
  pinMode(32, OUTPUT);
  digitalWrite(32, HIGH);
  // Butt
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
  // Buzz
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  while (1) {
    if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 chip");
      delay(10);
    } else {
      break;
    }
  }
  Serial.println("MPU6050 Found!");
  tft.fillRect(0, 190, 350, 320, 0x0000);
  tft.drawString("Connecting...", 150, 210);
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }

  hwSerial.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  while (1) {
    DBG("Initializing modem...");
    if (!modem.restart()) {
      DBG("Failed to restart modem, delaying 10s and retrying");
      delay(2000);
    } else {
      String name = modem.getModemName();
      DBG("Modem Name: ", name);

      String modemInfo = modem.getModemInfo();
      DBG("Modem Info: ", modemInfo);
      // Test is complete Set it to sleep mode
      break;
    }
  }
  //Set to GSM mode, please refer to manual 5.11 AT+CNMP Preferred mode selection for more parameters
  String result;
  do {
    result = modem.setNetworkMode(13);
    delay(500);
  } while (result != "OK");



#if TINY_GSM_TEST_GPRS
  DBG("Connecting to", apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    delay(2000);
    return;
  }

  bool res = modem.isGprsConnected();
  DBG("GPRS status:", res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  DBG("CCID:", ccid);

  String imei = modem.getIMEI();
  DBG("IMEI:", imei);

  cop = modem.getOperator();

  DBG("Operator:", cop);

  IPAddress local = modem.localIP();
  DBG("Local IP:", local);

  csq = modem.getSignalQuality();
  DBG("Signal quality:", csq);

#endif



  Serial.print("Set GNSS Mode BEIDOU : ");
  String resp = modem.setGNSSMode(0, 1);
  Serial.println(resp);
#if ENABLE_GPS
  /**
    CGNSSMODE: <gnss_mode>,<dpo_mode>
    This command is used to configure GPS, GLONASS, BEIDOU and QZSS support mode.
    gnss_mode:
    0 : GLONASS
    1 : BEIDOU
    2 : GALILEO
    3 : QZSS
    dpo_mode :
    0 disable
    1 enable
  */

  enableGPS( );

#endif



#if TINY_GSM_TEST_TIME
  int year3 = 0;
  int month3 = 0;
  int day3 = 0;
  int hour3 = 0;
  int min3 = 0;
  int sec3 = 0;
  float timezone = 7;
  for (int8_t i = 5; i; i--) {
    DBG("Requesting current network time");
    if (modem.getNetworkTime(&year3, &month3, &day3, &hour3, &min3, &sec3,
                             &timezone)) {
      DBG("Year:", year3, "\tMonth:", month3, "\tDay:", day3);
      DBG("Hour:", hour3, "\tMinute:", min3, "\tSecond:", sec3);
      DBG("Timezone:", timezone);
      break;
    } else {
      DBG("Couldn't get network time, retrying in 15s.");
      delay(3000L);
    }
  }
  DBG("Retrieving time again as a string");
  String time = modem.getGSMDateTime(DATE_FULL);
  DBG("Current Network Time:", time);
  rtc.setTime(sec3, min3, hour3, day3, month3, year3);  // 17th Jan 2021 15:24:30

#endif

  setupWIFI();
  setupOTA();
 
  tft.fillScreen(TFT_BLACK);            // Clear screen
  tft.fillRect(5, 185, tft.width() - 15, 5, TFT_BLUE); // Print the test text in the custom font
  tft.fillRect(70, 185, tft.width() - 15, 5, TFT_GREEN); // Print the test text in the custom font
  tft.fillRect(135, 185, tft.width() - 15, 5, TFT_YELLOW); // Print the test text in the custom font
  tft.fillRect(200, 185, tft.width() - 15, 5, TFT_ORANGE); // Print the test text in the custom font
  tft.fillRect(260, 185, tft.width() - 15, 5, TFT_RED); // Print the test text in the custom font
  previousMillis = millis();
  Serial.println("----------------------------------------");
  Serial.print("group:"); Serial.println(group);
  Serial.print("med:"); Serial.println(med);
  message.concat("Driver:");
  message.concat(driver);
  message.concat(" Group:");
  message.concat(group);
  message.concat(" Drug Allergy:");
  message.concat(med);
  message.concat(" ");
  Serial.println(message);
  Serial.println("----------------------------------------");
  drawHeader();
  drawAccele();
  drawFooter();



  Serial.println("Initialize...");
  value1 = HIGH;
  value2 = HIGH;

}


float voltMeasure(int Pin)
{
  unsigned int vRAW = 0;
  float Vout = 0.0;
  float Vin = 0.0;


  vRAW = analogRead(Pin);

  Vout = (vRAW * 3.3) / 4096;
  Vin = Vout / (R2 / (R1 + R2));
  if (Vin < 0.05)
  {
    Vin = 0.0;
  }

  return Vin + 1;
}


void drawHeader() {

  headerEnv.createSprite(320, 30);
  //  header.fillSprite(TFT_GREEN);
  headerEnv.setFreeFont(FS9);
  headerEnv.setTextColor(TFT_RED);
  headerEnv.setTextSize(1);

  headerEnv.setTextSize(1);           // Font size scaling is x1
  headerEnv.drawString(getOper(), 5, 5, GFXFF); // Print the test text in the custom font
  headerEnv.setTextColor(TFT_GREEN);
  headerEnv.drawString(getSignal(), 90, 5, GFXFF); // Print the test text in the custom font

  headerEnv.drawNumber(getBattery(), 275, 5);
  headerEnv.drawString("%", 300, 5, GFXFF); // Print the test text in the custom font

  headerEnv.pushSprite(5, 5);
  headerEnv.deleteSprite();

}

void drawStatus(String b) {
  tftButton.createSprite(300, 20);
  //  tftButton.fillSprite(TFT_ORANGE);
  tftButton.setFreeFont(FS9);
  tftButton.setTextColor(TFT_WHITE);
  tftButton.setTextSize(1);

  tftButton.drawString(b, 5, 0, GFXFF); // Print the test text in the custom font



  tftButton.pushSprite(5, 195);
  tftButton.deleteSprite();

}


void drawCenter( )
{
  // Create a sprite 80 pixels wide, 50 high (8kbytes of RAM needed)
  center.createSprite(260, 30);
  //  center.fillSprite(TFT_YELLOW);
  center.fillScreen(TFT_BLACK);
  center.setTextSize(1);
  center.setTextColor(TFT_WHITE);  // White text, no background colour
  // Set text coordinate datum to middle centre
  center.setTextDatum(MC_DATUM);
  // Draw the number in middle of 80 x 50 sprite
  center.setFreeFont(FS9);
  center.setTextColor(TFT_YELLOW);  // White text, no background colour
  maxAcceleStr = "Max: " + String(gsensor) + " m/s2";
  center.drawString(maxAcceleStr, 5, 175, GFXFF);

  center.drawString(getTemp(), 210, 175, GFXFF); // Print the test text in the custom font
  center.pushSprite(30, 145);
  // Delete sprite to free up the RAM
  accele.deleteSprite();
}

void drawFooter() {
  drawCenter();
  footerEnv.createSprite(320, 25);
  //   footerEnv.fillSprite(TFT_GREEN);
  footerEnv.setFreeFont(FS9);
  footerEnv.setTextColor(TFT_WHITE);
  footerEnv.setTextSize(1);

  footerEnv.drawString(rtc.getTime("%A, %B %d %Y %H:%M"), 5, 0, GFXFF);

  footerEnv.pushSprite(55, 215);
  footerEnv.deleteSprite();

}

void drawAccele()
{
  // Create a sprite 80 pixels wide, 50 high (8kbytes of RAM needed)
  accele.createSprite(260, 100);
  //    accele.fillSprite(TFT_ORANGE);
  accele.setTextSize(2);           // Font size scaling is x1
  //  accele.setFreeFont(CF_OL24);  // Select free font
  accele.setFreeFont(CF_OL32);
  accele.setTextColor(TFT_WHITE);
  accele.setTextSize(4);

  int mid = (tftMax / 2) - 1;

  accele.setTextColor(TFT_WHITE);  // White text, no background colour
  // Set text coordinate datum to middle centre
  accele.setTextDatum(MC_DATUM);
  // Draw the number in middle of 80 x 50 sprite



  accele.drawNumber(calX() / 10, mid, 30);

  accele.pushSprite(30, 45);
  // Delete sprite to free up the RAM
  accele.deleteSprite();
}

void loop()
{
  ArduinoOTA.handle();
  unsigned long ms = millis();
  if (ms % 1000 == 0)
  {
    Serial.println("hello，OTA now");
  }
  mpu.getEvent(&a, &g, &temp);

  tft.setTextSize(2);


  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    drawHeader();
    drawFooter();
  
    Serial.println(getGPS());
    previousMillis = currentMillis;
  }


  if (currentMillis - previousMillis1 >= interval1)
  {

    circularX[0] = a.acceleration.x * 10;
    circularY[0] = a.acceleration.y * 10;
    circularZ[0] = a.acceleration.z * 10;


    previousMillis1 = currentMillis;
  }

  if (currentMillis - previousMillis2 >= interval2)
  {



    circularX[1] = a.acceleration.x * 10;
    circularY[1] = a.acceleration.y * 10;
    circularZ[1] = a.acceleration.z * 10;

    //    Serial.print("Acceleration X: ");
    //    Serial.print(circularX[1]);
    //    Serial.print(", Y: ");
    //    Serial.print(circularY[1]);
    //    Serial.print(", Z: ");
    //    Serial.println(circularZ[1]);
    //        Serial.println(" m/s^2");
    drawAccele();
    countAccele = 1;
    previousMillis2 = currentMillis;
  }


  if (( value2 == LOW)  ) {
    //    if (( value2 == LOW) && (debounceB   - previousB  >=  1000  ) ){
    Serial.print("value2:");  Serial.println(value2);
    Serial.print("PressedB " );
    digitalWrite(buzzer, LOW);
    sendCancel = true;
    drawStatus("Cancel Sending SMS");
  }

  currentG = atoi(gsensor);
  //  Serial.print(currentG); Serial.print(","); Serial.println(shownX);
  if ( (shownX >= currentG) || (shownX >= currentG) || (shownX >= currentG) ) {
    digitalWrite(buzzer, HIGH);
    sendCancel = false;
    Serial.print(shownX); Serial.print("-------Speed Exceed-----"  ); Serial.println(currentG);

    Serial.print(debounceSOS); Serial.print(" - "); Serial.println(previosSOS);


    //    if (debounceSOS - previosSOS >=  delayTimerSOS  )
    //    {

    for (int i = 0; i <= 1000; i++) {
      //        Serial.print(i); Serial.print(" ");
      debouncerB.update();

      value2 = debouncerB.read();
      Serial.print("sendCancel:"); Serial.println(sendCancel);
      drawStatus("Warning...");
      Serial.print("value2:"); Serial.println(value2);


      if ( value2 == LOW) {


        digitalWrite(buzzer, LOW);

        Serial.print("PressedB " );
        drawStatus("Cancel Sending SMS");
        sendCancel = true;
        break;
      }

    }

    if (sendCancel) {

      Serial.print("sendCancel:"); Serial.println(sendCancel);
    } else  {
      Serial.println("----------------------send SMS-------------------------------");
      digitalWrite(buzzer, LOW);
      sendSMS(true); // raie by exceed limit
      previosSOS = debounceSOS;



    }
    //    }
  }


  debouncerA.update();
  debouncerB.update();
  // Get the updated value :
  value1 = debouncerA.read();
  value2 = debouncerB.read();
  if ( value1 == LOW) {
    Serial.print("value1:");  Serial.println(value1);
    Serial.print("PressedA " );
    buttonA();

  }

}

String getOper() {
  String tempStr = getValue(cop, ' ', 0);

  return getValue(tempStr, ' ', 0);

}


//function to extract decimal part of float
long getDecimal(float val)
{
  int intPart = int(val);
  long decPart = 1000000 * (val - intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places.
  //Change to match the number of decimal places you need
  if (decPart > 0)return (decPart);       //return the decimal part of float number if it is available
  else if (decPart < 0)return ((-1) * decPart); //if negative, multiply by -1
  else if (decPart = 0)return (00);       //return 0 if decimal part of float number is not available
}

void findGPS() {
  if (modem.getGPS(&lat, &lon)) {

    latStr = String(int(lat)) + "." + String(getDecimal(lat)); //combining both whole and decimal part in string with a fullstop between them
    lonStr = String(int(lon)) + "." + String(getDecimal(lon)); //combining both whole and decimal part in string with a fullstop between them
    Serial.print(latStr); Serial.print(","); Serial.println(lonStr);
  } else {
    Serial.println("Get GPS Failed.");
    latStr = "NA";
    lonStr = "NA";

  }
}
String getGPS() {

  String gpsStr = "";
  gpsStr.concat(latStr);
  gpsStr.concat(",");
  gpsStr.concat(lonStr);
  Serial.println(gpsStr);
  return gpsStr;


}

String getSignal() {
  String temp = getValue(cop, ' ', 0);
  String rssiStr = getValue(temp, ' ', 0);
  int rssi = rssiStr.toInt();



  if ((rssi >= 0) && (rssi < 1)) return "Signal:Normal";
  if ((rssi >= 1) && (rssi < 2)) return "Signal:Low";
  if ((rssi >= 2) && (rssi < 31)) return "Signal:Good";
  if ((rssi >= 31) && (rssi < 98)) return "Signal:Very Good";
  if (rssi == 99) return "No Signal";
  if (rssi == 100) return "Signal:Very Low";
  if (rssi == 101) return "Signal:Very Low";
  if ((rssi > 102) && (rssi <= 191)) return "Signal:Very Low";

}


int getBattery() {

  float volt = voltMeasure(35);

  long val;
  val = (long) (volt * 10L);
  val = (val * 10L) / 12;
  if (val > 100)
    val = 100;

  return val;

}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String getTemp() {

  //  Serial.print("Temperature: ");
  //  Serial.print(temp.temperature);
  String tempStr = "Temp: ";
  tempStr.concat(String(temp.temperature));
  tempStr.concat("  C");
  return tempStr;
}
void buttonA() {

  //  A = true;
  Serial.print("ButtonA:");
  Serial.println(sendCancel);
  //  getGPS();
  drawStatus("A");
  sendSMS(false);
}


void enableGPS( ) {

  modem.enableGPS();
  Serial.printf("Try to Get GPS...");





  for (int i = 0; i < 5; i++) {
    if (modem.getGPS(&lat, &lon)) {
      Serial.printf("lat : % f lon : % f\n", lat, lon);
      latStr = String(int(lat)) + "." + String(getDecimal(lat)); //combining both whole and decimal part in string with a fullstop between them
      lonStr = String(int(lon)) + "." + String(getDecimal(lon)); //combining both whole and decimal part in string with a fullstop between them
      Serial.print(latStr); Serial.print(","); Serial.println(lonStr);
      break;
    } else {
      Serial.printf("Get GPS Failed.");
      String temp = "Search for GPS ... ";

      tft.fillRect(0, 200, 320, 25, TFT_BLACK);
      tft.drawString(temp, tft.width() / 2, 210);
    }
    delay(2000);
  }

}

void waitForBreak() {

}



void sendSMS(boolean isExceed) {
  Serial.println("--------------------SMS:----------------");

  //  Serial.println(modem.sendSMS(number, message) ? "OK" : "fail");
  bool res ;
  //String("066" + phone2), "กรุ๊ปเลือด : " + String(group) + " Maps : https://www.google.com/maps/@" + String(lat) + "," + String(lon));
  findGPS();
  String sms = "";
  if (isExceed) {
    sms.concat("Alarm ");
  } else
    sms.concat("SOS ");
  sms.concat(message);

  sms.concat(" Click ");
  sms.concat("https://www.google.com/maps/place/");
  sms.concat(getGPS());
  sms.concat(" ");
  sms.concat(rtc.getTime("%d/%m/%Y %H:%M"));
  sms.trim();
  Serial.println(sms);
  drawStatus("SMS Sending...");

  if (strlen(phone1) > 0 ) {
    Serial.print("send1:");
    res = modem.sendSMS(phone1, sms );
    Serial.println(res);
    if (res)
      drawStatus("Send SMS Success");
    else
      drawStatus("Send SMS Failed");

  }

  if (strlen(phone2) > 0 ) {
    Serial.println("send2");
    res = modem.sendSMS(phone2, sms );
    Serial.println(res);
    if (res)
      drawStatus("Send SMS Success");
    else
      drawStatus("Send SMS Failed");


  }

  if (strlen(phone3) > 0 ) {
    Serial.println("send3");
    res = modem.sendSMS(phone3, sms );
    Serial.println(res);
    if (res)
      drawStatus("Send SMS Success");
    else
      drawStatus("Send SMS Failed");

  }
  if (strlen(phone4) > 0 ) {
    Serial.println("send4");
    res = modem.sendSMS(phone4, sms );
    Serial.println(res);
    if (res)
      drawStatus("Send SMS Success");
    else
      drawStatus("Send SMS Failed");


  }
  if (strlen(phone5) > 0 ) {
    Serial.println("send5");
    res = modem.sendSMS(phone5, sms );
    Serial.println(res);
    if (res)
      drawStatus("Send SMS Success");
    else
      drawStatus("Send SMS Failed");


  }

  sendCancel = false;
}

void loadCredentials() {
  preferences.getString("driver", driver, sizeof(driver));
  preferences.getString("phone1", phone1, sizeof(phone1));
  preferences.getString("phone2", phone2, sizeof(phone2));
  preferences.getString("phone3", phone3, sizeof(phone3));
  preferences.getString("phone4", phone4, sizeof(phone4));
  preferences.getString("phone5", phone5, sizeof(phone5));
  preferences.getString("group", group, sizeof(group));
  preferences.getString("med", med, sizeof(med));
  preferences.getString("gsensor", gsensor, sizeof(gsensor));
  Serial.println("Recovered credentials:");

  if (strcmp(driver, "") != 0) {
    Serial.print("driver : ");
    Serial.println(driver);
  }

  if (strcmp(phone1, "") != 0) {
    Serial.print("phone1 : ");
    Serial.println(phone1);
  }
  if (strcmp(phone2, "") != 0) {
    Serial.print("phone2 : ");
    Serial.println(phone2);
  }
  if (strcmp(phone3, "") != 0) {
    Serial.print("phone3 : ");
    Serial.println(phone3);
  }
  if (strcmp(phone4, "") != 0) {
    Serial.print("phone4 : ");
    Serial.println(phone4);
  }
  if (strcmp(phone5, "") != 0) {
    Serial.print("phone5 : ");
    Serial.println(phone5);
  }
  if (strcmp(group, "") != 0) {
    Serial.print("group : ");
    Serial.println(group);
  }
  if (strcmp(med, "") != 0) {
    Serial.print("med : ");
    Serial.println(med);
  }
  if (strcmp(gsensor, "") != 0) {
    Serial.print("gsensor : ");
    Serial.println(gsensor);
  }
}

/** Store WLAN credentials to Preference */
void saveCredentials() {
  preferences.putString("driver", driver);
  preferences.putString("phone1", phone1);
  preferences.putString("phone2", phone2);
  preferences.putString("phone3", phone3);
  preferences.putString("phone4", phone4);
  preferences.putString("phone5", phone5);
  preferences.putString("group", group);
  preferences.putString("med", med);
  preferences.putString("gsensor", gsensor);
}

void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent("<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>SOSBox</title></head><body> <div style=\"text-align: center;\"> <h1>SOSBox by TBK</h1><a style=\"background-color: red;border: 0;padding: 10px 20px;color: white;font-weight: 600;border-radius: 5px;\" href=\"/setting\">ตั้งค่าข้อมูล / แก้ไขข้อมูล</a> </div></body></html>");
  server.client().stop(); // Stop is needed because we sent no content length
}

boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.print("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void handleSetting() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.sendContent("<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>SOSBox - ตั้งค่าข้อมูล / แก้ไขข้อมูล</title></head><body> <h1>SOSBox - ตั้งค่าข้อมูล / แก้ไขข้อมูล</h1> <form method=\"POST\" action=\"save_setting\">");

  server.sendContent(" <label for=\"m\">Driver : </label><input type=\"text\" name=\"driver\" ");
  if (strcmp(driver, "") != 0) {
    server.sendContent("value=\"" + String(driver) + "\"");
  }
  server.sendContent("><br><label for=\"p1\">เบอร์โทร (1) : </label> <input type=\"text\" name=\"p1\" ");
  if (strcmp(phone1, "") != 0) {
    server.sendContent("value=\"" + String(phone1) + "\"");
  }
  server.sendContent("><br><label for=\"p2\">เบอร์โทร (2) : </label><input type=\"text\" name=\"p2\" ");
  if (strcmp(phone2, "") != 0) {
    server.sendContent("value=\"" + String(phone2) + "\"");
  }
  server.sendContent("><br><label for=\"p3\">เบอร์โทร (3) : </label><input type=\"text\" name=\"p3\" ");
  if (strcmp(phone3, "") != 0) {
    server.sendContent("value=\"" + String(phone3) + "\"");
  }
  server.sendContent("><br><label for=\"p4\">เบอร์โทร (4) : </label><input type=\"text\" name=\"p4\" ");
  if (strcmp(phone4, "") != 0) {
    server.sendContent("value=\"" + String(phone4) + "\"");
  }
  server.sendContent("><br><label for=\"p5\">เบอร์โทร (5) : </label><input type=\"text\" name=\"p5\" ");
  if (strcmp(phone5, "") != 0) {
    server.sendContent("value=\"" + String(phone5) + "\"");
  }
  server.sendContent("><br><label for=\"g\">กรุ๊ปเลือด : </label><select name=\"g\"><option value=\"A\" ");
  if (String(group) == "A") {
    server.sendContent("selected=\"selected\"");
  }
  server.sendContent(">A</option><option value=\"B\" ");
  if (String(group) == "B") {
    server.sendContent("selected=\"selected\"");
  }
  server.sendContent(">B</option><option value=\"O\" ");
  if (String(group) == "O") {
    server.sendContent("selected=\"selected\"");
  }
  server.sendContent(">O</option><option value=\"AB\" ");
  if (String(group) == "AB") {
    server.sendContent("selected=\"selected\"");
  }
  server.sendContent(">AB</option></select><br><label for=\"m\">ยาที่แพ้ : </label><input type=\"text\" name=\"m\" ");
  if (strcmp(med, "") != 0) {
    server.sendContent("value=\"" + String(med) + "\"");
  }
  server.sendContent("><br><label for=\"s\">ความเร่งสูงสุด : </label><input type=\"text\" name=\"s\" ");
  if (strcmp(gsensor, "") != 0) {
    server.sendContent("value=\"" + String(gsensor) + "\"");
  }
  server.sendContent("><br><input type=\"submit\" value=\"ตั้งค่าข้อมูล\"></form></body></html>");
  server.client().stop();
}

void handleSettingSave() {
  Serial.println("setting save");
  server.arg("driver").toCharArray(driver, sizeof(driver) - 1);
  server.arg("p1").toCharArray(phone1, sizeof(phone1) - 1);
  server.arg("p2").toCharArray(phone2, sizeof(phone2) - 1);
  server.arg("p3").toCharArray(phone3, sizeof(phone3) - 1);
  server.arg("p4").toCharArray(phone4, sizeof(phone4) - 1);
  server.arg("p5").toCharArray(phone5, sizeof(phone5) - 1);
  server.arg("g").toCharArray(group, sizeof(group) - 1);
  server.arg("m").toCharArray(med, sizeof(med) - 1);
  server.arg("s").toCharArray(gsensor, sizeof(gsensor) - 1);
  server.sendHeader("Location", "setting", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  disconnected = 1;
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 404, "text/plain", message );
}

boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

int calX() {

  int avgX = 0;
  int avgY = 0;
  int avgZ = 0;
  //  Serial.print("circularX[1]"); Serial.print(circularX[1]);   Serial.print("-"); Serial.println(circularX[0]);
  //  Serial.print("circularY[1]"); Serial.print(circularY[1]);   Serial.print("-"); Serial.println(circularY[0]);
  //  Serial.print("circularZ[1]"); Serial.print(circularZ[1]);   Serial.print("-"); Serial.println(circularZ[0]);

  avgX = circularX[1] - circularX[0];
  avgY = circularY[1] - circularY[0];
  avgZ = circularZ[1] - circularZ[0];
  //  Serial.println(avgX);
  //  Serial.println(avgY);
  //  Serial.println(avgZ);
  //  Serial.println("---");



  if ((avgX > avgY) && (avgX > avgZ)) {
    shownX = avgX;
    //    return avgX;
  }
  if ((avgY > avgZ) && (avgY > avgX)) {
    shownX = avgY;
    //    return avgY;
  }
  if ((avgZ > avgY) && (avgZ > avgX)) {
    shownX = avgZ;
    //    return avgZ;
  } else {
    shownX = 0;
  }

  //  Serial.print("shownX:");
  //  Serial.println(shownX);
  return shownX * 10;
}

/****************************************************
   [通用函数]ESP32 WiFi Kit 32事件处理
*/
void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event)
  {
    case SYSTEM_EVENT_WIFI_READY:               /**< ESP32 WiFi ready */
      break;
    case SYSTEM_EVENT_SCAN_DONE:                /**< ESP32 finish scanning AP */
      break;

    case SYSTEM_EVENT_STA_START:                /**< ESP32 station start */
      break;
    case SYSTEM_EVENT_STA_STOP:                 /**< ESP32 station stop */
      break;

    case SYSTEM_EVENT_STA_CONNECTED:            /**< ESP32 station connected to AP */
      break;

    case SYSTEM_EVENT_STA_DISCONNECTED:         /**< ESP32 station disconnected from AP */
      break;

    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:      /**< the auth mode of AP connected by ESP32 station changed */
      break;

    case SYSTEM_EVENT_STA_GOT_IP:               /**< ESP32 station got IP from connected AP */
    case SYSTEM_EVENT_STA_LOST_IP:              /**< ESP32 station lost IP and the IP is reset to 0 */
      break;

    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:       /**< ESP32 station wps succeeds in enrollee mode */
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:        /**< ESP32 station wps fails in enrollee mode */
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:       /**< ESP32 station wps timeout in enrollee mode */
    case SYSTEM_EVENT_STA_WPS_ER_PIN:           /**< ESP32 station wps pin code in enrollee mode */
      break;

    case SYSTEM_EVENT_AP_START:                 /**< ESP32 soft-AP start */
    case SYSTEM_EVENT_AP_STOP:                  /**< ESP32 soft-AP stop */
    case SYSTEM_EVENT_AP_STACONNECTED:          /**< a station connected to ESP32 soft-AP */
    case SYSTEM_EVENT_AP_STADISCONNECTED:       /**< a station disconnected from ESP32 soft-AP */
    case SYSTEM_EVENT_AP_PROBEREQRECVED:        /**< Receive probe request packet in soft-AP interface */
    case SYSTEM_EVENT_AP_STA_GOT_IP6:           /**< ESP32 station or ap interface v6IP addr is preferred */
      break;

    case SYSTEM_EVENT_ETH_START:                /**< ESP32 ethernet start */
    case SYSTEM_EVENT_ETH_STOP:                 /**< ESP32 ethernet stop */
    case SYSTEM_EVENT_ETH_CONNECTED:            /**< ESP32 ethernet phy link up */
    case SYSTEM_EVENT_ETH_DISCONNECTED:         /**< ESP32 ethernet phy link down */
    case SYSTEM_EVENT_ETH_GOT_IP:               /**< ESP32 ethernet got IP from connected AP */
    case SYSTEM_EVENT_MAX:
      break;
  }
}
