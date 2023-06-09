#include <EEPROM.h>

#include "input_output.h"

int pw_speed;

int ir_stop;

void (* resetFunc) (void) = 0;

void setup() {
  // put your setup code here, to run once:
  pw_speed = EEPROM.read(0);

  Serial.begin(115200);
  Serial.println("Cap Elevator Orienter");

  setup_input_output();

  standby();
}

int menu = 0;
int cursor_menu = 0;

int dummy_speed = 0;

int delay_ir = 1000;

int smooth_start_speed = 0;

bool masuk_standby = false;

int last_counter_cap;

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(led_pin, !digitalRead(led_pin));
  refresh_screen(5);


  last_counter_cap = total_cap;

  lcd.setCursor(0, 0), lcd.print(F("    Elev Run"));
  lcd.setCursor(0, 1), lcd.print(F("S:")), lcd.print(pw_speed);
  lcd.setCursor(7, 1), lcd.print(total_cap);
  lcd.setCursor(12, 1), lcd.print(F("IR:")), lcd.print(ir_state());


  if (button(0)) pw_speed += 5, beep_buz(1, 25);
  if (button(1)) pw_speed -= 5, beep_buz(1, 25);

  if (pw_speed > 255) pw_speed = 255;
  if (pw_speed < 0) pw_speed = 0;

  dummy_speed = pw_speed;

  counter_cap();

  if (ir_state()) {
    lcd.clear();
    beep_buz(1, 25);
    bool cap_full = true;
    unsigned long prev_millis = millis();
    //Serial.println(millis() - prev_millis);
    smooth_start_speed = 0;
    while (1) {
      lcd.setCursor(0, 0), lcd.print(F("    Cap Full"));
      if (!ir_state()) {
        cap_full = false;
        if (smooth_start_speed >= pw_speed || dummy_speed == pw_speed) break;
        if (smooth_start_speed == 0) beep_buz(1, 500);
        smooth_transition(&smooth_start_speed, pw_speed, 5, 25);
        run_motor(smooth_start_speed, 1);
      }
      if (cap_full && millis() - prev_millis >= delay_ir) {
        lcd.setCursor(0, 1), lcd.print(F("    ElevStop"));
        // variable to smooth, target, step, interval(ms)
        smooth_transition(&dummy_speed, 0, 5, 5);
        run_motor(dummy_speed, 1);
        //cap_full = true;
        if (button(2)) masuk_standby = false, standby();
      }
      if (dummy_speed == 0 && smooth_start_speed == 0) {
        led_blink(100, 1000);
        digitalWrite(buz_pin, HIGH), delay(100);
        digitalWrite(buz_pin, LOW), delay(1000);
      }
    }

  }
  if (!ir_state()) run_motor(pw_speed, 1);

  if (button(2)) masuk_standby = false, standby();
}

void standby() {
  lcd.clear();
  while (!masuk_standby) {
    led_blink(100, 100);
    smooth_transition(&dummy_speed, 0, 5, 20);
    run_motor(dummy_speed, 1);
    lcd.setCursor(0, 0), lcd.print(F(" Elevator Stop"));
    lcd.setCursor(0, 1), lcd.print(F("      S:")), lcd.print(dummy_speed);
    if (dummy_speed == 0) {
      beep_buz(1, 500), masuk_standby = true;
      break;
    }
  }

  while (masuk_standby) {
    refresh_screen(3);
    lcd.setCursor(0, 0), lcd.print(F("Elevator StandBy"));
    lcd.setCursor(11, 1), lcd.print(total_cap);
    lcd.setCursor(15, 1), lcd.print(ir_state());

    if (button(0) && button(1)) save();

    if (!button(0) && !button(1)) {
      led_blink(25, 1000);
      lcd.setCursor(0, 1), lcd.print(F("OFF :"));
      lcd.print(F("0")), run_motor(0, 1);
    }

    if (button(0)) {
      led_blink(50, 100);
      lcd.setCursor(0, 1), lcd.print(F("UP  :"));
      run_motor(pw_speed, 1), lcd.print(pw_speed);
    }
    if (button(1)) {
      led_blink(100, 50);
      lcd.setCursor(0, 1), lcd.print(F("DOWN:"));
      run_motor(-pw_speed, 1), lcd.print(-pw_speed);
    }

    if (button(2)) {
      lcd.clear();
      while (1) {
        lcd.setCursor(0, 0), lcd.print(F(" Elevator Start"));
        lcd.setCursor(0, 1), lcd.print(F("      S:")), lcd.print(smooth_start_speed);
        led_blink(100, 100);
        if (smooth_start_speed == 0) beep_buz(2, 50);
        smooth_transition(&smooth_start_speed, pw_speed, 5, 20);
        run_motor(smooth_start_speed, 1);
        if (smooth_start_speed >= pw_speed) break;
      }
      if (smooth_start_speed >= pw_speed) {
        delay(100);
        smooth_start_speed = 0;
        break;
      }
    }
  }
}

void smooth_transition(int* value, int target, int stp, unsigned long interval) {
  static unsigned long previousTime = 0; // Previous time

  unsigned long currentTime = millis(); // Current time
  unsigned long elapsedTime = currentTime - previousTime; // Elapsed time

  if (elapsedTime >= interval) {
    previousTime = currentTime; // Update the previous time
    if (*value < target) {
      *value += stp;
      if (*value > target) *value = target;
    }
    else if (*value > target) {
      *value -= stp;
      if (*value < target) *value = target;
    }
    //Serial.print("Current value: ");
    //Serial.println(*value);
  }
}

void save() {
  beep_buz(2, 400);
  delay(500), lcd.clear();
  lcd.setCursor(0, 0), lcd.print("  Save & Rstrt");
  EEPROM.write(0, pw_speed);
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1), lcd.print(F("."));
    delay(100);
  }
  resetFunc();
}

