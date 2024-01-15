#include "main.h"

Servo myservo; 

String buf = "";

void setup() {
  
  pinMode(PHOTO_INTERRUPTER_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  // attachInterrupt(digitalPinToInterrupt(PHOTO_INTERRUPTER_PIN), PhotoInterrupterInterrupt, RISING);

  Serial.begin(115200);


} 

int last_tick = 0;
bool is_active = false;
int tour_countdown_index = 0;

bool is_cw = true;
bool photo_interrupter_state = false;
bool photo_interrupter_laststate = false;

int photo_interrupter_delta = 0;
int photo_interrupter_lasttimedelta = 0;

int photo_interrupter_emergency_stop_delta = EMERGENCY_STOP_DELTA;

bool init_servo = false;

void loop() {

  // noInterrupts();
  
  // interrupts();

  // photo_interrupter_state = digitalRead(PHOTO_INTERRUPTER_PIN);
  // Serial.println(photo_interrupter_state);
  // if (photo_interrupter_state != photo_interrupter_laststate) {
  //   Serial.print("Change to: ");
  //   Serial.println(photo_interrupter_state);
  //   photo_interrupter_laststate = photo_interrupter_state;
  // }

  
  
  if (last_tick != tickcount) {

    Serial.print(photo_interrupter_lasttimedelta);
    Serial.print(" : ");
    Serial.print(photo_interrupter_delta);
    Serial.print(" : ");
    Serial.print(last_tick);
    Serial.print(" : ");
    Serial.println(tickcount);
    last_tick = tickcount;
    // if (optic_sensor_current_state) {
    //   digitalWrite(2, HIGH); // NO LIGHT
    //   //myservo.write(0);
    // }
    // else {
    //   digitalWrite(2, LOW); // LIGHT
    //   //myservo.write(90);
    // }
   }

   handlePhotoInterrupter(photo_interrupter_emergency_stop_delta);

  if (Serial.available()) {
    buf = Serial.readString();
    buf.trim();


    if (buf == "init") {
      Serial.println("Init Servo");
      init_servo = true;
      is_cw = false;
      startServo(45);
    }
    if (buf == "tr") {
      Serial.println("Turn Right");
      is_cw = false;
      startServo(0);
    }
    if (buf == "trh") {
      Serial.println("Turn Right Half Speed");
      is_cw = false;
      startServo(45);
    }
    if (buf == "trhh") {
      Serial.println("Turn Right Half Half Speed");
      is_cw = false;
      startServo(67);
    }
    if (buf == "s") {
      Serial.println("Stop");
      stopServo();
    }
    if (buf == "tlhh") {
      Serial.println("Turn Left Half Half Speed");
      is_cw = true;
      startServo(112);
    }
    if (buf == "tlh") {
      Serial.println("Turn Left Half Speed");
      is_cw = true;
      startServo(135);
    }
    if (buf == "tl") {
      Serial.println("Turn Left");
      is_cw = true;
      startServo(180);
    }
    else {

      int tokenCount = 0;
      String* tokenContent;

      tokenContent = getStringToken(tokenCount, buf);

      if (tokenCount == 4) {
        if (tokenContent[0] == "turn") {
          startServo(tokenContent[1].toInt(), tokenContent[2].toInt(), tokenContent[3].toInt());
        }
      }
      else if (tokenCount == 3) {
        if (tokenContent[0] == "set") {
          if (tokenContent[1] == "esd") {
            photo_interrupter_emergency_stop_delta = tokenContent[2].toInt();
            Serial.print("Emergency Stop Delta set to: ");
            Serial.println(photo_interrupter_emergency_stop_delta);
          }
        }
      }
      
    }

  }


}

String* getStringToken(int &tokenCount, String s) 
{

  int repcount = 0;
  static String repbuf[10];

  String buf = "";

  while (s != "") {
    buf = s.substring(0, s.indexOf(" "));
    // Serial.println(buf);
    repbuf[repcount++] = buf;
    s = s.substring(buf.length() + 1);
  }



  tokenCount = repcount;
  return repbuf;

}

void handlePhotoInterrupter(int esd)
{

  if (is_active) {
    
    // Turn off after tour_countdown_index tours
    if (!init_servo && (last_tick >= (COMPLETE_TOUR * tour_countdown_index)))
    {
      stopServo();
    }

    // Turn off if resistence
    if (photo_interrupter_delta >= esd)
    {
      stopServo();
      Serial.print("Emergency stop: ");
      Serial.print(photo_interrupter_delta);
      Serial.print("/");
      Serial.println(esd);
    }

  }

  photo_interrupter_state = digitalRead(PHOTO_INTERRUPTER_PIN);
  
  if (photo_interrupter_state != photo_interrupter_laststate) {
    photo_interrupter_laststate = photo_interrupter_state;
    photo_interrupter_delta = millis() - photo_interrupter_lasttimedelta;
    photo_interrupter_lasttimedelta = photo_interrupter_delta + photo_interrupter_lasttimedelta;
    tickcount++;

    if (init_servo)
      if (photo_interrupter_delta < 15) {
        stopServo();
        init_servo = false;
        Serial.print(photo_interrupter_delta);
        Serial.println(" Initialization done.");
      }

  }
}

void stopServo(int recoilms) {

  if (recoilms != -1) {
    
    if (is_cw)
      myservo.write(70);
    else
      myservo.write(110);
    
    Serial.println("Wait");
    delay(recoilms);
    Serial.println("Wait done");
  }

  myservo.write(90);
  delay(500);
  myservo.detach();
  is_active = false;
  tickcount = 0;
  last_tick = 0;
}

void startServo(short speedPercent, short turnCount, bool cw) {

  if (!is_active) {

    tour_countdown_index = turnCount;

    photo_interrupter_delta = 0;
    photo_interrupter_lasttimedelta = millis();

    int speed = 90;

    if (cw)
      speed = map(speedPercent, 0, 100, 90, 180);
    else
      speed = map(speedPercent, 0, 100, 90, 0);

    Serial.print("Start at: ");
    Serial.println(speed);

    is_active = true;
    tickcount = 0;
    last_tick = 0;
    is_cw = cw;

    if (!myservo.attached())
      myservo.attach(SERVO_CONTROL_PIN);

    myservo.write(speed);

  }
}




void IRAM_ATTR PhotoInterrupterInterrupt() {
  tickcount++;
}

