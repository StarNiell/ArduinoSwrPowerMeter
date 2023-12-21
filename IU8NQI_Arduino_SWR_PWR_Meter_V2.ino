
#include <Wire.h> // #include <Wire.h> default to Wiring for Uno; SCL = A5, SDA = A4, VCC = 5V+, GND = GND                    
#include <LiquidCrystal_I2C.h>         // The Arduino I2C is hard coded SDA = pin A4) & SCL = pin A5 
LiquidCrystal_I2C lcd(0x27, 16, 2);    //  serial rows X lines LCD Display      //  Address  A2    A1    A0      //    0x27   Hi    Hi    Lo

// Pinouts for NANO I/O  // Analog VDC signals from Directional Coupler
#define Vfwdpin_Anlg 6   // Forward Voltage Vfwd VDC A6
#define Vrevpin_Anlg 7   // Reverse Voltage Vrev VDC A7
///////////////////////////////////////////////////////

#define FULL_SCALE_FORWARD (40.0)
//#define FULL_SCALE_REFLECTED (20.0)
#define MIN_READ (5.0)

// SWR calculating Variables
float VoltRef = 4.48; // Arduino REF voltage measured (some time is not 5 Volt, but lower)
//
float Vfwd;
float Vrev;
float Pfwd;
float Prev;
float Swr;
float PfwdPeak;
float PrevPeak;
float SwrPeak;

const int ARDUINO_AD_MAX = 1023;

// compute scaling constants for power readings
//const float FORWARD_SCALE = (float)FULL_SCALE_FORWARD / (float)ARDUINO_AD_MAX;
//const float REFLECT_SCALE = (float)FULL_SCALE_REFLECTED / (float)ARDUINO_AD_MAX;
const float SIGNAL_TRANSLATE_RATIO = VoltRef / (float)ARDUINO_AD_MAX;

bool IsTransmitting = false;
unsigned long lLastTransittingTime = 0;
unsigned long lLastDisplayTime = 0;
const unsigned long EndTransmittingInterval = 1000;
const unsigned long ShowPeaksInterval = 4000;
const unsigned long ShowDisplayInterval = 500;
bool PeakDataShowing = false;
bool PeakDataShowed = false;
bool FirstTransmition = false;

float rvolt[14] = {0.00, 0.90, 1.16, 1.53, 1.90, 2.00, 2.15, 2.40, 2.80, 3.35, 3.80, 4.23, 4.5,  4.65};
float rwatt[14] = {0.0,  1.0,  2.0,  3.0,  5.0,  6.0,  7.0,  10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0};
float alpha[14];

void setup() {

  Serial.begin(9600);
  SetupLCD();
  SetupAlpha();
}

void SetupLCD()
{
  lcd.init();  // this clears display for new data
  lcd.backlight();   // Turn on the blacklight   
  lcd.clear(); // clear all    
  lcd.setCursor (0, 0);  // go to Column, Row 
  lcd.print("IU8NQI SWR METER");    // displays text         
  lcd.setCursor (0, 1);  // go to Column, Row 
  lcd.print(" " + String((int)FULL_SCALE_FORWARD) + "W Max Power  ");      // displays text  
  delay(2000);
  lcd.clear(); // clear all 
  lcd.setCursor (0, 0);  // go to Column, Row 
  lcd.print(" SWR  FWD  REF "); 
}

void SetupAlpha(){
  alpha[0] = 0.0000;
  for (int i = 1; i < 14; i++) 
  {
    alpha[i] = rwatt[i] / rvolt[i];
  }
}

void loop() {
  elaborateSWR();
}

void elaborateSWR() {
  // read forward and reverse voltage from RF remote directional detector
  Vfwd = analogRead(Vfwdpin_Anlg); 
  Vrev = analogRead(Vrevpin_Anlg);

  if (Vfwd <= MIN_READ) {
      Vfwd = 0.0;
  }

  if (Vfwd <= MIN_READ) {
      Vfwd = 0.0;
  }

  // Translate data in real voltage value
  float VfwdMeasured = Vfwd * SIGNAL_TRANSLATE_RATIO;
  float VrevMeasured = Vrev * SIGNAL_TRANSLATE_RATIO;
 
  Pfwd = getWattByVolt(VfwdMeasured);
  Prev = getWattByVolt(VrevMeasured);

  Serial.println(VfwdMeasured);

  // Get string visible values from numeric data
  String sPfwd = String(Pfwd);
  sPfwd = sPfwd.substring(0, 4);

  String sPrev = String(Prev);
  sPrev = sPrev.substring(0, 4);

  String sSwr = " -- ";
  // Standard Calculation
  float fp = sqrt ( Prev / Pfwd );
  Swr = ( 1 + fp ) / ( 1 - fp );                          
  
  // Voltage Calculation
  //Swr = ( VfwdMeasured + VrevMeasured ) / ( VfwdMeasured - VrevMeasured );                          

  if (Swr < 0)
  {
    Swr = Swr * -1;
  }
  Swr = constrain(Swr, 1, 11); 

  // Peak Calculating
  if (Pfwd > PfwdPeak)
  {
    PfwdPeak = Pfwd;
  }

  if (Prev > PrevPeak)
  {
    PrevPeak = Prev;
  }

  if (!isnan(Swr) && Swr > SwrPeak)
  {
    SwrPeak = Swr;
  }

  IsTransmitting = !isnan(Swr);

  if (IsTransmitting)
  {
    //Serial.println(" > In Trasmitions");
    FirstTransmition = true;
    lLastTransittingTime = millis();
    PeakDataShowed = false;
    sSwr = String(Swr);
    sSwr = sSwr.substring(0, 4);
    if (Swr > 10)
    {
      sSwr = "+10 ";
    }

    //Display visible values
    DisplayInfo(sSwr, sPfwd, sPrev, false, true);
  }
  else
  {
    if (FirstTransmition)
    {
      unsigned long TimeToWait = EndTransmittingInterval;
      if (PeakDataShowing)
      {
        TimeToWait = ShowPeaksInterval;
      }
      //Serial.println(" < No Trasmitions");
      if (!PeakDataShowed &&  Elapsed(lLastTransittingTime, TimeToWait))
      {
        lLastTransittingTime = millis();
        if (!PeakDataShowing)
        {
          // Get string visible values from numeric data of Peak Values
          String sPfwdPeak = String(PfwdPeak);
          sPfwdPeak = sPfwdPeak.substring(0, 4);

          String sPrevPeak = String(PrevPeak);
          sPrevPeak = sPrevPeak.substring(0, 4);

          String sSwrPeak = " -- ";
          if (!isnan(SwrPeak))
          {
            sSwrPeak = String(SwrPeak);
            sSwrPeak = sSwrPeak.substring(0, 4);
          }

          //Display visible values
          lLastDisplayTime = 0;
          DisplayInfo(sSwrPeak, sPfwdPeak, sPrevPeak, true, false);
          PeakDataShowing = true;
        }
        else 
        {
          resetPeakData();
          lLastDisplayTime = 0;
          DisplayInfo(" -- ", "0.00", "0.00", false, false);
          PeakDataShowing = false;
          PeakDataShowed = true;
          lLastDisplayTime = 0;
          Serial.println(" - Reset Peak data: ");
          
        }
      }
    }

    if ((!FirstTransmition || !PeakDataShowed) && !PeakDataShowing)
    {
      DisplayInfo(sSwr, sPfwd, sPrev, false, false);
    }
  }
}

float getWattByVolt(float volt)
{
  float ret = 0.0;
  //float alphaTop = 0.00;
  float alphaPrev = 0.00;
  float deltaVbase = 0.00;
  float deltaAdiff = 0.00;
  float deltaVXdiff = 0.00;
  float deltaAXdiff = 0.00;
  float alphaX = 0.00;
  for (int i = 0; i < 14; i++) 
  {
    if (volt <= rvolt[i])
    {
      //Serial.print(i);
      //Serial.print(" = ");
      //Serial.println(volt);
      alphaPrev = alpha[i-1];
      //alphaTop = alpha[i];
      deltaVbase = rvolt[i] - rvolt[i-1];
      deltaAdiff = alpha[i] - alpha[i-1];
      deltaVXdiff = volt - rvolt[i-1];
      deltaAXdiff = deltaVXdiff * deltaAdiff / deltaVbase;
      alphaX = alphaPrev + deltaAXdiff;
      ret = volt * alphaX;
      int a = ret * 100;
      ret = (float)a / 100;
      break;
    }
  }

  return ret;
}

void resetPeakData()
{
  PfwdPeak = 0.0;
  PrevPeak = 0.0;
  SwrPeak = 0.0;
}

void DisplayInfo(String sSwr, String sPfwd, String sPrev, bool IsPeak, bool IsTrasm)
{
  if (Elapsed(lLastDisplayTime, ShowDisplayInterval))
  {
    if (IsPeak)
    {
      lcd.setCursor (15, 0);   
      lcd.print("#");
      lcd.setCursor (15, 1);   
      lcd.print("#");
    }
    else if (IsTrasm)
    {
      lcd.setCursor (15, 0);   
      lcd.print(char(255));
      lcd.setCursor (15, 1);   
      lcd.print(char(255));
    }
    else
    {
      lcd.setCursor (15, 0);   
      lcd.print(" ");
      lcd.setCursor (15, 1);   
      lcd.print(" ");
    }
    
    lcd.setCursor (0, 1); 
    lcd.print(sSwr);
    lcd.print(" "); 
    lcd.print(sPfwd);
    lcd.print(" "); 
    lcd.print(sPrev);
    lcd.print(" "); 
    //delay(500);
    lLastDisplayTime = millis();
  }
}

bool Elapsed(unsigned long lastTime, unsigned long howManyTime)
{
  return (millis() - lastTime) >= howManyTime;
} 