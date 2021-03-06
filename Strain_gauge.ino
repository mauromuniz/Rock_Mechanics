/*
  Strain Gauge Logger V 3.0
  Arduino nano compatible
  4 DAC HX 711
  LCD 16x2 i2C
  Real time clock DS3231
  SD Card Holder
*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Sodaq_DS3231.h>


HX711 B_1(4, 10);   // DOUT, SCK
HX711 B_2(6, 5);    // DOUT, SCK
HX711 B_3(8, 7);    // DOUT, SCK
HX711 B_4(A3, A2);  // DOUT, SCK


LiquidCrystal_I2C lcd(0x3F, 16, 2); //

unsigned long previousMillis = 0;

const long interval = 500;
const float int_sec = 0.5;  //interval / 1000
float tiempo = 0;
unsigned long dato = 1;

const float calib = -497.9; // Calibration based on electronic circuit calculi
const int ganancia = 64;

const int chipSelect = 14; //CS of the SD card

bool grabar = 0;
String archivo = "sin_hora.csv";

void setup() {

  pinMode(1, OUTPUT);     //gren LED
  digitalWrite(1, LOW);   //red LED
  pinMode(2, INPUT);      //button G (Record)
  pinMode(3, INPUT);      //button T (Tare)

  Wire.begin();
  rtc.begin();

  B_1.set_gain(ganancia);
  B_1.set_scale(calib); 
  B_2.set_gain(ganancia);
  B_2.set_scale(calib); 
  B_3.set_gain(ganancia);
  B_3.set_scale(calib); 
  B_4.set_gain(ganancia);
  B_4.set_scale(calib);

  lcd.init();
  lcd.backlight();

  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR TARJETA SD");
    lcd.setCursor(0, 1);
    lcd.print("PULSE RESET");
    while (1);
  }

  lcd.clear();

  attachInterrupt(digitalPinToInterrupt(2), cambio, RISING);
  attachInterrupt(digitalPinToInterrupt(3), cero, RISING);
}


void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (grabar == 1) {
      if (dato == 1) {

      String Ho = "";
      String Mi = "";
      String Se = "";
      String Me = "";
      String Di = "";

      DateTime now = rtc.now();
      if (now.hour() < 10) {Ho = "0";}
      if (now.minute() < 10) {Mi = "0";}
      if (now.second() < 10) {Se = "0";}

  archivo = "BAN_" + Ho + String(now.hour()) + Mi + String(now.minute()) + ".csv";

  SdFile::dateTimeCallback(dateTime); 
  File dataFile = SD.open(archivo, FILE_WRITE);
  if (dataFile) {
    dataFile.println("Medidor de bandas con 4 canales - Version hardware 3.0");
    dataFile.println("Version firmware: V3_punto_y_coma.ino - 20/05/2018");
    dataFile.println("Informacion en mauro.muniz@cedex.es");
    dataFile.print("Fecha inicio: ;");
    dataFile.println(String(now.date()) + "/" + String(now.month()) + "/" + String(now.year()));    
    dataFile.print("Hora inicio: ;");
    dataFile.println(String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));    
    dataFile.println();        
    dataFile.println("Dato;Tiempo(s);Fecha;Hora;Banda1;Banda2;Banda3;Banda4");
    dataFile.close(); //cerramos el archivo
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR CREANDO");
    lcd.setCursor(0,1);
    lcd.print("ARCHIVO");
  }

      }
      digitalWrite(1, HIGH);

      float b1 = B_1.get_units();
      float b2 = B_2.get_units();
      float b3 = B_3.get_units();
      float b4 = B_4.get_units();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(b1, 1);
      lcd.setCursor(8, 0);
      lcd.print(b2, 1);
      lcd.setCursor(0, 1);
      lcd.print(b2), 1);
      lcd.setCursor(8, 1);
      lcd.print(b4, 1);

      File dataFile = SD.open(archivo, FILE_WRITE);
      if (dataFile) {
        DateTime now = rtc.now();
        dataFile.print(dato);
        dataFile.print(";");
        dataFile.print(tiempo);
        dataFile.print(";");
        dataFile.print(now.date(), DEC);
        dataFile.print("/");
        dataFile.print(now.month(), DEC);
        dataFile.print("/");
        dataFile.print(now.year(), DEC);
        dataFile.print(";");
        dataFile.print(now.hour(), DEC);
        dataFile.print(":");
        dataFile.print(now.minute(), DEC);
        dataFile.print(":");
        dataFile.print(now.second(), DEC);
        dataFile.print(";");
        dataFile.print(B_1.get_units(), 2);
        dataFile.print(";");
        dataFile.print(B_2.get_units(), 2);
        dataFile.print(";");
        dataFile.print(B_3.get_units(), 2);
        dataFile.print(";");
        dataFile.println(B_4.get_units(), 2);
        dataFile.close(); //cerramos el archivo

        tiempo = tiempo + int_sec;


      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ERROR LEYENDO");
        lcd.setCursor(0,1);
        lcd.print("ARCHIVO");
      }

      dato = dato + 1;
      digitalWrite(1, LOW);


    } else {
      tiempo = 0;
      dato = 1;

      float b1 = B_1.get_units();
      float b2 = B_2.get_units();
      float b3 = B_3.get_units();
      float b4 = B_4.get_units();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(b1, 1);
      lcd.setCursor(8, 0);
      lcd.print(b2, 1);
      lcd.setCursor(0, 1);
      lcd.print(b2), 1);
      lcd.setCursor(8, 1);
      lcd.print(b4, 1);
    }
  }
}


void cambio() {
  grabar = !grabar;
}

void cero() {
  B_1.tare();
  B_2.tare();
  B_3.tare();
  B_4.tare();
}

void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = rtc.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.date());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}
