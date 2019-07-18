//-----------------------------------------------------
//  Project  : Down Timer for ON/OFF
//  Author   : Irfan Indra Kurniawan
//  Site     : www.fanselectronics.com
//-----------------------------------------------------
#include <LiquidCrystal.h>
#include <Timer.h>
#include <EEPROM.h>

//-----------------------------------------------------
#define B_SWT        2        // TOMBOL START / STOP MANUAL  (MERAH)
#define B_MEN        3        // TOMBOL MENU SETTING         (KUNING)
#define B_STR        4        // TOMBOL START TIMER           (HIJAU)
#define B_RST        5        // TOMBOL RESET TIMER          (HIJAU)
#define BUZZ        12
#define SSR         11
#define L_SSR       13
#define L_BLINK     10

#define ON          HIGH
#define OFF         LOW
#define BOUNCE      50        // DELAY TOMBOL
//-----------------------------------------------------
int detik, menit, jam, lcdBlink;
char lcdBuff[16];
String status_SSR, status_ALARM;
bool setting  = false;
bool timer    = false;
bool manual   = false;
bool alarm    = false;
//-----------------------------------------------------
Timer tMain, tAlarm, tCheck;
LiquidCrystal lcd (A0, A1, A2, A3, A4, A5);

//-----------------------------------------------------
void setup() {
  lcd.begin(16, 2);

  pinMode(B_SWT, INPUT_PULLUP);
  pinMode(B_MEN, INPUT_PULLUP);
  pinMode(B_STR, INPUT_PULLUP);
  pinMode(B_RST, INPUT_PULLUP);
  pinMode(BUZZ, OUTPUT);
  pinMode(SSR, OUTPUT);
  pinMode(L_SSR, OUTPUT);
  pinMode(L_BLINK, OUTPUT);

  digitalWrite(BUZZ, OFF);
  digitalWrite(SSR, OFF);
  digitalWrite(L_SSR, OFF);
  digitalWrite(L_BLINK, OFF);

  tMain.every(1000, intDownCount);
  tAlarm.oscillate(BUZZ, 1000, ON);
  tCheck.oscillate(L_BLINK, 500, ON);

  // BACA EEPROM DULU
  jam   = EEPROM.read(3);
  menit = EEPROM.read(4);
  detik = 0;
}

//-----------------------------------------------------

void loop() {
  //--------------------------------------------------- TOMBOL START STOP MODE
  if (digitalRead(B_SWT) == LOW) {
    while (digitalRead(B_SWT) == LOW) {}
    delay(BOUNCE);
    if (manual == false and timer == false) {
      manual = true;
      digitalWrite(SSR, ON);
      digitalWrite(L_SSR, ON);
    } else {
      manual = false;
      digitalWrite(SSR, OFF);
      digitalWrite(L_SSR, OFF);
    }
  }

  //--------------------------------------------------- TOMBOL DOWN / MENU
  if (digitalRead(B_MEN) == LOW) {
    while (digitalRead(B_MEN) == LOW) {}
    delay(BOUNCE);
    if (timer == false) {
      digitalWrite(SSR, ON);
      digitalWrite(L_SSR, ON);
      setting = true;
    } else {
      setting = false;
    }
  }

  //--------------------------------------------------- TOMBOL START TIMER
  if (digitalRead(B_STR) == LOW) {
    while (digitalRead(B_STR) == LOW) {}
    delay(BOUNCE);
    if (timer == false) {
      digitalWrite(SSR, ON);
      digitalWrite(L_SSR, ON);
      timer = true;
      // BACA EEPROM WAKTU TERAKHIR
      jam   = EEPROM.read(3);
      menit = EEPROM.read(4);
      detik = EEPROM.read(5);
    } else {
      timer = false;
      // TULIS EEPROM WAKTU TERAKHIR
      EEPROM.write(3, jam);
      EEPROM.write(4, menit);
      EEPROM.write(5, detik);
    }
  }

  //--------------------------------------------------- TOMBOL RESET TIMER
  if (digitalRead(B_RST) == LOW) {
    while (digitalRead(B_RST) == LOW) {}
    delay(BOUNCE);
    jam   = EEPROM.read(0); // EEPROM 0, Jam Timer
    menit = EEPROM.read(1); // EEPROM 1,
    detik = 0;
    EEPROM.write(3, jam);
    EEPROM.write(4, menit);
    EEPROM.write(5, detik);
  }


  if (digitalRead(SSR) == ON) {
    status_SSR = "ON ";
  } else {
    status_SSR = "OFF";
  }


  if (setting == true) {
    menuSetting();
  }
  if (timer == true) {
    tMain.update();
  }
  if (alarm == true) {
    tAlarm.update();
  }

  sprintf(lcdBuff, "    %s:%s:%s    ", duaDigit(jam), duaDigit(menit), duaDigit(detik));
  lcd.setCursor(0, 0);
  lcd.print(lcdBuff);
  sprintf(lcdBuff, "SWT MEN TIME RST");
  lcd.setCursor(0, 1);
  lcd.print(lcdBuff);

  tCheck.update();
}
//-----------------------------------------------------
void intDownCount() {
  if (jam == 0 and menit == 0 and detik <= 0) {
    timer = false;
    manual = false;
    digitalWrite(SSR, OFF);
    digitalWrite(L_SSR, OFF);
  }

  detik--;
  if (detik < 0) {
    detik = 59;
    menit--;
  }
  if (menit < 0) {
    jam--;
  }

  // Jika Jam = 0, Menit = 0 dan Detik <= 30 sampai 1, maka alarm bunyi
  if ((jam == 0 and menit == 0 and detik <= 30) and (jam == 0 and menit == 0 and detik >= 1)) {
    alarm = true;
  } else {
    alarm = false;
  }

  EEPROM.write(3, jam);
  EEPROM.write(4, menit);
  EEPROM.write(5, detik);
}
//-----------------------------------------------------
void menuSetting() {
  EEPROM.write(2, 0);     //
  int h = EEPROM.read(0); // EEPROM 0, Jam Timer
  int m = EEPROM.read(1); // EEPROM 1, Menit Timer
  int s = 0;
setJAM:
  if (digitalRead(B_SWT) == LOW) {
    while (digitalRead(B_SWT) == LOW) {}
    delay(BOUNCE);
    goto setEND;
  } else if (digitalRead(B_MEN) == LOW) {
    while (digitalRead(B_MEN) == LOW) {}
    delay(BOUNCE);
    goto setMNT;
  } else if (digitalRead(B_STR) == LOW) {
    delay(BOUNCE);
    h--;
    EEPROM.write(0, h);
  } else if (digitalRead(B_RST) == LOW) {
    delay(BOUNCE);
    h++;
    EEPROM.write(0, h);
  }

  if (digitalRead(L_BLINK) == OFF) {
    sprintf(lcdBuff, "    %s:%s:%s    ", duaDigit(h), duaDigit(m), duaDigit(s));
  } else {
    sprintf(lcdBuff, "      :%s:%s    ", duaDigit(m), duaDigit(s));
  }
  lcd.setCursor(0, 0);
  lcd.print(lcdBuff);
  sprintf(lcdBuff, "EXIT SET DOWN UP");
  lcd.setCursor(0, 1);
  lcd.print(lcdBuff);

  tCheck.update();
  goto setJAM;

setMNT:
  if (digitalRead(B_SWT) == LOW) {
    while (digitalRead(B_SWT) == LOW) {}
    delay(BOUNCE);
    goto setEND;
  } else if (digitalRead(B_MEN) == LOW) {
    while (digitalRead(B_MEN) == LOW) {}
    delay(BOUNCE);
    goto setMNT;
  } else if (digitalRead(B_STR) == LOW) {
    delay(BOUNCE);
    m--;
    EEPROM.write(1, m);
  } else if (digitalRead(B_RST) == LOW) {
    delay(BOUNCE);
    m++;
    EEPROM.write(1, m);
  }

  if (digitalRead(L_BLINK) == OFF) {
    sprintf(lcdBuff, "    %s:%s:%s    ", duaDigit(h), duaDigit(m), duaDigit(s));
  } else {
    sprintf(lcdBuff, "    %s:  :%s    ", duaDigit(h), duaDigit(s));
  }
  lcd.setCursor(0, 0);
  lcd.print(lcdBuff);
  sprintf(lcdBuff, "EXIT SET DOWN UP");
  lcd.setCursor(0, 1);
  lcd.print(lcdBuff);

  tCheck.update();
  goto setMNT;

setEND:
  delay(500);
  lcd.clear();
  // BACA EEPROM WAKTU TERBARU
  jam   = EEPROM.read(0);
  menit = EEPROM.read(1);
  detik = EEPROM.read(2);
}

//-----------------------------------------------------
String duaDigit(byte nilai) {
  if (nilai < 10) {
    return "0" + String(nilai);
  } else {
    return String(nilai);
  }
}
//-----------------------------------------------------
