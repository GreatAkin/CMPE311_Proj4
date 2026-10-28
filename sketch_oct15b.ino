#include "myMilli.h" // Includes custom myMilli() and Timer 2 ISR

// --- Hardware Pin Definitions ---
const int LED_1 = 2;
const int LED_2 = 3;
const int BUTTON_PIN = 4; // Single-pole, momentary-on button
const int MOTOR_PIN = 5;  // PWM output pin for motor

// --- Serial & LED Task Variables ---
unsigned long interval1 = 0;
unsigned long interval2 = 0;
bool led1State = LOW;
bool led2State = LOW;
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
enum State { WAITING_FOR_LED, WAITING_FOR_INTERVAL };
State currentState = WAITING_FOR_LED;
int selectedLED = 0;

// --- Motor & Button Task Variables ---
// C9 & HL.15 requires: OFF -> LOW -> MED -> HIGH -> MED -> LOW -> OFF
enum MotorState { OFF_1, LOW_1, MED_1, HIGH_1, MED_2, LOW_2 };
MotorState currentMotorState = OFF_1;

int lastButtonState = LOW;
int buttonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// --- TASK 1: Handle Serial Monitor Inputs ---
void task_SerialMonitor(void) {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      int value = input.toInt();
      if (currentState == WAITING_FOR_LED) {
        if (value == 1 || value == 2) {
          selectedLED = value;
          currentState = WAITING_FOR_INTERVAL;
          Serial.println("What interval (in msec)?");
        } else {
          Serial.println("Invalid LED. Please enter 1 or 2.");
          Serial.println("What LED? (1 or 2)");
        }
      } 
      else if (currentState == WAITING_FOR_INTERVAL) {
        if (value < 0) {
          Serial.println("Invalid interval.");
        } else {
          unsigned long interval = (unsigned long)value;
          if (selectedLED == 1) {
            interval1 = interval;
            previousMillis1 = myMilli(); 
            led1State = LOW;
            digitalWrite(LED_1, LOW);
            Serial.print("LED 1 set to blink at "); Serial.print(interval1); Serial.println(" ms");
          } else if (selectedLED == 2) {
            interval2 = interval;
            previousMillis2 = myMilli(); 
            led2State = LOW;
            digitalWrite(LED_2, LOW);
            Serial.print("LED 2 set to blink at "); Serial.print(interval2); Serial.println(" ms");
          }
        }
        currentState = WAITING_FOR_LED;
        Serial.println("What LED? (1 or 2)");
      }
    }
  }
}

// --- TASK 2: Control LED 1 Timing ---
void task_LED1(void) {
  if (interval1 > 0) {
    unsigned long currentMillis = myMilli(); 
    if (currentMillis - previousMillis1 >= interval1) {
      previousMillis1 = currentMillis;
      led1State = !led1State;
      digitalWrite(LED_1, led1State);
    }
  } else {
    led1State = LOW;
    digitalWrite(LED_1, LOW);
  }
}

// --- TASK 3: Control LED 2 Timing ---
void task_LED2(void) {
  if (interval2 > 0) {
    unsigned long currentMillis = myMilli(); 
    if (currentMillis - previousMillis2 >= interval2) {
      previousMillis2 = currentMillis;
      led2State = !led2State;
      digitalWrite(LED_2, led2State);
    }
  } else {
    led2State = LOW;
    digitalWrite(LED_2, LOW);
  }
}

// --- TASK 4: Non-Blocking Button Polling & State Machine ---
void task_Button(void) {
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = myMilli();
  }
  
  if ((myMilli() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // Advance state machine on button press
      if (buttonState == HIGH) {
        switch (currentMotorState) {
          case OFF_1:  currentMotorState = LOW_1;  break;
          case LOW_1:  currentMotorState = MED_1;  break;
          case MED_1:  currentMotorState = HIGH_1; break;
          case HIGH_1: currentMotorState = MED_2;  break;
          case MED_2:  currentMotorState = LOW_2;  break;
          case LOW_2:  currentMotorState = OFF_1;  break;
        }
        Serial.print("Fan State Changed: ");
        Serial.println(currentMotorState);
      }
    }
  }
  lastButtonState = reading;
}

// --- TASK 5: PWM Motor Controller ---
void task_Motor(void) {
  int pwmValue = 0;
  
  // Set PWM values based on justified inference for CFM/dB requirements
  switch (currentMotorState) {
    case OFF_1: pwmValue = 0;   break;
    case LOW_1: pwmValue = 90;  break;
    case MED_1: pwmValue = 150; break;
    case HIGH_1:pwmValue = 255; break;
    case MED_2: pwmValue = 150; break;
    case LOW_2: pwmValue = 90;  break;
  }
  
  analogWrite(MOTOR_PIN, pwmValue);
}

// --- CYCLIC EXECUTIVE SETUP & SCHEDULER ---
typedef void (*TaskFunction_t)(void);

TaskFunction_t taskTable[] = {
  task_SerialMonitor,
  task_LED1,
  task_LED2,
  task_Button,
  task_Motor
};

const int NUM_TASKS = sizeof(taskTable) / sizeof(taskTable[0]);

void setup() {
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT); // Assumes external pull-down resistor
  
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  analogWrite(MOTOR_PIN, 0);
  
  // Custom Timer 2 Configuration (from myMilli.h)
  MY_TCCR2A = 0x03; 
  MY_TCCR2B = 0x04; 
  MY_TCNT2  = 0x00; 
  MY_TIMSK2 = 0x01; 
  MY_SREG  |= 0x80; 

  Serial.begin(9600);
  Serial.println("System Ready: LEDs and Motor Active.");
  Serial.println("What LED? (1 or 2)");
}

void loop() {
  for (int i = 0; i < NUM_TASKS; i++) {
    taskTable[i]();
  }
}