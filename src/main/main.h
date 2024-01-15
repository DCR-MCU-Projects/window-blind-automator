#include <Servo.h>

#define SERVO_CONTROL_PIN 5
#define PHOTO_INTERRUPTER_PIN 4
#define LED_PIN 2

#define EMERGENCY_STOP_DELTA 100
#define COMPLETE_TOUR 50

int tickcount = -1;

int motor_tick_start = -1;
int motor_tick_count = -1;

void IRAM_ATTR PhotoInterrupterInterrupt();

void startServo(short speed, short turnCount = 1, bool cw = true);
void stopServo(int recoilms = -1);

// void getStringToken(int &tokenCount, String* &tokenContent, String s);