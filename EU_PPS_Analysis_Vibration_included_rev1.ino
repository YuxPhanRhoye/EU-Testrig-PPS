#include "Arduino.h"
#include "MCP45HVX1.h"
#include "MPU9250.h"

MCP45HVX1 digiPot(0x3E);
MPU9250 mpu;

#define FORWARD true
#define REVERSE false
#define MAX_WIPER_VALUE 127   //  Maximum wiper valued
#define MIN_WIPER_VALUE 50

int inPin = 8;
int delaytime = 0;

float averageVoltage = 0.00;
boolean up = false;

//measure frequency
#define pulse_ip 7
float freq30 = 0.00;
float freq24 = 0.00;
float freq12 = 0.00;
float freq30a = 0.00; 
float freq30b = 0.00;
int i30 = 0;

float freq24a = 0.00; 
float freq24b = 0.00;
int i24 = 0;

float freq12a = 0.00;
float freq12b = 0.00;
int i12 = 0;
//

String MotorSerial;
String PCBSerial;

void setup ()
{ 
  Serial.begin(9600);
  MotorSerial = WaitForInput("Please enter motor serial number.");
  PCBSerial = WaitForInput("Please enter PCB serial number.");
  Serial.println("Switch on power supply");
  delay(4000);
  digiPot.begin();
    
  Serial.print("Starting 28s Delay");
  digiPot.writeWiper(MAX_WIPER_VALUE);
  //delay(28000);
  while (delaytime != 28)
  {
    delaytime++;
    Serial.print(".");
    delay(1000);  
  }
  
  if (delaytime == 28)
  {
    Serial.println("Done");
    delaytime++;  
  }

    Serial.println("");
    Serial.println("Motor Analysis Report");
    Serial.print("Motor Serial: ");
    Serial.println(MotorSerial);
    Serial.print("PCB Serial: ");
    Serial.println(PCBSerial);
    Serial.println("");

    pinMode(inPin, INPUT);
    int inPin = 8;

    //measure frequency
    pinMode(pulse_ip,INPUT);
    //

    if (!mpu.setup(0x68)) {  // change to your own address
        while (1) {
            Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
            delay(5000);
        }
    }
}

void loop ()
{
  int   nWiper = digiPot.readWiper();
  static bool  bDirection = FORWARD;
  // Get current wiper position
  // Determine the direction.
  if( MAX_WIPER_VALUE == nWiper)
  {
    if (up == true)
    {     
      ResultFeedback();
      delay(1000);
      VibrationsTest();
      delay(5000);
      exit(0);
    }
    bDirection = REVERSE;
  }
  else if( MIN_WIPER_VALUE == nWiper)
  {
    bDirection = FORWARD;
    up = true;
  }
  // Move the digipot wiper
  if( FORWARD == bDirection)
  {
    digiPot.incrementWiper();
    nWiper = digiPot.readWiper();    // The direction is forward
  }
  else
  {
    digiPot.decrementWiper();
    nWiper = digiPot.readWiper();    // The direction is backward
  }
  MeasureRPM(nWiper);
  MeasureFrequency(nWiper);
  delay(300);
}

String WaitForInput(String MSerial) {
  Serial.println(MSerial);
 
  while(!Serial.available()) {
    // wait for input
  }
 
  return Serial.readStringUntil(10);
}

void ResultFeedback(){
  Serial.println("");
  Serial.println("For CSV use:");
  Serial.print(MotorSerial);
  Serial.print(",");
  Serial.print(PCBSerial);
  Serial.print(",");
  Serial.print(freq30);
  Serial.print(",");
  Serial.print(freq24);
  Serial.print(",");
  Serial.print(freq12);
  Serial.print(",");
  Serial.println(averageVoltage);
  Serial.println("");
  Serial.println("Vibration Results");
}

void VibrationsTest(){
  float value = 0;
  float maxvalue = -10;
  int i = 0;
  float minvalue = 10;
  float diff = 0;
  
  while (i >= 0 && i<= 1200)
  {
    if (mpu.update()) 
    {
      static uint32_t prev_ms = millis();
      if (millis() > prev_ms + 25) 
      {
        prev_ms = millis();
      }
    }
    i++;
    if (i >= 600 && i<= 1200)
    {
      value = mpu.getRoll();
      if (value >= maxvalue)
      {
        maxvalue = value;
      }
      if (value <= minvalue)
      {
        minvalue = value;
      }
    }
  }
  if (i == 1201)
  {
    Serial.print("Max: ");
    Serial.println(maxvalue);
    Serial.print("Min: ");
    Serial.println(minvalue);
    Serial.print("The difference is: ");
    diff = maxvalue - minvalue;
    Serial.println(diff);
    Serial.println("");
    Serial.println("Switch off power supply");
  }  
}

void MeasureRPM(int Wiper_RPM){
//measure Voltage
float input_voltage = 0.0;
float temp = 0.0;
float r1 = 10000.00;
float r2 = 2020.0;

float fWiper = 0.00;
float convertWiper = 0.00;
int analog_value = 0;

boolean RPMsensor;

  RPMsensor = digitalRead(inPin);
  if (RPMsensor == 0)
  {
    fWiper = Wiper_RPM;
    convertWiper = 0.3+(30*(fWiper)/MAX_WIPER_VALUE+0.2)-0.001*(fWiper*2);
    analog_value = analogRead(A1);     
    temp = (analog_value * 5.07) / 1023.0;   
    input_voltage = temp / (r2/(r1+r2));        
    if (input_voltage < 0.1)   
    {     
        input_voltage=0.0;    
    }
    averageVoltage = (input_voltage+convertWiper)/2;
    Serial.print("Minimum RPM......................................... ");
    Serial.print(averageVoltage);
    Serial.println(" V");
  }  
}

void MeasureFrequency(int Wiper){
int ontime;
int offtime;
int dutycycle;

float period = 0.00;
float freq = 0.00;

  if (Wiper == 101 || Wiper == 126 || Wiper == 52)
  {
    //measure frequency
    ontime = pulseIn(pulse_ip,HIGH);
    offtime = pulseIn(pulse_ip,LOW);
    period = ontime+offtime;
    freq = 1000000.0/period;
    dutycycle = (ontime/period)*100;

    if (Wiper == 126){ 
    Serial.print("Frequency at 30V.................................... ");
      if (i30 == 0){
      freq30a = freq;
      i30++;}
      if (i30 == 1){
      freq30b = freq;}
    }
    if (Wiper == 101){ 
    Serial.print("Frequency at 24V.................................... ");
      if (i24 == 0){
      freq24a = freq;
      i24++;}
      if (i24 == 1){
      freq24b = freq;}
    }
    if (Wiper == 52){ 
    Serial.print("Frequency at 12.5V.................................. ");
      if (i12 == 0){
      freq12a = freq;
      i12++;}
      if (i12 == 1){
      freq12b = freq;}
    }

    freq30 = abs((freq30a+freq30b)/2);
    freq24 = abs((freq24a+freq24b)/2);
    freq12 = abs((freq12a+freq12b)/2);
    
    Serial.print(freq);
    Serial.println(" Hz");
    //
  }
}
