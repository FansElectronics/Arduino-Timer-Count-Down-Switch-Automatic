//-----------------------------------------------------
//  Project  : Down Timer for ON/OFF
//  Author   : Irfan Indra Kurniawan
//  Site     : www.fanselectronics.com

//  Catatan  :
//
//-----------------------------------------------------
#include <LiquidCrystal.h>
#include <Timer.h>
#include <EEPROM.h>

//-----------------------------------------------------
#define B_SWT        2        // TOMBOL START / STOP MANUAL  (MERAH)
#define B_MEN        3        // TOMBOL MENU SETTING         (KUNING)
#define B_STR        4        // TOMBOL START TIMER          (HIJAU)
#define B_RST        5        // TOMBOL RESET TIMER          (HIJAU)
#define BUZZ        12
#define SSR         11
#define L_SSR       10
#define L_BLINK     13

#define ON          HIGH
#define OFF         LOW
#define BOUNCE      150       // DELAY TOMBOL
//-----------------------------------------------------
int detik, menit, jam, lcdBlink;
char lcdBuff[16];
String status_SSR, status_ALARM, lcdString;
bool setting  = false;
bool timer    = false;
bool manual   = false;
bool alarm    = false;
//-----------------------------------------------------
Timer tMain, tAlarm, tCheck;
LiquidCrystal lcd (A0, A1, A2, A3, A4, A5);

//-----------------------------------------------------
void setup() {
  Serial.begin(9600);
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
  tAlarm.oscillate(BUZZ, 1000, OFF);
  tCheck.oscillate(L_BLINK, 500, ON);

  // BACA EEPROM DULU
  jam   = EEPROM.read(0);
  menit = EEPROM.read(1);
  detik = 0;
  EEPROM.write(3, jam);
  EEPROM.write(4, menit);
  EEPROM.write(5, detik);
  EEPROM.write(7, OFF);   // EEPROM SSR OFF SAAT RESTART
}

//-----------------------------------------------------

void loop() {
  //--------------------------------------------------- TOMBOL START STOP MODE
  if (digitalRead(B_SWT) == LOW) {
    while (digitalRead(B_SWT) == LOW) {}
    delay(BOUNCE);
    alarm = false;
    if (manual == false and timer == false) {
      manual = true;
      EEPROM.write(7, ON);
    } else if (manual == true and timer == false) {
      manual = false;
      EEPROM.write(7, OFF);
    }
  }

  //--------------------------------------------------- TOMBOL DOWN / MENU
  if (digitalRead(B_MEN) == LOW) {
    while (digitalRead(B_MEN) == LOW) {}
    delay(BOUNCE);
    alarm = false;
    if (timer == false) {
      setting = true;
    } else {
      setting = false;
    }
  }

  //--------------------------------------------------- TOMBOL START TIMER
  if (digitalRead(B_STR) == LOW) {
    while (digitalRead(B_STR) == LOW) {}
    delay(BOUNCE);
    alarm = false;
    if (timer == false) {
      EEPROM.write(7, ON);
      timer = true;
      // BACA EEPROM WAKTU TERAKHIR
      // jam   = EEPROM.read(3);
      // menit = EEPROM.read(4);
      // detik = EEPROM.read(5);
    } else {
      timer = false;
      // TULIS EEPROM WAKTU TERAKHIR
      //EEPROM.write(3, jam);
      //EEPROM.write(4, menit);
      //EEPROM.write(5, detik);
      //EEPROM.write(7, OFF);
    }
  }

  //--------------------------------------------------- TOMBOL RESET TIMER
  if (digitalRead(B_RST) == LOW) {
    while (digitalRead(B_RST) == LOW) {}
    delay(BOUNCE);
    timer = false;
    alarm = false;
    jam   = EEPROM.read(0); // EEPROM 0, Jam Timer
    menit = EEPROM.read(1); // EEPROM 1,
    detik = 0;
    // EEPROM.write(3, jam);
    //EEPROM.write(4, menit);
    //EEPROM.write(5, detik);
    EEPROM.write(7, OFF);
  }

  if (jam < 0 or jam >= 100) {
    jam = 0; menit = 0; detik = 0;
    timer = false;
    manual = false;
    alarm = false;
    EEPROM.write(7, OFF);
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
    if (detik < 0) {
      detik = 59;
      menit--;
    }
    if (menit < 0) {
      jam--;
      menit = 59;
    }

    if ((jam == 0 and menit == 0 and detik <= 30) and (jam == 0 and menit == 0 and detik >= 1)) {
      alarm = true;
    } else {
      alarm = false;
    }
   // EEPROM.write(3, jam);
   // EEPROM.write(4, menit);
   // EEPROM.write(5, detik);
  }
  if (alarm == true) {
    tAlarm.update();
  }

  // TAMPILAN LCD
  lcdString = "[   " + duaDigit(jam) + ":" + duaDigit(menit) + ":" + duaDigit(detik) + "   ]";
  lcd.setCursor(0, 0);
  lcd.print(lcdString);
  lcd.setCursor(0, 1);
  lcd.print("SWT MEN TIME RST");

  // BACA KONDISI SSR VIA EERPROM
  digitalWrite(SSR, EEPROM.read(7));
  digitalWrite(L_SSR, EEPROM.read(7));

  String S = String(jam) + ":" + String(menit) + ":" + String(detik);
  Serial.println(S);

  tCheck.update();
}
//-----------------------------------------------------
void intDownCount() {
  detik--;
  if (jam > 99) {
    timer = false;
    manual = false;
    alarm = false;
    digitalWrite(SSR, OFF);
    digitalWrite(L_SSR, OFF);
  }
}
//-----------------------------------------------------
void menuSetting() {
  EEPROM.write(2, 0);     //
  int h = EEPROM.read(0); // EEPROM 0, Jam Timer
  int m = EEPROM.read(1); // EEPROM 1, Menit Timer
  int s = 0;
  lcd.clear();
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
    if (h < 0) h = 99;
    EEPROM.write(0, h);
  } else if (digitalRead(B_RST) == LOW) {
    delay(BOUNCE);
    h++;
    if (h >= 99) h = 0;
    EEPROM.write(0, h);
  }

  if (digitalRead(L_BLINK) == OFF) {
    lcdString = "[   " + duaDigit(h) + ":" + duaDigit(m) + ":" + duaDigit(s) + "   ]";
  } else {
    lcdString = "[     :" + duaDigit(m) + ":" + duaDigit(s) + "   ]";
  }
  lcd.setCursor(0, 0);
  lcd.print(lcdString);
  lcd.setCursor(0, 1);
  lcd.print("EXIT SET DOWN UP");

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
    goto setJAM;
  } else if (digitalRead(B_STR) == LOW) {
    delay(BOUNCE);
    m--;
    if (m < 0) m = 59;
    EEPROM.write(1, m);
  } else if (digitalRead(B_RST) == LOW) {
    delay(BOUNCE);
    m++;
    if (m >= 60) m = 0;
    EEPROM.write(1, m);
  }

  if (digitalRead(L_BLINK) == OFF) {
    lcdString = "[   " + duaDigit(h) + ":" + duaDigit(m) + ":" + duaDigit(s) + "   ]";
  } else {
    lcdString = "[   " + duaDigit(h) + ":  :" + duaDigit(s) + "   ]";
  }
  lcd.setCursor(0, 0);
  lcd.print(lcdString);
  lcd.setCursor(0, 1);
  lcd.print("EXIT SET DOWN UP");

  tCheck.update();
  goto setMNT;

setEND:
  delay(500);
  lcd.clear();
  // BACA EEPROM WAKTU TERBARU
  jam   = EEPROM.read(0); // EEPROM 0, Jam Timer
  menit = EEPROM.read(1); // EEPROM 1,
  detik = EEPROM.read(2);
  EEPROM.write(3, jam);
  EEPROM.write(4, menit);
  EEPROM.write(5, detik);
  setting = false;

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
