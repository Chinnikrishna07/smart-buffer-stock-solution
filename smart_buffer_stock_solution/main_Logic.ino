#include <WiFi.h>
#include <HTTPClient.h>
#include "DHTesp.h"
#include "LiquidCrystal_I2C.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int channelNum = 1904920;
const String writeAPIKey = "TKCUKXHB1WQ7H76H";
DHTesp dht;
byte led = 33;
byte dhtSensor = 13;
short dist;
String url="https://dweet.io/dweet/for/hyd-storage?dist=";
String url1="https://api.thingspeak.com/update?api_key=TKCUKXHB1WQ7H76H";
constexpr byte noOfSensors = 2;
HTTPClient http;

struct ultraSonic{
  const byte trigPin, echoPin;
};
struct ultraSonic sensors[noOfSensors]={
  {2,4},
  {12,14}
};

class rgbLed{
  const short red,green,blue;
  public:
    rgbLed(byte r, byte g, byte b): red(r),green(g),blue(b){
    pinMode(r, OUTPUT);
    pinMode(g, OUTPUT);
    pinMode(b,OUTPUT);
    set(0,0,0);
  }
  void set(byte r, byte g, byte b){
    analogWrite(this->red,r);
    analogWrite(this->green,g);
    analogWrite(this->blue,b);
  }
};
rgbLed leds[noOfSensors]={
  {18,19,23},
  {25,26,27},
};

short detect(struct ultraSonic sensor){
  digitalWrite(sensor.trigPin,LOW);
//  digitalWrite(LED_BUILTIN,HIGH);
  delay(3);
  digitalWrite(sensor.trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(sensor.trigPin,LOW);
  int distance=0.034/2*pulseIn(sensor.echoPin,HIGH);
  // dist=speed of sound/2 * round trip time taken by the wave
  return distance;
}

void setup() {
  lcd.init(); lcd.backlight();
  pinMode(dhtSensor, INPUT);
  pinMode(led, OUTPUT);
  for(ultraSonic i: sensors){
    pinMode(i.trigPin, OUTPUT);
    pinMode(i.echoPin, INPUT);
  }
  dht.setup(dhtSensor, DHTesp::DHT22);
  Serial.begin(9600);
  WiFi.begin("Wokwi-GUEST","",6);
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print('.');
  }
  Serial.println("Connected!");
  }

void loop() {
  short i=0,avg=0;
  for(i=0; i<noOfSensors; i++ ){
  dist=detect(sensors[i]);
  avg+=dist;
  Serial.printf("Sensor%d: %3d   ",i,dist);
  leds[i].set(255-dist*0.6375, dist*0.6375,0);
  }
  avg=avg/i;

  short temp = dht.getTemperature();
  short humidity = dht.getHumidity();
  Serial.printf("Average: %d \n-> ",avg);
  lcd.setCursor(0,0);
  lcd.print("Temp: "); lcd.print(temp);
  lcd.print("'C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: "); lcd.print(humidity);
  lcd.print("%");

  String dweet =  url + dist + "&hum=" + humidity + "&temp=" + temp;
  String thingsp = url1+ "&field3=" + humidity + "&field2=" + temp;
  http.begin(dweet);
  short httpResponse = http.GET();
  if (httpResponse){
    Serial.println("Sent!");
    digitalWrite(led,HIGH);
    delay(1000);
    digitalWrite(led,LOW);
  }
  else 
    Serial.println("Error, not sent :(");
  http.end();
   http.begin(thingsp); http.GET(); http.end();
  delay(6000);
}