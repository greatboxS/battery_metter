#include <Arduino.h>
#line 1 "d:\\vs_code\\Arduino\\metter\\metter.ino"
#include "Keyboard.h"
#include "EEPROM.h"

#define ADC_PIN A0
#define BUTTON_1_PIN B0
#define BUTTON_2_PIN B1
#define BUZZER_CONTROL_PIN B10

#define TOTAL_ADC_SAMPLE 50

#define ALARM_MAX_ADDR 0
#define ALARM_MIN_ADDR 4

#define R1_VAL 10000
#define R2_VAL 47000

float adc_result = 0.0;
int current_adc_value;
int totalCalValue = 0;

int sys_tick = 0;
int total_sample = 0;
float alarm_max = 15.0;
float alarm_min = 11.0;
double adc_ratio = 0.0;
bool adc_start = false;

uint8_t keyboard_buffer[50];

void setup()
{

  // button
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUZZER_CONTROL_PIN, OUTPUT);
  digitalWrite(BUZZER_CONTROL_PIN, LOW);

  // adc
  pinMode(ADC_PIN, INPUT_ANALOG);
  analogReadResolution(12);
  // open the serial port:
  Serial.begin(115200);
  Serial.setTimeout(100);
  // initialize control over the keyboard:
  Keyboard.begin();
  printf("12VDC Battery DC Metter\r\n");

  eeprom_read(ALARM_MIN_ADDR, alarm_min);
  eeprom_read(ALARM_MAX_ADDR, alarm_max);

  adc_ratio = 57.0 / 10.0;
}

void loop()
{

  if (Serial.available() > 0)
  {
    int timeout = 10000;
    String s = Serial.readString();
    if (s.indexOf("set-max") > -1)
    {
      while (timeout > 0)
      {
        if (Serial.available() > 0)
        {
          String num = Serial.readString();
          float max = num.toFloat();
          if (max > 0)
          {
            eeprom_write(ALARM_MAX_ADDR, max);
            printf("max value %.2f\r\n", max);
          }
        }
        timeout -= 20;
        delay(20);
      }
    }

    if (s.indexOf("set-min") > -1)
    {
      while (timeout > 0)
      {
        if (Serial.available() > 0)
        {
          String num = Serial.readString();
          float min = num.toFloat();
          if (min > 0)
          {
            eeprom_write(ALARM_MIN_ADDR, min);
            printf("min value %.2f\r\n", min);
          }
        }
        timeout -= 20;
        delay(20);
      }
    }
  }

  if (!adc_start)
  {
    current_adc_value = analogRead(ADC_PIN);
    if (current_adc_value > 1)
    {
      adc_start = true;
      total_sample = 0;
      printf("Start take sample\r\n");
    }
  }

  if (adc_start)
  {
    if (total_sample < (TOTAL_ADC_SAMPLE - 1))
    {
      int temp = analogRead(ADC_PIN);
      if (temp > 0)
      {
        totalCalValue += temp;
        total_sample++;
        printf("sample: %d -->  %d\r\n", total_sample, temp);
      }
    }
    else
    {
      adc_start = false;
      adc_result = (totalCalValue / 50.0 / 4096.0) * adc_ratio;
      totalCalValue = 0;
      send_value();
      buzzer_sound();
      // send data
    }
  }

  delay(20);
}

void metter_start()
{
  if (millis() - sys_tick > 20)
  {
    sys_tick = 0;
  }
}

void eeprom_write(int addr, float value)
{
  EEPROM.begin();
  EEPROM.put(addr, value);
  EEPROM.end();
}

void eeprom_read(int addr, float value)
{
  EEPROM.begin();
  EEPROM.get(addr, value);
  EEPROM.end();
}

void send_value()
{
  printf("send value %.2f\r\n", adc_result);
  
  int len = snprintf((char *)keyboard_buffer, sizeof(keyboard_buffer), "%.2f\r", adc_result);
  for (int i = 0; i < len; i++)
  {
    Keyboard.write(keyboard_buffer[i]);
  }
  adc_result = 0.0;

  if (adc_result > alarm_max)
  {
    Keyboard.write(26); //->
    Keyboard.write(24); //^
    Keyboard.write('H');
    Keyboard.write('i');
    Keyboard.write('g');
    Keyboard.write('h');
    Keyboard.write('\r');
  }
  else if (adc_result < alarm_min)
  {
    Keyboard.write(26); //->
    Keyboard.write(24); //^
    Keyboard.write('L');
    Keyboard.write('o');
    Keyboard.write('w');
    Keyboard.write('\r');
  }
}

void buzzer_sound()
{
  digitalWrite(BUZZER_CONTROL_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_CONTROL_PIN, LOW);
}
