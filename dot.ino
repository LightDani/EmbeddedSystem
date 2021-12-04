// Library Button
#include <Button.h>
// Library RTC
#include "RTClib.h"
#include <Wire.h>
// Library Display
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Additional library for custom font
#include "Font_Data.h"

// Define variable for display
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    3

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Declare variable for RTC
RTC_DS3231 rtc;
DateTime t;

// Define variable for sensor
#define SENSOR_PIN  0
#define LDR_PIN 1
// Declare variable for button
Button buttonA = Button(7, BUTTON_PULLUP);
Button buttonB = Button(8, BUTTON_PULLUP);
Button buttonC = Button(9, BUTTON_PULLUP);

// Global variable
char disp[10];
char temp[10];
int jam, menit, detik, tahun, bulan, hari, timer, mode = 0, bulat, desimal, intensitas;
float suhu;

void setup() {
  // serial
  Serial.begin(9600);
  // display
  P.begin();
  P.setFont(fontMini);
  P.setIntensity(0);
  P.displayClear();
  P.displayScroll("Digital Clock by: Dwi Cahya Nur Faizi    0721 18 4000 0010", PA_CENTER, PA_SCROLL_LEFT, 150);
  P.setTextAlignment(PA_CENTER);
  //rtc
  Wire.begin();
  rtc.begin();
  // button
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
}

void loop() {
  if(P.displayAnimate())
  {
    // mode run
    if (mode == 0) {
      // update waktu
      updateWaktu();
      // tampil tanggal pada detik 10-12
      if (detik / 10 == 1 && detik % 10 == 0) {
        while (detik % 10 < 3) {
          updateWaktu();
          tampilTanggal();
        }
      }
      // tampil suhu pada detik 13-15
      else if (detik / 10 == 1 && detik % 10 == 3) {
        while (detik % 10 < 6) {
          tampilSuhu();
          updateWaktu();
        }
      }
      // tampil waktu pada detik yang lain
      else {
        tampilJam();
      }
    }
    /*
      mode ubah:
      1. jam
      2. menit
      3. detik
      4. tanggal
      5. bulan
      6. tahun
    */
    else if (mode != 0 && mode < 4) {
      tampilJam();
      updateWaktu();
    } else if (mode > 3) {
      tampilTanggal();
      updateWaktu();
    }
  }
}

void updateWaktu() {
  t = rtc.now();
  jam = t.hour();
  menit = t.minute();
  detik = t.second();
  tahun = t.year();
  bulan = t.month();
  hari = t.day();
}

void tampilJam() {
  checkButton();
  setIntensitas();
  sprintf(temp, "%d", jam / 10);
  strcat(disp, temp);
  sprintf(temp, "%d", jam % 10);
  strcat(disp, temp);

  if (mode == 1){strcat(disp, "'");} // jam edit mode
  
  strcat(disp, ":");

  sprintf(temp, "%d", menit / 10);
  strcat(disp, temp);
  sprintf(temp, "%d", menit % 10);
  strcat(disp, temp);

  if (mode == 2){strcat(disp, "'");} // menit edit mode
  
  strcat(disp, ":");

  sprintf(temp, "%d", detik / 10);
  strcat(disp, temp);
  sprintf(temp, "%d", detik % 10);
  strcat(disp, temp);

  if (mode == 3){strcat(disp, "'");} // detik edit mode
  
  P.print(disp);
  memset(disp, 0, 10);
  memset(temp, 0, 10);
}

void tampilTanggal() {
  checkButton();
  setIntensitas();
  sprintf(temp, "%d", hari / 10);
  strcat(disp, temp);
  sprintf(temp, "%d", hari % 10);
  strcat(disp, temp);

  if (mode == 4){strcat(disp, "'");} // hari edit mode
  
  strcat(disp, "-");

  sprintf(temp, "%d", bulan / 10);
  strcat(disp, temp);
  sprintf(temp, "%d", bulan % 10);
  strcat(disp, temp);

  if (mode == 5){strcat(disp, "'");} // bulan edit mode
  
  strcat(disp, "-");

  sprintf(temp, "%d", tahun % 100);
  strcat(disp, temp);

  if (mode == 6){strcat(disp, "'");} // tahun edit mode
  
  P.print(disp);
  memset(disp, 0, 10);
  memset(temp, 0, 10);
}

void tampilSuhu(){
  checkButton();
  setIntensitas();
  suhu = analogRead(SENSOR_PIN) * (5.0 / 1024.0) * 100;
  bulat = suhu;
  desimal = suhu*100 - bulat*100;
  if (suhu < 0){
    sprintf(disp, "-%d.%02d°C", bulat, desimal);
  }else{
    sprintf(disp, "%d.%02d°C", bulat, desimal);
  }
  P.print(disp);
  delay(1000);
  memset(disp, 0, 10);
}

void checkButton() {
  // button A
  if (buttonA.uniquePress()) {
    // ganti mode
    mode ++;
    if (mode > 6) {
      mode = 0;
    }
  }
  // button B
  if (buttonB.uniquePress()) {
    // mode run tampil tanggal 5 detik
    if (mode == 0) {
      timer = (detik + 5) % 10 ;
      while (detik % 10 != timer) {
        tampilTanggal();
        updateWaktu();
      }
    }
    // mode ubah
    else {
      if (mode == 1) { // ubah jam
        DateTime future (t + TimeSpan(3600));
        jam = future.hour();
        rtc.adjust(DateTime(tahun, bulan, hari, future.hour(), menit, detik));
      }
      if (mode == 2) { // ubah menit
        DateTime future (t + TimeSpan(60));
        menit = future.minute();
        rtc.adjust(DateTime(tahun, bulan, hari, jam, future.minute(), detik));
      }
      if (mode == 3) { // ubah detik
        DateTime future (t + TimeSpan(1));
        detik = future.second();
        rtc.adjust(DateTime(tahun, bulan, hari, jam, menit, future.second()));
      }
      if (mode == 4) { // ubah hari
        DateTime future (t + TimeSpan(86400));
        hari = future.day();
        rtc.adjust(DateTime(tahun, bulan, future.day(), jam, menit, detik));
      }
      if (mode == 5) { // ubah bulan
        if (bulan < 8) { // januari - juli
          if (bulan % 2 == 1) { // bulan ganjil
            DateTime future (t + TimeSpan(2678400)); // 31 hari
            if (future.month() == 3){
              if (checkKabisat()){
                rtc.adjust(DateTime(tahun, 2, 29, jam, menit, detik));
              }else{
                rtc.adjust(DateTime(tahun, 2, 28, jam, menit, detik));
              }
            }else{
              rtc.adjust(DateTime(tahun, future.month(), future.day(), jam, menit, detik));
            }
          } else { // bulan genap
            if (bulan == 2) { // bulan feb
              if (checkKabisat()) {
                DateTime future (t + TimeSpan(2505600)); // 29 hari
                rtc.adjust(DateTime(tahun, future.month(), future.day(), jam, menit, detik));
              } else {
                DateTime future (t + TimeSpan(2419200)); // 28 hari
                rtc.adjust(DateTime(tahun, future.month(), future.day(), jam, menit, detik));
              }
            } else { // selain bulan februari
              DateTime future (t + TimeSpan(2592000)); // 30 hari
              rtc.adjust(DateTime(tahun, future.month(), future.day(), jam, menit, detik));
            }
          }
        } else { // agustus - desember
          if (bulan % 2 == 0) { // bulan genap
            DateTime future (t + TimeSpan(2678400)); // 31 hari
            rtc.adjust(DateTime(tahun, future.month(), future.day(), jam, menit, detik));
          } else { // bulan ganjil
            DateTime future (t + TimeSpan(2592000)); // 30 hari
            rtc.adjust(DateTime(tahun, future.month(), future.day(), jam, menit, detik));
          }
        }
      }
      if (mode == 6) { // ubah tahun
        tahun++;
        if(bulan == 2 && hari > 28){
          if(checkKabisat()){
            hari = 29;
          }else{
            hari = 28;
          }
        }
        rtc.adjust(DateTime(tahun, bulan, hari, jam, menit, detik));
      }
    }
  }
  // button C
  if (buttonC.uniquePress()) {
    // mode run tampil suhu 5 detik
    if (mode == 0) {
      timer = (detik + 5) % 10 ;
      while (detik % 10 != timer) {
        tampilSuhu();
        updateWaktu();
      }
    }
    // mode ubah
    else {
      if (mode == 1) { // ubah jam
        DateTime future (t - TimeSpan(3600));
        jam = future.hour();
        rtc.adjust(DateTime(tahun, bulan, hari, future.hour(), menit, detik));
      }
      if (mode == 2) { // ubah menit
        DateTime future (t - TimeSpan(60));
        menit = future.minute();
        rtc.adjust(DateTime(tahun, bulan, hari, jam, future.minute(), detik));
      }
      if (mode == 3) { // ubah detik
        DateTime future (t - TimeSpan(1));
        detik = future.second();
        rtc.adjust(DateTime(tahun, bulan, hari, jam, menit, future.second()));
      }
      if (mode == 4) { // ubah hari
        if (hari == 1){
          if (bulan == 12){
            bulan = 1;
          }else{
            bulan++;
          }
          rtc.adjust(DateTime(tahun, bulan, hari, jam, menit, detik));
          updateWaktu();
        }
        DateTime future (t - TimeSpan(86400));
        hari = future.day();
        rtc.adjust(DateTime(tahun, future.month(), future.day(), jam, menit, detik));
      }
      if (mode == 5) { // ubah bulan
        DateTime future (t - TimeSpan(2592000));
        if (future.month() == 1){
          if(checkKabisat()){
            rtc.adjust(DateTime(tahun, 2, 29, jam, menit, detik));
          }else{
            rtc.adjust(DateTime(tahun, 2, 28, jam, menit, detik));
          }
        }
        if(future.month() == 2 && hari > 28){
          if(checkKabisat()){
            hari = 29;
          }else{
            hari = 28;
          }
        }
        rtc.adjust(DateTime(tahun, future.month(), hari, jam, menit, detik));
        }
      if (mode == 6) { // ubah tahun
        tahun--;
        if(bulan == 2 && hari > 28){
          if(checkKabisat()){
            hari = 29;
          }else{
            hari = 28;
          }
        }
        rtc.adjust(DateTime(tahun, bulan, hari, jam, menit, detik));
      }
    }
  }
}

bool checkKabisat() {
  if (tahun % 100 != 0 && tahun % 4 == 0) {
    return true;
  }
  else if (tahun % 100 == 0 && tahun % 400 == 0) {
    return true;
  }
  else if (tahun % 100 == 0 && tahun % 400 != 0) {
    return false;
  }
  else {
    return false;
  }
}

void setIntensitas() {
  intensitas = analogRead(LDR_PIN);
  P.setIntensity((intensitas/100)*2);
}