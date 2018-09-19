/*
*/

const int cNumberOfZones = 4;
const int armIn = 0;
const int panicIn = 1;
const int alarmOut = 3;
const int armedOut = 4;
bool armedPressed = false;
const int readyOut = 5;
const int cSirenDuration = 10; // seconds
unsigned long startTime = 0;
int zoneFluctuation = 50;
int zoneExpectedAnalogueValue = 511;
bool isAlarmRaised = false;
bool isArmed = false;
bool isPanic = false;

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
    pinMode(armIn, INPUT_PULLUP);
    pinMode(panicIn, INPUT_PULLUP);
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

void trigger_Alarm()
{
  if (!isAlarmRaised)
  {
    isAlarmRaised = true;
    startTime = micros();
    digitalWrite(alarmOut, HIGH);
  }
}

void stop_Alarm()
{
  if (isAlarmRaised)
  {
    isAlarmRaised = false;
    isPanic = false;
    digitalWrite(alarmOut, LOW);
  }
}

bool HasAlarmTimedout()
{
  unsigned long duration = 0;
  if (isAlarmRaised)
  {
    duration = micros() - startTime;
    if (duration > cSirenDuration * 1000000)
    {
      return true;
    }
  }
  return false;
}

bool IsZoneTriggered(Zone& zone)
{
  zone.Value = analogRead(zone.Input);
  zone.Status = 0;
  if (zone.Value < zoneExpectedAnalogueValue - zoneFluctuation)
  {
    zone.Status = 1;
  }
  else if (zone.Value > zoneExpectedAnalogueValue + zoneFluctuation)
  {
    zone.Status = 2;
  }
  return (zone.Status > 0);
}

void loop()
{
  bool ready = true;
  bool armed = digitalRead(armIn);
  bool panic = digitalRead(panicIn);

  if (isAlarmRaised)
  {
    if (HasAlarmTimedout())
    {
      stop_Alarm();
    }
  }

  for (int i = 0; i < cNumberOfZones; i++)
  {
    if (zone[i].Status == 3) // bypass
    {
      continue;
    }

    if (IsZoneTriggered(zone[i]))
    {
      digitalWrite(zone[i].Output, HIGH);
      ready = false;
      Serial.print("zone[");
      Serial.print(i + 1);
      Serial.print("].Value=");
      Serial.println(zone[i].Value);
    }
    else
    {
      digitalWrite(zone[i].Output, LOW);
    }
  }

  if (isArmed)
  {
    digitalWrite(readyOut, LOW);
    if (!ready)
    {
      trigger_Alarm();
    }
  }
  else if (ready)
  {
    digitalWrite(readyOut, HIGH);
  }
  else
  {
    digitalWrite(readyOut, LOW);
  }

  if (armed == false)
  {
    Serial.println("armed button pressed");

    if (armedPressed == false)
    {
      armedPressed = true;
      if (isArmed || isPanic)
      {
        stop_Alarm();
        isArmed = false;
      }
      else if (ready)
      {
        isArmed = true;
      }
      digitalWrite(armedOut, isArmed);
    }
  }
  else
  {
    armedPressed = false;
  }

  if (panic == false) // normally high
  {
    isPanic = true;
    Serial.println("Panic");
    trigger_Alarm();
  }

  delay(100);

}

