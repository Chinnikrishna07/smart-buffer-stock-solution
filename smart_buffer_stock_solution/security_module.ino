#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Servo.h>
#include <IRremote.h>
#define EEPROM_SIZE 5

IRrecv ir(0);

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define KEY_SIZE 4
String key;

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
uint8_t colPins[COLS] = { 5, 4, 3, 2 }; // Pins connected to C1, C2, C3, C4
uint8_t rowPins[ROWS] = { 9, 8, 7, 6 }; // Pins connected to R1, R2, R3, R4
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int servoPin = 10;
Servo servo;

void setup() {
  Serial.begin(9600);
  servo.attach(servoPin);
  lcd.init();
  lcd.backlight();
  ir.enableIRIn(); // Start the receiver

  if (EEPROM.read(0) != true)
    setupLock();
}

void loop() {
  lcd.clear();
  lcd.setCursor(4,0);
  if(EEPROM.read(0) == true)
    lcd.print("Locked!");
  else
    lcd.print("Unlocked!");
  String newkey;
  while(true){
    if(ir.decode()){
      lcd.setCursor(0,0);
      lcd.print("  Enter PIN:");
      lcd.setCursor(6,1); lcd.print("[____]");
      lcd.setCursor(7,1);
      newkey = translateIR();
      break;
    }
    else if (keypad.getKey() != NO_KEY){
      lcd.clear(); lcd.print("   Enter Keycode:");
      newkey = getKey();
      break;
    }
  }
  verify(newkey);
  delay(10);
}

String translateIR(){
  char ans[KEY_SIZE] = "";
  // Takes command based on IR code received
  for(int i=0; i<KEY_SIZE; i++){
    while(!ir.decode());
    switch (ir.decodedIRData.command) {
      case 104:
        ans[i] = '0'; break;
      case 48:
        ans[i] = '1';  break;
      case 24:
        ans[i] = '2';  break;
      case 122:
        ans[i] = '3';  break;
      case 16:
        ans[i] = '4';  break;
      case 56:
        ans[i] = '5';  break;
      case 90:
        ans[i] = '6';  break;
      case 66:
        ans[i] = '7';  break;
      case 74:
        ans[i] = '8';  break;
      case 82:
        ans[i] = '9';  break;
    }
  lcd.print("*");
  ir.resume();
  delay(10);
  }
  return ans;
}


void setupLock(){
  lcd.print("   Setup Key:");
  key = getKey(); 
  for(int i=1; i<=KEY_SIZE; i++){
    EEPROM.update(i, key[i]);
  }
}
 
void unlock(){
  EEPROM.update(0,false);
  servo.write(0);
}
void lock(){
  EEPROM.update(0,true);
  servo.write(180);
}

void verify(String newKey){
  if (newKey == key){
    lcd.clear();
    lcd.print("  Unlocked!");
    unlock();
  }
  else{
    lcd.clear();
    lcd.print("   Try Again!");
    lock();
  }
  delay(1000);
}

String getKey(){
  lcd.setCursor(6,1);
  lcd.print("[____]");
  lcd.setCursor(7,1);
  String newKey;
  for(int i=0; i<KEY_SIZE; i++){
    char dig = keypad.getKey();
    while (dig == NO_KEY){
      dig = keypad.getKey();
      delay(3);
    }
    newKey += dig;
    lcd.print('*');
  }
  lcd.clear();
  lcd.setCursor(6,0);
  return newKey; 
}