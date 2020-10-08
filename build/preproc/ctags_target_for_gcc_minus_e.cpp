# 1 "d:\\vs_code\\Arduino\\metter\\metter.ino"

# 3 "d:\\vs_code\\Arduino\\metter\\metter.ino" 2
# 4 "d:\\vs_code\\Arduino\\metter\\metter.ino" 2
# 18 "d:\\vs_code\\Arduino\\metter\\metter.ino"
float adc_result = 0.0;
int current_adc_value;
int totalCalValue = 0;

int sys_tick = 0;
int total_sample = 0;
float alarm_max = 15.0;
float alarm_min = 11.0;
double adc_ratio = 0.0;
bool adc_start = false;
char keyboard_buffer[50];
bool isSent = false;
int timeout_tick = 0;

bool manual_mode = false;

void setup()
{
  // button
  pinMode(0, 0x2);
  pinMode(1, 0x2);
  pinMode(2, 0x1);
  digitalWrite(2, 0x0);

  // adcs
  pinMode(A0, 0x4);
  analogReadResolution(12);
  // open the serial port:
  Serial1.begin(115200);
  Serial1.setTimeout(100);
  // initialize control over the keyboard:
  Keyboard.begin();
  printf("12VDC Battery DC Metter\r\n");

  int alr_min = 0, alr_max = 0;
  alr_min = eeprom_read(4);
  alr_max = eeprom_read(0);

  alarm_max = alr_max / 100;
  alarm_min = alr_min / 100;

  adc_ratio = 57.0 / 10.0;

  if (alarm_min < 0 || alarm_min > 15)
  {
    int temp = 1100;
    eeprom_write(4, temp);
    alr_min = eeprom_read(4);
    alarm_min = alr_min / 100.0;
  }

  if (alarm_max < 0 || alarm_max > 20)
  {
    int temp = 1500;
    eeprom_write(0, temp);
    alr_max = eeprom_read(0);
    alarm_max = alr_max / 100.0;
  }
}

void loop()
{
  if (digitalRead(0) == 0x0)
  {
    int t = 5000;
    while (digitalRead(0) == 0x0)
    {
      delay(50);
      t -= 50;
      if (t < 0)
        break;
    }
    manual_mode = !manual_mode;
    if(manual_mode)
      printf("select manual mode\r\n");
      else
      printf("select auto mode\r\n");

    metter_reset();
  }

  if (digitalRead(1) == 0x0)
  {
    int t = 5000;
    while (digitalRead(1) == 0x0)
    {
      delay(50);
      t -= 50;
      if (t < 0)
        break;
    }
    metter_reset();
    adc_start = true;
  }

  if (Serial1.available() > 0)
  {

    int timeout = 10000;
    String s = Serial1.readString();

    if (s.indexOf("set mode manual") > -1)
    {
      manual_mode = true;
      metter_reset();
    }

    if (s.indexOf("set mode auto") > -1)
    {
      manual_mode = false;
      metter_reset();
    }

    if (s.indexOf("manual read") > -1)
    {
      metter_reset();
      adc_start = true;
    }

    if (s.indexOf("set-max") > -1)
    {
      printf("Set alarm max value (float):\r\n");
      while (timeout > 0)
      {
        if (Serial1.available() > 0)
        {
          String num = Serial1.readString();
          float max = num.toFloat();
          int i_max = (int)(max * 100.0);
          if (i_max > 0)
          {
            eeprom_write(0, i_max);
            printf("max value %.2f\r\n", max);
          }
        }
        timeout -= 20;
        delay(20);
      }
    }

    if (s.indexOf("set-min") > -1)
    {
      printf("Set alarm min value (float):\r\n");
      while (timeout > 0)
      {
        if (Serial1.available() > 0)
        {
          String num = Serial1.readString();
          float min = num.toFloat();
          int i_min = (int)(min * 100.0);
          if (i_min > 0)
          {
            eeprom_write(4, i_min);
            printf("min value %.2f\r\n", min);
          }
        }
        timeout -= 20;
        delay(20);
      }
    }
  }

  if (manual_mode == false)
  {
    if (!adc_start)
    {
      current_adc_value = analogRead(A0);
      if (current_adc_value > 10)
      {
        metter_reset();
        adc_start = true;
        printf("Start take sample\r\n");
      }
    }
  }
  if (adc_start)
  {
    if (total_sample < (50 - 1))
    {
      timeout_tick++;
      if (timeout_tick >= 100)
      {
        metter_reset();
        printf("timeout\r\n");
      }
      int temp = analogRead(A0);
      if (temp >= 0)
      {
        totalCalValue += temp;
        total_sample++;
        printf("sample: %d -->  %d\r\n", total_sample, temp);
      }
    }
    else
    {
      adc_result = (totalCalValue / 50.0 / 4095.0) * adc_ratio * 3.3;
      if (!isSent)
      {
        printf("total adc value %d\r\n", totalCalValue);
        printf("avg value = %.2f\r\n",(float) totalCalValue/50.0);
        send_value();
        buzzer_sound();
        isSent = true;
      }
      else
      {
        int check_adc = analogRead(A0);
        if (check_adc == 0)
        {
          metter_reset();
        }
      }
    }
  }

  delay(20);
}

void metter_reset()
{
  adc_start = false;
  isSent = false;
  totalCalValue = 0;
  total_sample = 0;
  timeout_tick = 0;
}

void metter_start()
{
  if (millis() - sys_tick > 20)
  {
    sys_tick = 0;
  }
}

void eeprom_write(int addr, int value)
{
  EEPROM.begin();
  for (int i = 0; i < 4; i++)
  {
    EEPROM.write(addr + i, (uint8_t)(value >> (i * 8)));
  }
  EEPROM.end();
  printf("write value %d\r\n", value);
}

int eeprom_read(int addr)
{
  int value = 0;
  EEPROM.begin();
  for (int i = 0; i < 4; i++)
  {
    uint8_t temp = EEPROM.read(addr + i);
    value += (temp << (8 * i));
  }
  EEPROM.end();
  printf("read value %d\r\n", value);
  return value;
}

void send_value()
{
  printf("send value %.2f\r\n", adc_result);

  int len = snprintf((char *)keyboard_buffer, sizeof(keyboard_buffer), "%.2f", adc_result);
  printf("packet len %d\r\n", len);
  for (int i = 0; i < len; i++)
  {
    Keyboard.write(keyboard_buffer[i]);
  }

  if (adc_result > alarm_max)
  {
    /*



In short:



left arrow: 37

up arrow: 38

right arrow: 39

down arrow: 40

tab 09

    */
# 301 "d:\\vs_code\\Arduino\\metter\\metter.ino"
    Keyboard.write(9); //tab
    Keyboard.write('H');
    Keyboard.write('i');
    Keyboard.write('g');
    Keyboard.write('h');
    Keyboard.write('\r\n');
    Keyboard.write(27); // <-
  }
  else if (adc_result < alarm_min)
  {
    Keyboard.write(9); //tab
    Keyboard.write('L');
    Keyboard.write('o');
    Keyboard.write('w');
    Keyboard.write('\r\n');
    Keyboard.write(27); // <-
  }
  else
  {
    Keyboard.write('\r\n');
  }
}

void buzzer_sound()
{
  digitalWrite(2, 0x1);
  delay(50);
  digitalWrite(2, 0x0);
}
