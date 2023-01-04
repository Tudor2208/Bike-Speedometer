#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#define buzzer_pin 3
#define hall_sensor_pin A0
LiquidCrystal_I2C lcd(0x27,20,4);

int op = 1;
bool alarm_activated = false;

const int val_detect_magnet = 500;
const float dist_between_detections = 2.194; //m 
const int max_time_between_detections = 5;

long long t0 = 0;
long long t1 = 0;

float seconds = 0;
float current_speed = 0;
float max_speed = 0;
float total_distance = 0;
int current_rpm = 0;

boolean stringComplete = false;
String inputString = "";

void setup() {
  op = 1;
  Serial.begin(9600);
  
  // configurare pini
  pinMode(hall_sensor_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
  
  //initializare LCD
  lcd.init();
  lcd.backlight();
  
  printInfo(0, 0);
  analogWrite(buzzer_pin, 255);  
}

void printInfo(float speed, float dist) {
  //speed (m/s), dist (m)
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Speed:");
  if(op > 0) {
    lcd.print(speed * 3.6);
    lcd.print("km/h");
  } else {
    lcd.print(current_rpm);
    lcd.print(" RPM");
  }
  lcd.setCursor(0, 1);
  lcd.print("Distance:");
  lcd.print(dist / 1000);
  lcd.print("km");
}

void was_detected() {
  t0 = t1;
  t1 = millis();
  seconds = (float)(t1 - t0) / 1000;
  current_speed = dist_between_detections / seconds;
  current_rpm = (int)60 / seconds;
  if(current_speed * 3.6> max_speed) {
    max_speed = current_speed * 3.6;
  }
  total_distance += dist_between_detections;
  printInfo(current_speed, total_distance);
}

void check_stop(){
  if(millis() - t1 > 1000 * max_time_between_detections) {
    //bicicleta s-a oprit
    t1 = millis();
    current_speed = 0;
    current_rpm = 0;
    seconds = 0;
    printInfo(current_speed, total_distance);
    
  }
}
void alarm() {
  for(int i=0; i<255; i+=5) {
    analogWrite(buzzer_pin, i);
    delay(50); 
  }

  for(int i=255; i>0; i-=5) {
    analogWrite(buzzer_pin, i);
    delay(50); 
  }

  analogWrite(buzzer_pin, 255);    
}

void loop() {
  int val_read = analogRead(hall_sensor_pin);
  delay(5);

  if (val_read < val_detect_magnet){
    was_detected();
  } else {
    check_stop();
  }

  if(current_speed > 0 && alarm_activated == true) {
    alarm();
  }

  if (Serial.available()) {
    char c = Serial.read();
    if (c != '\n')
      inputString += c;
    else
    {
      stringComplete = true;
    }
  }

  if (stringComplete) {
    if (inputString.indexOf("alarm_enable") == 0) {
      alarm_activated = true;
    } else if (inputString.indexOf("alarm_disable") == 0) {
      alarm_activated = false;
    } else if (inputString.indexOf("max_speed") == 0) {
      Serial.print("Viteza maxima: ");
      Serial.print(max_speed);
      Serial.println("km/h");
    } else if (inputString.indexOf("reset_distance") == 0) {
      total_distance = 0;
    }

    inputString = "";
    stringComplete = false;
   }

   if(millis() % 3000 == 0) {
      op = op * -1;
   }
  
}
