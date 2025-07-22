#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define RX 2
#define TX 3
#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);

String AP = "Galaxy";
String PASS = "password";
String HOST = "api.thingspeak.com";
String PORT = "80";
String API_KEY = "2HMZ6F3OHQ8WJAYI"; // Replace with your ThingSpeak API key
int FIELD_NUMBER = 1; // Field number on ThingSpeak channel

int countTrueCommand;
int countTimeCommand;
boolean found = false;
int valSensor = 1;

SoftwareSerial esp8266(RX, TX);

void setup() {
  Serial.begin(9600);
  SPI.begin(); // Initialize SPI bus
  mfrc522.PCD_Init();
  delay(2500); // Initialize MFRC522 RFID module
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  esp8266.begin(115200);
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
  delay(2000);
  Serial.println("Ready");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("UID tag: ");
    String content = "", id = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : "%20"));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
      id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      id.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    
    int decimal_value = hex_to_decimal(id); // Convert hexadecimal ID to decimal

    Serial.println(id);
    delay(250);
    lcd.setCursor(0, 0);
    lcd.print("ID: " + id);

    sendCommand("AT+CIPMUX=1", 5, "OK");
    sendCommand("AT+CIPSTART=\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
    String url = "GET /update?api_key=" + API_KEY + "&field" + String(FIELD_NUMBER) + "=" + String(decimal_value)+"\r\nHost:api.thingspeak.com\r\n\r\n";
    delay(5000);
    sendCommand("AT+CIPSEND=" + String(url.length() + 4), 10, ">");
    delay(2000);
    esp8266.println(url);
    delay(2000);
    countTrueCommand++;
    sendCommand("AT+CIPCLOSE", 10, "OK");
    lcd.clear();
    delay(10000);
  }
}

void sendCommand(String command, int maxTime, char readReplay[]){
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("Success");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
  delay(1000);
}

int hex_to_decimal(const String& hex_str) {
    return strtol(hex_str.c_str(), NULL, 16);
}
