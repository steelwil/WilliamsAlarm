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
const int cZoneViolationDisplayDuration = 1500; // ms
unsigned long startTime = 0;
int zoneFluctuation = 50;
int zoneExpectedAnalogueValue = 511;
bool isAlarmRaised = false;
bool isArmed = false;
bool isPanic = false;
bool isZoneTriggered = false;

struct Zone
{
  int Input;
  int Output;
  int Value;
  int Status; // 0=normal, 1=short, 2=open, 3=bypass
  unsigned long StatusChanged; // ms
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

void check_Zones()
{
  isZoneTriggered = false;
  for (int i = 0; i < cNumberOfZones; i++)
  {
    if (zone[i].Status == 3) // bypass
    {
      continue;
    }

    if (HasZoneStatusChanged(zone[i]))
    {
      if (zone[i].Status == 1 || zone[i].Status == 2)
      {
        digitalWrite(zone[i].Output, HIGH);
      }
      else
      {
        digitalWrite(zone[i].Output, LOW);
      }
      Serial.print("zone[");
      Serial.print(i + 1);
      Serial.print("].Value=");
      Serial.println(zone[i].Value);
    }
    if (zone[i].Status == 1 || zone[i].Status == 2)
    {
      isZoneTriggered = true;
    }
  }
}

void trigger_Alarm()
{
  if (!isAlarmRaised)
  {
    isAlarmRaised = true;
    startTime = millis();
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
    duration = millis() - startTime;
    if (duration > cSirenDuration * 1000)
    {
      return true;
    }
  }
  return false;
}

bool HasZoneStatusChanged(Zone &zone)
{
  bool rtc = false;
  unsigned long ms = millis();
  zone.Value = analogRead(zone.Input);
  if (zone.Value < zoneExpectedAnalogueValue - zoneFluctuation)
  {
    if (zone.Status != 1)
    {
      zone.Status = 1;
      zone.StatusChanged = ms;
      rtc = true;
    }
  }
  else if (zone.Value > zoneExpectedAnalogueValue + zoneFluctuation)
  {
    if (zone.Status != 2)
    {
      zone.Status = 2;
      zone.StatusChanged = ms;
      rtc = true;
    }
  }
  else if (zone.Status > 0)
  {
    if (ms > zone.StatusChanged)
    {
      if (ms - zone.StatusChanged > cZoneViolationDisplayDuration)
      {
        zone.Status = 0;
        zone.StatusChanged = ms;
        rtc = true;
      }
    }
    else
    {
      if (ms + (0xffffffff - zone.StatusChanged) > cZoneViolationDisplayDuration)
      {
        zone.Status = 0;
        zone.StatusChanged = ms;
        rtc = true;
      }
    }
  }
  return rtc;
}

void loop()
{
  bool ready = true;
  bool armed = digitalRead(armIn);
  bool panic = digitalRead(panicIn);

  check_Zones();
  ready = !isZoneTriggered;

  if (isAlarmRaised)
  {
    if (HasAlarmTimedout())
    {
      stop_Alarm();
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
