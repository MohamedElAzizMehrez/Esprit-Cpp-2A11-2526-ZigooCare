#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//General
String msg = "";
bool loggedin = false;

//Fingerprint
SoftwareSerial mySerial(2, 3);// RX, TX Arduino
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//buzzer
#define PIN_CAPTEUR_PLUIE 9
#define PIN_BUZZER 8
String risque = "0";

//Water_level
#define SENSOR_1 A0
#define SENSOR_2 A1
#define SENSOR_3 A2
#define SEWER_ID_1 1
#define SEWER_ID_2 2
#define SEWER_ID_3 3
#define SEND_INTERVAL 1000
unsigned long lastSend = 0;

//capacity
#define capacity1 4
#define capacity2 5
#define capacity3 6

bool etatSewer1 = false;
bool etatSewer2 = false;
bool etatSewer3 = false;

bool last1 = HIGH;
bool last2 = HIGH;
bool last3 = HIGH;

// Temps anti-rebond sans delay
unsigned long lastDebounce1 = 0;
unsigned long lastDebounce2 = 0;
unsigned long lastDebounce3 = 0;

const unsigned long debounceTime = 50;

//Center
#define CenterId 1
LiquidCrystal_I2C lcd(0x27, 16, 2);
String wasteLine = "";
String fullLine = "";
String wasteLinetemp = "";
String fullLinetemp = "";
bool connected = false;
int CenterEtat=0,CenterEtattemp=0;

// ================= SETUP =================
void setup() {
  Serial.begin(9600);

  // Fingerprint
  mySerial.begin(57600);
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint : Found fingerprint sensor!");
  } else {
    Serial.println("Fingerprint : Did not find fingerprint sensor!");
  }
  finger.LEDcontrol(false);

  //buzzer
  pinMode(PIN_CAPTEUR_PLUIE, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  //capacity
  pinMode(capacity1, INPUT_PULLUP);
  pinMode(capacity2, INPUT_PULLUP);
  pinMode(capacity3, INPUT_PULLUP);

//water_level
 pinMode(SENSOR_1, INPUT);
  pinMode(SENSOR_2, INPUT);
   pinMode(SENSOR_3, INPUT);

  //Center
  lcd.init();
  lcd.noBacklight();
}

// ================= LOOP =================
void loop() {

  // Buffer
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      msg.trim();
      handleMessage(msg);
      msg = "";
    } else {
      msg += c;
    }
  }

  if (loggedin) {

    //buzzer
    int etatPluie = digitalRead(PIN_CAPTEUR_PLUIE);
    if(etatPluie==LOW && risque == "1") { 
      alerteInondation();
    }
    else {
      noTone(PIN_BUZZER);
    }

    //water_level
    if (millis() - lastSend >= SEND_INTERVAL)
      {
        lastSend = millis();

        int value1 = analogRead(SENSOR_1);
        int value2 = analogRead(SENSOR_2);
        int value3 = analogRead(SENSOR_3);

        Serial.print("SEWER:");
        Serial.print(SEWER_ID_1);
        Serial.print(":");
        Serial.print(value1);
        Serial.print(";");

        Serial.print(SEWER_ID_2);
        Serial.print(":");
        Serial.print(value2);
        Serial.print(";");

        Serial.print(SEWER_ID_3);
        Serial.print(":");
        Serial.println(value3);
      }

  //capacity
  unsigned long currentMillis = millis();

  bool current1 = digitalRead(capacity1);
  bool current2 = digitalRead(capacity2);
  bool current3 = digitalRead(capacity3);

  // ----- Bouton 1 -----
  if (last1 == HIGH && current1 == LOW && (currentMillis - lastDebounce1 >= debounceTime)) {
    etatSewer1 = !etatSewer1;

    if (etatSewer1)
      Serial.println("capacity 1 ON");
    else
      Serial.println("capacity 1 OFF");

    lastDebounce1 = currentMillis;
  }

  // ----- Bouton 2 -----
  if (last2 == HIGH && current2 == LOW && (currentMillis - lastDebounce2 >= debounceTime)) {
    etatSewer2 = !etatSewer2;

    if (etatSewer2)
      Serial.println("capacity 2 ON");
    else
      Serial.println("capacity 2 OFF");

    lastDebounce2 = currentMillis;
  }

  // ----- Bouton 3 -----
  if (last3 == HIGH && current3 == LOW && (currentMillis - lastDebounce3 >= debounceTime)) {
    etatSewer3 = !etatSewer3;

    if (etatSewer3)
      Serial.println("capacity 3 ON");
    else
      Serial.println("capacity 3 OFF");

    lastDebounce3 = currentMillis;
  }

  last1 = current1;
  last2 = current2;
  last3 = current3;

      //Center
      DisplayCenter();
  }
}

void handleMessage(String m) {

  if (m == "connect") {
    loggedin = true;
  }
  else if (m == "disconnect") {
    loggedin = false;
    lcd.clear();
    lcd.noBacklight();
    connected = false;
    CenterEtat=0;
    wasteLine="";
    fullLine="";
    noTone(PIN_BUZZER);
  } 

  // Fingerprint
  if (m == "enroll") {
    enrollFingerprint();
  }
  else if (m == "scan") {
    scanFingerprint();
  }
  else if (m == "delete") {
    finger.emptyDatabase();
  }
  else if (m == "led_on") {
    finger.LEDcontrol(true);
  }
  else if (m == "led_off") {
    finger.LEDcontrol(false);
  }

  //buzzer
  else if (m == "0" || m == "1") { 
    risque = m;
  }

  //Center
  if (m == "OK" && !connected) {
    connected = true;
    lcd.backlight();
    lcd.clear();
    return;
  }

  if (m == "Center_Saturated") {
    CenterEtat=1;
    wasteLinetemp="";
    fullLinetemp="";
    return;
  }

  if (m == "Center_Inactive") {
    CenterEtat=2;
    wasteLinetemp="";
    fullLinetemp="";
    return;
  }

  if (m.startsWith("Waste:")) {
    CenterEtat=0;
    CenterEtattemp=0;
    wasteLine = m;
    return;
  }

  else if (m.startsWith("Full-in:")) {
    CenterEtat=0;
    CenterEtattemp=0;
    fullLine = m;
    return;
  }

  if (m.startsWith("give-center-id")) {
    sendID();
  }
}

// Fingerprint functions
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

void enrollFingerprint() {
  int id;

  Serial.println("Fingerprint : Please type in the ID # (from 1 to 127)");

  id = readnumber();

  if (id == 128) {
    Serial.println("Fingerprint : Enrollment cancelled");
    return;
  }

  Serial.print("Fingerprint : Enrolling ID #");
  Serial.println(id);

  int p = -1;

  Serial.println("Fingerprint : Put new finger");
  while (p != FINGERPRINT_OK) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'c') {
        Serial.println("Fingerprint : Scan cancelled");
        return;
      }
    }
    p = finger.getImage();
  }

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("Fingerprint : Error");
    return;
  }

  Serial.println("Fingerprint : Remove finger");
  delay(2000);

  Serial.println("Fingerprint : Place same finger again");
  while (finger.getImage() != FINGERPRINT_OK)
  {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'c') {
        Serial.println("Fingerprint : Scan cancelled");
        return;
      }
    }
  };

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("Fingerprint : Error");
    return;
  }

  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("Fingerprint : Error");
    return;
  }

  if (finger.storeModel(id) == FINGERPRINT_OK) {
    Serial.println("Fingerprint : Stored!");
  } else {
    Serial.println("Fingerprint : Error");
  }
}

void scanFingerprint() {
  int p = -1;

  Serial.println("Fingerprint : Put your finger");

  while (p != FINGERPRINT_OK) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'c') {
        Serial.println("Fingerprint : Scan cancelled");
        return;
      }
    }
    
    p = finger.getImage();

    if (p == FINGERPRINT_NOFINGER) {
      //Serial.print(".");
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println("Fingerprint : Error");
      return;
    } else if (p == FINGERPRINT_IMAGEFAIL) {
      Serial.println("Fingerprint : Error");
      return;
    }
    delay(100);
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprint : Error");
    return;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Fingerprint : Found ID #");
    Serial.println(finger.fingerID);
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Fingerprint : Unknown fingerprint");
  } else {
    Serial.println("Fingerprint : Error");
  }

  delay(2000);
}

//buzzer
void alerteInondation() {
  static unsigned long previousMillis = 0;
  static int etat = 0;

  unsigned long currentMillis = millis();

  switch (etat) {
    case 0:
      tone(PIN_BUZZER, 800);
      previousMillis = currentMillis;
      etat = 1;
      break;

    case 1:
      if (currentMillis - previousMillis >= 250) {
        noTone(PIN_BUZZER);
        previousMillis = currentMillis;
        etat = 2;
      }
      break;

    case 2:
      if (currentMillis - previousMillis >= 80) {
        tone(PIN_BUZZER, 1200);
        previousMillis = currentMillis;
        etat = 3;
      }
      break;

    case 3:
      if (currentMillis - previousMillis >= 250) {
        noTone(PIN_BUZZER);
        previousMillis = currentMillis;
        etat = 4;
      }
      break;

    case 4:
      if (currentMillis - previousMillis >= 80) {
        etat = 0; // recommencer
      }
      break;
  }
}

//Center
void sendID() {
  Serial.print("Center-id:");
  Serial.println(CenterId);
}

void DisplayCenter(){
  if(connected){
    if(CenterEtat==1){
      if(CenterEtat!=CenterEtattemp){
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print("CENTER");
        lcd.setCursor(4,1);
        lcd.print("Saturated");
        CenterEtattemp=CenterEtat;
      }
    }else if(CenterEtat==0){
      if(wasteLinetemp!=wasteLine || fullLinetemp!=fullLine){
        lcd.clear();

        lcd.setCursor(1,0);
        lcd.print(wasteLine);

        lcd.setCursor(1,1);
        lcd.print(fullLine);
        wasteLinetemp=wasteLine;
        fullLinetemp=fullLine;
      }
    }else{
        if(CenterEtat!=CenterEtattemp){
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print("CENTER");
        lcd.setCursor(4,1);
        lcd.print("Inactive");
        CenterEtattemp=CenterEtat;
      }
    }
  }
}