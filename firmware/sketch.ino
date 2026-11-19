const unsigned long T_MS      = 1000;   // период опроса T = 1 с
const float         THRESHOLD = 4.0;    // порог: давление > 4 бар
const float S1_MIN = 0.0, S1_MAX = 100.0;   // диапазон NTC, °C
const float S2_MIN = 0.0, S2_MAX = 6.0;     // диапазон POT, бар
// -------------------------------------------------------------

const uint8_t PIN_NTC   = A0;
const uint8_t PIN_POT   = A1;
const uint8_t PIN_TRIG  = 7;
const uint8_t PIN_ECHO  = 8;
const uint8_t PIN_RELAY = 2;
const uint8_t PIN_LED   = 12;

unsigned long nMeas = 0;      // номер измерения n
unsigned long tPrev = 0;      // время предыдущего опроса
bool alarm = false;           // текущее состояние порога

// учебное допущение: шкала датчика линейная
float readScaled(uint8_t pin, float lo, float hi) {
  int raw = analogRead(pin);                  // 0…1023
  return lo + (hi - lo) * raw / 1023.0;
}

// расстояние, см (звук 343 м/с, путь туда-обратно)
float readDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long us = pulseIn(PIN_ECHO, HIGH, 30000UL);  // таймаут ~5 м
  if (us == 0) return 0.0;      // эха нет -> 0 см -> Node-RED запишет FAULT
  return us * 0.0343 / 2.0;
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_LED, LOW);
}

void loop() {
  if (millis() - tPrev < T_MS) return;   // опрос строго каждые T
  tPrev += T_MS;
  nMeas++;

  float s1 = readScaled(PIN_NTC, S1_MIN, S1_MAX);
  float s2 = readScaled(PIN_POT, S2_MIN, S2_MAX);
  float s3 = readDistanceCm();

  Serial.print(F("DATA n="));  Serial.print(nMeas);
  Serial.print(F(" s1="));     Serial.print(s1, 1);   // °C
  Serial.print(F(" s2="));     Serial.print(s2, 2);   // бар
  Serial.print(F(" s3="));     Serial.println(s3, 1); // см

  bool over = (s2 > THRESHOLD);
  if (over != alarm) {                   // печать события только по фронту
    alarm = over;
    digitalWrite(PIN_RELAY, alarm ? HIGH : LOW);
    digitalWrite(PIN_LED,   alarm ? HIGH : LOW);
    Serial.print(F("EVENT n=")); Serial.print(nMeas);
    Serial.println(alarm ? F(" type=THRESHOLD") : F(" type=NORMAL"));
  }
}