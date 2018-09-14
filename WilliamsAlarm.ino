/*
*/

int alarmOut = 3;
int zone1 = 0;
int zone1LED = 6;
int zone2LED = 7;
int zone1Val = 0;
int zone1Status = 0; // 0=normal, 1=short, 2=open
int zoneFluctuation = 8;
int zoneExpectedAnalogueValue = 511;
bool alarmRaised = false;

struct Zone {
  int Output;
  int Value;
  int Status;
};

void setup()
{
  pinMode(alarmOut, OUTPUT);
  pinMode(zone1LED, OUTPUT);
  pinMode(zone2LED, OUTPUT);
  digitalWrite(alarmOut, LOW);
  digitalWrite(zone1LED, LOW);
  digitalWrite(zone2LED, LOW);
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
      zone1Status = 0;
    }
  }
}

void loop()
{
  do_Alarm();

  zone1Val = analogRead(zone1);
  Serial.print("zone1Val=");
  Serial.println(zone1Val);

  if (zone1Val < zoneExpectedAnalogueValue - zoneFluctuation)
  {
    Serial.println("short");
    alarmRaised = true;
    zone1Status = 1;
  }
  else if (zone1Val > zoneExpectedAnalogueValue + zoneFluctuation)
  {
    Serial.println("open circit");
    alarmRaised = true;
    zone1Status = 2;
  }
  delay(200);

}

