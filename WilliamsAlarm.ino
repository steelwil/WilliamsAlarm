/*
*/

const int cNumberOfZones = 4;
const int alarmOut = 3;
const int armedOut = 4;
const int readyOut = 5;

int zoneFluctuation = 50;
int zoneExpectedAnalogueValue = 511;
bool alarmRaised = false;
bool armed = false;

struct Zone {
  int Input;
  int Output;
  int Value;
  int Status; // 0=normal, 1=short, 2=open, 3=bypass
};

Zone zone[cNumberOfZones];


void setup()
{
  for (int i = 0; i < cNumberOfZones; i++)
  {
    zone[i].Input = 0 + i;
    zone[i].Output = 6 + i;
    pinMode(zone[i].Input, OUTPUT);
    pinMode(zone[i].Output, OUTPUT);
    digitalWrite(zone[i].Input, LOW);
    digitalWrite(zone[i].Output, LOW);
  }

  pinMode(alarmOut, OUTPUT);
  digitalWrite(alarmOut, LOW);
  pinMode(armedOut, OUTPUT);
  digitalWrite(armedOut, LOW);
  pinMode(readyOut, OUTPUT);
  digitalWrite(readyOut, LOW);

  Serial.begin(9600);
}

void do_Alarm()
{
  static unsigned long startTime = 0;
  unsigned long duration = 0;
  if (alarmRaised)
  {
    Serial.println(startTime);

    if (startTime == 0)
    {
      startTime = micros();
    }
    duration = micros() - startTime;
    if (duration < 10000000)
    {
      Serial.println(duration);
      digitalWrite(alarmOut, HIGH);   // Blink the LED
      delay(400);
      digitalWrite(alarmOut, LOW);
      delay(400);
    }
    else
    {
      startTime = 0;
      alarmRaised = false;
      //zone1.Status = 0;
    }
  }
}

void loop()
{
  do_Alarm();
  bool ready = true;

  for (int i = 0; i < cNumberOfZones; i++)
  {
    if (zone[i].Status == 3) // bypass
    {
      continue;
    }
    zone[i].Value = analogRead(zone[i].Input);
    Serial.print("zone[");
    Serial.print(i + 1);
    Serial.print("].Value=");
    Serial.println(zone[i].Value);

    if (zone[i].Value < zoneExpectedAnalogueValue - zoneFluctuation)
    {
      Serial.println("short");
      alarmRaised = true;
      zone[i].Status = 1;
      digitalWrite(zone[i].Output, HIGH);
      ready = false;
    }
    else if (zone[i].Value > zoneExpectedAnalogueValue + zoneFluctuation)
    {
      Serial.println("open circit");
      alarmRaised = true;
      zone[i].Status = 2;
      digitalWrite(zone[i].Output, HIGH);
      ready = false;
    }
    else
    {
      zone[i].Status = 0;
      digitalWrite(zone[i].Output, LOW);
    }
  }

  if (armed)
  {
    ready = false;
  }
  if (ready)
  {
    digitalWrite(readyOut, HIGH);
    Serial.println("ready");
  }
  else
  {
    digitalWrite(readyOut, LOW);
  }

  delay(100);

}

