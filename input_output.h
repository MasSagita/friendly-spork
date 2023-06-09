#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//char panah
byte customChar[] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000
};

int btn_pin[3] = {2, 3, 4};

int ir_pin = A0;

const int led_pin = 13;
const int buz_pin = 12;

const int motor_pwm_pin = 9;
const int motor_left_pin = 10;
const int motor_right_pin = 11;

void setup_input_output() {
  lcd.init();

  for (int i = 0; i < 3; i++) {
    pinMode(btn_pin[i], INPUT);
  }
  pinMode(ir_pin, INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(buz_pin, OUTPUT);
  
  pinMode(motor_pwm_pin, OUTPUT);
  pinMode(motor_left_pin, OUTPUT);
  pinMode(motor_right_pin, OUTPUT);
  
  for (int i = 0; i < 6; i++) {
    digitalWrite(buz_pin, !digitalRead(buz_pin));
    digitalWrite(led_pin, !digitalRead(led_pin));
    delay(30);
  }
  
  lcd.createChar(0, customChar);
  lcd.backlight();
  lcd.setCursor(0, 0), lcd.print(F(" CAP's ELEVATOR"));
  delay(500);
  lcd.clear();
}

void run_motor(int spd, int brk) {
  if (spd == 0 && brk == 1) {
    analogWrite(motor_pwm_pin, 255);
    digitalWrite(motor_left_pin, 0);
    digitalWrite(motor_right_pin, 0);
  }
  if (spd == 0 && brk == 0) {
    analogWrite(motor_pwm_pin, 0);
    digitalWrite(motor_left_pin, 0);
    digitalWrite(motor_right_pin, 0);
  }
  if (spd > 0) {
    analogWrite(motor_pwm_pin, spd);
    digitalWrite(motor_left_pin, 1);
    digitalWrite(motor_right_pin, 0);
  }
  if (spd < 0) {
    analogWrite(motor_pwm_pin, -spd);
    digitalWrite(motor_left_pin, 0);
    digitalWrite(motor_right_pin, 1);
  }
}

bool button(int ch) {
  return !digitalRead(btn_pin[ch]); 
}

bool ir_state() {
  return !digitalRead(ir_pin); 
}

int last_state_ir = HIGH;
int ir_counter = 0;
int total_cap = 0;

void counter_cap() {
  if (!last_state_ir && ir_state()) {
    ir_counter += 1;
    total_cap += 1;
  }
  last_state_ir = ir_state();
}

unsigned long prev_millis_led;
int led_state = LOW;
void led_blink(int on_time, int off_time) {
  unsigned long current_millis_led = millis();
  if ((led_state == 1) && (current_millis_led - prev_millis_led >= on_time)) {
    led_state = 0;
    prev_millis_led = current_millis_led;
  }
  else if ((led_state == 0) && (current_millis_led - prev_millis_led >= off_time)) {
    led_state = 1;
    prev_millis_led = current_millis_led;
  }
  digitalWrite(led_pin, led_state);
  //if(ch == 1) digitalWrite(buzzer_pin, led_state);
}

void beep_buz(int sum, int interval) {
  if (sum > 0) sum = sum * 2;
  if (interval >= 500) interval = 500;
  for (int i = 0; i < sum; i++) {
    digitalWrite(buz_pin, !digitalRead(buz_pin));
    delay(interval);
  }
}



const int short_press_time = 500;
const int long_press_time = 1000;

int last_state_rtry = LOW;  // previous state
int current_state_rtry;     // current reading
unsigned long pressed_time = 0;
unsigned long realesed_time = 0;
bool is_pressing = false;
bool is_long_pressing = false;

bool long_press_btn = false;
bool short_press_btn = false;

void short_long_ir(int pin) {
  current_state_rtry = pin;
  if (last_state_rtry == HIGH && current_state_rtry == LOW) {
    pressed_time = millis();
    is_pressing = true;
    is_long_pressing = false;
  } else if (last_state_rtry == LOW && current_state_rtry == HIGH) {
    is_pressing = false;
    realesed_time = millis();
    long press_duration = realesed_time - pressed_time;
    if (press_duration < short_press_time) {
      long_press_btn = false;
      short_press_btn = true;
      Serial.print("short press ");
      Serial.println(pressed_time);
    }
  }
  if (is_pressing == true && is_long_pressing == false) {
    long press_duration = millis() - pressed_time;
    if (press_duration > long_press_time) {
      long_press_btn = true;
      short_press_btn = false;
      Serial.print("long press");
      Serial.println(pressed_time);
      is_long_pressing = true;
    }
  }
  last_state_rtry = current_state_rtry;
}

int refresh;
void refresh_screen(int intervalRefresh) {
  if (++refresh > intervalRefresh) {
    lcd.clear();
    refresh = 0;
  }
}

