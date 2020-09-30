#include <Arduino.h>
#include <GravityTDS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#define TdsSensorPin A0
#define ONE_WIRE_BUS 2
#define TurbiditySensorPin A2
#define WaterFlowSensorPin A3
#define PHSensorPin A4 //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00    //deviation compensate ph
#define LED 13
#define printInterval 800
#define ArrayLenth 40 //times of collection

GravityTDS gravityTds;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void TDS();
void Suhu();
void Turbidity();
void WaterFlow();
void PH();
double avergearray(int *arr, int number);
int X;
int Y;
int samplingInterval;
int pHArray[ArrayLenth]; //Store the average value of the sensor feedback
int pHArrayIndex = 0;
float TDStemperature = 25, tdsValue = 0;
float tegangan; //data untuk tegangan sensor turbidity
float ntu;      //data untuk nilai pembacaan satuan sensor turbidity
float round_to_dp(float, int);
float TIME = 0;
float FREQUENCY = 0;
float WATER = 0;
float TOTAL = 0;
float LS = 0;

void setup()
{
  Serial.begin(9600);
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);      //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024); //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();           //initialization TDS
  sensors.begin();
  pinMode(LED, OUTPUT);
  pinMode(WaterFlowSensorPin, INPUT);
}

void loop()
{
  TDS();
  Suhu();
  Turbidity();
  WaterFlow();
  PH();
}

void TDS()
{
  gravityTds.setTemperature(TDStemperature); // set the temperature and execute temperature compensation
  gravityTds.update();                       //sample and calculate
  tdsValue = gravityTds.getTdsValue();       // then get the value
  Serial.print("TDS Value: ");
  Serial.print(tdsValue, 0);
  Serial.println(" ppm");
  delay(1000);
}

void Suhu()
{
  sensors.requestTemperatures(); // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  Serial.print("Celsius temperature: ");
  Serial.println(sensors.getTempCByIndex(0));
  delay(1000);
}

void Turbidity()
{
  tegangan = 00;
  for (int i = 00; i < 800; i++)
  {
    tegangan += ((float)analogRead(TurbiditySensorPin) / 1023) * 5;
  }
  tegangan = tegangan / 800;
  tegangan = round_to_dp(tegangan, 1);
  if (tegangan < 2.5)
  {
    ntu = 3000;
  }
  else
  {
    ntu = -1120.4 * sq(tegangan) + 5742.3 * tegangan - 4353.8;
  }
  Serial.print("Sensor Turbidity: ");
  Serial.print(tegangan);
  Serial.print(" V  ");
  Serial.print(ntu);
  Serial.println(" NTU");
  delay(1000);
}

float round_to_dp(float nilaibaca, int desimal)
{
  float multiplier = powf(10.0f, desimal);
  nilaibaca = roundf(nilaibaca * multiplier) / multiplier;
  return nilaibaca;
}

void WaterFlow()
{
  X = pulseIn(WaterFlowSensorPin, HIGH);
  Y = pulseIn(WaterFlowSensorPin, LOW);
  TIME = X + Y;
  FREQUENCY = 1000000 / TIME;
  WATER = FREQUENCY / 7.5;
  LS = WATER / 60;
  if (FREQUENCY >= 0)
  {
    if (isinf(FREQUENCY))
    {
      Serial.println("VOL. : 0.00");
      Serial.println("TOTAL : ");
      Serial.print(TOTAL);
      Serial.print(" L");
    }
    else
    {
      TOTAL = TOTAL + LS;
      Serial.println(FREQUENCY);
      Serial.println("VOL. : ");
      Serial.print(WATER);
      Serial.println(" L / M");
      Serial.println("TOTAL : ");
      Serial.print(TOTAL);
      Serial.print(" L");
    }
  }
  delay(1000);
}

void PH()
{
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue, voltage;
  samplingInterval = 0;
  while (samplingInterval < 20)
  {
    pHArray[pHArrayIndex++] = analogRead(PHSensorPin);
    if (pHArrayIndex == ArrayLenth)
      pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    pHValue = 3.5 * voltage + Offset;
    samplingInterval++;
    delay(10);
  }
  Serial.print("PH Sensor");
  Serial.print("Voltage:");
  Serial.println(voltage, 2);
  Serial.print("    pH value: ");
  Serial.println(pHValue, 2);
  digitalWrite(LED, digitalRead(LED) ^ 1);
}

double avergearray(int *arr, int number)
{
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0)
  {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5)
  { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++)
    {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else
  {
    if (arr[0] < arr[1])
    {
      min = arr[0];
      max = arr[1];
    }
    else
    {
      min = arr[1];
      max = arr[0];
    }
    for (i = 2; i < number; i++)
    {
      if (arr[i] < min)
      {
        amount += min; //arr<min
        min = arr[i];
      }
      else
      {
        if (arr[i] > max)
        {
          amount += max; //arr>max
          max = arr[i];
        }
        else
        {
          amount += arr[i]; //min<=arr<=max
        }
      } //if
    }   //for
    avg = (double)amount / (number - 2);
  } //if
  return avg;
}