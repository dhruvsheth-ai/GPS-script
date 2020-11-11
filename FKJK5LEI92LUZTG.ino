/*
 GPS distance measuring
    - GPS: Holux M-1000
    - Arduino: JeonLab mini v1.3
    - LCD: Nokia 5110
 Programmed by: Jinseok Jeon (JeonLab.wordpress.com)
 Date: Sep 2013
 Revised: Oct 28, 2013
 */

#include <SoftwareSerial.h>
SoftwareSerial gps(10, 0); // RX, TX

const int TimeZone = -5; //EST
int DSTbegin[] = { //DST 2013 - 2025 in Canada and US
  310, 309, 308, 313, 312, 311, 310, 308, 314, 313, 312, 310, 309
};
int DSTend[] = { //DST 2013 - 2025 in Canada and US
  1103, 1102, 1101, 1106, 1105, 1104, 1103, 1101, 1107, 1106, 1105, 1103, 1102
};
int DaysAMonth[] = { //number of days a month
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
int gpsYear;
int gpsMonth;
int gpsDay;
int gpsHour;
byte gpsMin;
byte gpsSec;
//float distance;
float gpsLat0;
float gpsLong0;
float gpsLat;
float gpsLong;
float gpsSpeed; //km/h
float gpsBearing; //deg
boolean LEDstate = false;
boolean SpeedWatch50;
boolean SpeedWatch70;
boolean SpeedWatch120;

#include <LCD5110_Basic.h>

LCD5110 LCD(6, 5, 4, 2, 3); //SCLK, MOSI/DIN, DC, RST, CS/CE
extern uint8_t SmallFont[];      // 6x8 pixels
extern uint8_t MediumNumbers[];  // 12x16 pixels
//extern uint8_t BigNumbers[];     // 14x24 pixels

void setup()
{
  gps.begin(38400);
  pinMode(8, OUTPUT); //LED
  digitalWrite(8, 1); //LED off
  pinMode(7, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  LCD.InitLCD();
  LCD.setContrast(70); //0-127, you need to find proper number
  LCD.setFont(SmallFont);
  LCD.print("JeonLab", RIGHT, 16);
}

void loop()
{
  int a1, a2, b1, b2;
  if (gps.available() > 1)
  {
    if (char(gps.read()) == 'R' && char(gps.read()) == 'M' && char(gps.read()) == 'C')
    {
      gpsTime(gps.parseInt());
      gps.parseFloat(); //discard unnecessary part
      a1 = gps.parseInt();
      a2 = gps.parseInt();
      b1 = gps.parseInt();
      b2 = gps.parseInt();
      gpsLatLong(a1, a2, b1, b2);
//      gpsLatLong(gps.parseInt(), gps.parseInt(), gps.parseInt(), gps.parseInt());
      gpsSpeed = gps.parseFloat() * 1.852; //knot to km/h
      gpsBearing = gps.parseFloat();
      gpsDate(gps.parseInt());
      if (gpsYear % 4 == 0) DaysAMonth[1] = 29; //leap year check

      //Time zone adjustment
      gpsHour += TimeZone;
      //DST adjustment
      if (gpsMonth * 100 + gpsDay >= DSTbegin[gpsYear - 13] &&
          gpsMonth * 100 + gpsDay < DSTend[gpsYear - 13]) gpsHour += 1;
      if (gpsHour < 0)
      {
        gpsHour += 24;
        gpsDay -= 1;
        if (gpsDay < 1)
        {
          if (gpsMonth == 1)
          {
            gpsMonth = 12;
            gpsYear -= 1;
          }
          else
          {
            gpsMonth -= 1;
          }
          gpsDay = DaysAMonth[gpsMonth - 1];
        }
      }
      if (gpsHour >= 24)
      {
        gpsHour -= 24;
        gpsDay += 1;
        if (gpsDay > DaysAMonth[gpsMonth - 1])
        {
          gpsDay = 1;
          gpsMonth += 1;
          if (gpsMonth > 12) gpsYear += 1;
        }
      }
      LCD.setFont(MediumNumbers);
      LCD.clrRow(0);//8 pixel high row to clear (0-5)
      LCD.clrRow(1);
      LCD.printNumF(gpsSpeed, 0, LEFT, 0); //km/h
      if (gpsSpeed > 2)
      {
        LCD.printNumF(gpsBearing, 0, RIGHT, 0); //bearing in degree
      }

      LCD.setFont(SmallFont);
      LCD.printNumI(gpsMonth, 0, 24, 2, '0');
      LCD.print("-", 12, 24);
      LCD.printNumI(gpsDay, 18, 24, 2, '0');
      LCD.print("-", 30, 24);
      LCD.printNumI(gpsYear, 36, 24);

      LCD.printNumI(gpsHour, 54, 24, 2, '0');
      LCD.print(":", 66, 24);
      LCD.printNumI(gpsMin, 72, 24, 2, '0');

      if (gpsLat0 != 0.0)
      {
        float distLat = abs(gpsLat0 - gpsLat) * 111194.9;
        float distLong = 111194.9 * abs(gpsLong0 - gpsLong) * cos(radians((gpsLat0 + gpsLat) / 2));
        float distance = sqrt(pow(distLat, 2) + pow(distLong, 2));

        LCD.clrRow(4);//8 pixel high row to clear (0-5)
        LCD.clrRow(5);
        LCD.printNumF(distance, 0, LEFT, 32);
        LCD.print("meter", RIGHT, 32);
        LCD.printNumF(distance / 0.9144, 0, LEFT, 40);
        LCD.print("yard", RIGHT, 40);
      }
      if (gpsSpeed <= 50) SpeedWatch50 = 0;
      if (gpsSpeed <= 70) SpeedWatch70 = 0;
      if (gpsSpeed <= 120) SpeedWatch120 = 0;

      if (gpsSpeed > 50 && SpeedWatch50 == 0)
      {
        SpeedWatch50 = 1;
        tone(11, 4978, 100);
        delay(100);
      }
      if (gpsSpeed > 70 && SpeedWatch70 == 0)
      {
        SpeedWatch70 = 1;
        for (int i = 1; i <= 2; i++)
        {
          tone(11, 4978, 100);
          delay(500);
        }
      }
      if (gpsSpeed > 120 && SpeedWatch120 == 0)
      {
        SpeedWatch120 = 1;
        for (int i = 1; i <= 3; i++)
        {
          tone(11, 4978, 100);
          delay(500);
        }
      }
    }
  }
  if (digitalRead(12) == LOW) //marking current position
  {
    tone(11, 3140, 100);
    if (gpsLat0 == 0.0)
    {
      gpsLat0 = gpsLat;
      gpsLong0 = gpsLong;
    }
    else
    {
      gpsLat0 = 0.0;
      gpsLong0 = 0.0;
      LCD.clrRow(4);
      LCD.clrRow(5);
    }
    delay(700);
  }
  if (digitalRead(7) == LOW) //LED backlight toggle
  {
    tone(11, 2810, 100);
    digitalWrite(8, LEDstate); //LED on
    LEDstate = !LEDstate;
    delay(700);
  }
  if (digitalRead(9) == LOW) //for future functional button
  {
    tone(11, 3910, 100);

  }
}

void gpsTime(long UTC)
{
  gpsHour = int(UTC / 10000);
  gpsMin = int(UTC % 10000 / 100);
  gpsSec = UTC % 100;
}

void gpsLatLong(int lat1, int lat2, int long1, int long2)
{
  gpsLat = int(lat1 / 100) + (lat1 % 100) / 60.0 + float(lat2) / 10000.0 / 60.0;
  gpsLong = int(long1 / 100) + (long1 % 100) / 60.0 + float(long2) / 10000.0 / 60.0;
}

void gpsDate(long dateRead)
{
  gpsDay = int(dateRead / 10000);
  gpsMonth = int(dateRead % 10000 / 100);
  gpsYear = dateRead % 100; //last 2 digits, e.g. 2013-> 13
}
