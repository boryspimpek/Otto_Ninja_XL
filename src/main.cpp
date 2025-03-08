#include <ESP32Servo.h>
#include <PS4Controller.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "robotface.h"
#include <DFRobotDFPlayerMini.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
RobotEyes eyes(u8g2);

// Definicja pinów RX i TX dla UART
#define RXD2 16
#define TXD2 17

static const int servoLeftFootPin = 13;   // 360° continuous rotation servo
static const int servoLeftLegPin = 12;    // Standard 180° servo
static const int servoRightFootPin = 14;  // 360° continuous rotation servo 
static const int servoRightLegPin = 27;   // Standard 180° servo

HardwareSerial mySerial(1); // Użycie UART1
DFRobotDFPlayerMini myDFPlayer;

Servo servoLeftFoot;
Servo servoLeftLeg;
Servo servoRightFoot;
Servo servoRightLeg;

bool wasUpPressed = false;
bool wasSquarePressed = false;
bool wasTrianglePressed = false;
bool wasCrossPressed = false;
bool wasL1Pressed = false;
bool wasR1Pressed = false;
bool SquareStateActive = false; 
bool UpStateActive = false;
bool manualOverride = false;

bool randomEyesMode = true;  // Domyślnie tryb losowy włączony
bool wasCirclePressed = false;  // Śledzi stan przycisku Circle
int currentEyeState = 0;     // Aktualny stan oczu

bool firstPlay = true;
unsigned long previousMillis = 0;  // Przechowuje czas rozpoczęcia utworu
const long playDuration = 5000;    // Czas trwania utworu (5 sekund)
bool isPlaying = false;            // Flaga odtwarzania
int currentTrack = 1;               // Aktualnie odtwarzany utwór

const int JOYSTICK_DEADZONE = 15;
const int MAX_SERVO_SPEED = 180;

int LNP = 88;     // Left leg neutral position
int RNP = 90;     // Right leg neutral position
int LFS = 96;     // Left foot stop position
int RFS = 94;     // Right foot stop position

int LL = 120;   // Left leg position LEFT TURN
int LR = 155;   // Right leg position LEFT TURN
int RL = 25;    // Left leg position RIGHT TURN
int RR = 55;    // Right leg position RIGHT TURN

void onConnect() {
  Serial.println("Connected!");
}

void onDisConnect() {
  Serial.println("Disconnected!");
}

void moveServosSmooth(Servo &servo1, int start1, int end1, Servo &servo2, int start2, int end2, int steps, int delayTime) {
  int diff1 = end1 - start1;
  int diff2 = end2 - start2;
  for (int i = 0; i <= steps; i++) {
    int pos1 = start1 + (diff1 * i / steps);
    int pos2 = start2 + (diff2 * i / steps);
    servo1.write(pos1);
    servo2.write(pos2);
    delay(delayTime);
  }
}

void moveServoSmooth(Servo &servo1, int start1, int end1, int steps, int delayTime) {
  int diff = end1 - start1;

  for (int i = 0; i <= steps; i++) {
    int pos = start1 + (diff * i / steps);
    servo1.write(pos);
    delay(delayTime);
  }
}

void returnToNeutral() {
  int currentLeftLeg = servoLeftLeg.read();
  int currentRightLeg = servoRightLeg.read();
  
  moveServosSmooth(servoLeftLeg, currentLeftLeg, LNP, servoRightLeg, currentRightLeg, RNP, 30, 15);
  
  servoLeftFoot.write(LFS);
  servoRightFoot.write(RFS);
}

int mapLeftJoystickToSpeed(int value) {
  if (abs(value) < JOYSTICK_DEADZONE) {
    return LFS;  // Zatrzymanie dla lewego serwa
  }
  
  int mappedSpeed = map(value, -128, 127, 0, 180);
  return mappedSpeed;
}

int mapRightJoystickToSpeed(int value) {
  if (abs(value) < JOYSTICK_DEADZONE) {
    return RFS;  // Zatrzymanie dla prawego serwa
  }
  
  int mappedSpeed = map(value, -128, 127, 0, 180);
  return mappedSpeed;
}

void rightLegSwing() {
  moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), 130, servoRightLeg, servoRightLeg.read(), LR, 30, 15); // Pozycja lewa
  delay(150);
  servoLeftFoot.write(LFS + 10);  // Obrót lewej stopy serwo 360

  moveServoSmooth(servoRightLeg, LR, 60, 30, 15);
  delay(100);

  for (int i = 0; i < 2; i++) {  
    moveServoSmooth(servoRightLeg, 60, 120, 30, 15);  
    delay(100);
    moveServoSmooth(servoRightLeg, 120, 60, 30, 15);  
    delay(100);
  }

  moveServoSmooth(servoRightLeg, 60, LR, 30, 15);  
  delay(100);
}

void moonWalk() {
  for (int i = 0; i < 1; i++) { 
    moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), RL, servoRightLeg, servoRightLeg.read(), RR, 35, 20);
    moveServoSmooth(servoLeftLeg, servoLeftLeg.read(), LL, 35, 20);
    moveServoSmooth(servoLeftLeg, servoLeftLeg.read(), RL, 35, 20);
    moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), LL, servoRightLeg, servoRightLeg.read(), LR, 35, 20);
    moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), RL, servoRightLeg, servoRightLeg.read(), RR, 35, 20);
    moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), LL, servoRightLeg, servoRightLeg.read(), LR, 35, 20);
    moveServoSmooth(servoRightLeg, servoRightLeg.read(), RR, 35, 20);
    moveServoSmooth(servoRightLeg, servoRightLeg.read(), LR, 35, 20);
  }
}

void steps() {
  moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), 65, servoRightLeg, servoRightLeg.read(), 65, 30, 15);
  delay(100);
  moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), 115, servoRightLeg, servoRightLeg.read(), 115, 30, 15);
  delay(100);
  moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), 65, servoRightLeg, servoRightLeg.read(), 65, 30, 15);
  delay(100);
  moveServoSmooth(servoRightLeg, servoRightLeg.read(), 115, 30, 15);
  delay(100);
  moveServoSmooth(servoLeftLeg, servoLeftLeg.read(), 115, 30, 15);
  delay(500);
  moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), 65, servoRightLeg, servoRightLeg.read(), 65, 30, 15);
  delay(100);
  moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), 115, servoRightLeg, servoRightLeg.read(), 115, 30, 15);
  delay(100);
  moveServoSmooth(servoLeftLeg, servoRightLeg.read(), 65, 30, 15);
  delay(100);
  moveServoSmooth(servoRightLeg, servoRightLeg.read(), 65, 30, 15);
  delay(500);
  moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), 115, servoRightLeg, servoRightLeg.read(), 115, 30, 15);
  delay(100);
}

void roll() {
  moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), 180, servoRightLeg, servoRightLeg.read(), 0, 20, 15);   // pozycja jazdy
}

void setup() {
  servoLeftFoot.attach(servoLeftFootPin);
  servoRightFoot.attach(servoRightFootPin);
  
  servoLeftLeg.attach(servoLeftLegPin, 500, 2400);
  servoRightLeg.attach(servoRightLegPin, 600, 2500);

  servoLeftFoot.write(LFS);    // Stop - no rotation
  servoRightFoot.write(RFS);   // Stop - no rotation
  
  servoLeftLeg.write(LNP);     // Position in degrees
  servoRightLeg.write(RNP);   // Position in degrees

  Serial.begin(115200);
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);
  PS4.begin();
  Serial.println("Ready.");

  Wire.begin();
  eyes.begin();

  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
    randomSeed(esp_random()); // Inicjalizacja generatora liczb losowych
    
    if (!myDFPlayer.begin(mySerial)) {
        Serial.println("Błąd komunikacji z DFPlayer Mini!");
        while (true);
    }
    
    myDFPlayer.volume(30); // Ustawienie głośności (0-30)
  delay(300);
}

void loop() {
  if (!isPlaying) {
    if (firstPlay) {
      // Odtwarzanie pierwszego utworu
      Serial.println("Odtwarzanie pliku 1...");
      myDFPlayer.play(1);
      currentTrack = 1; // Zapisujemy numer utworu
      firstPlay = false;
    } else {
      // Losowe odtwarzanie utworów od 2 do 8
      currentTrack = random(2, 9);
      Serial.print("Odtwarzanie pliku ");
      Serial.println(currentTrack);
      myDFPlayer.play(currentTrack);
    }

    // Rozpoczęcie śledzenia czasu
    previousMillis = millis();
    isPlaying = true; // Flaga odtwarzania aktywna
  }

  // Sprawdzamy, czy minęło 5000 ms (5 sekundy) od rozpoczęcia odtwarzania
  if (isPlaying && millis() - previousMillis >= playDuration) {
    isPlaying = false; // Kończymy odtwarzanie
  }  
  if (PS4.isConnected()) {
    // Sterowanie serwami 360 za pomocą joysticków (gdy nie ma manualOverride)
    if (!manualOverride) {
      int leftFootSpeed = mapLeftJoystickToSpeed(PS4.LStickY());      // sewrwo 360
      int rightFootSpeed = mapRightJoystickToSpeed(PS4.RStickY());    // serwo 360

      servoLeftFoot.write(leftFootSpeed);
      // Poprawka dla prawego serwa
      if (rightFootSpeed == RFS) {  
        servoRightFoot.write(RFS);  // Zatrzymanie w miejscu RFS
      } else {
        servoRightFoot.write(180 - rightFootSpeed);
      }
    }

    if (PS4.Up()) {
      if (!wasUpPressed) {
        wasUpPressed = true;
        if (!UpStateActive) {
          roll();
          UpStateActive = true;
        } else {
          returnToNeutral();
          UpStateActive = false;
        }
      }
    } else {
      wasUpPressed = false;
    }

    // Square button handling
    if (PS4.Square()) {
      if (!wasSquarePressed) {
        manualOverride = true;  // Blokowanie joysticka
        steps();
        wasSquarePressed = true;
      }
    } else if (wasSquarePressed) {
      returnToNeutral();  // Powrót do neutralnej pozycji po zakończeniu ruchów
      manualOverride = false; // Odblokowanie joysticka
      wasSquarePressed = false;
    }

    // L1 button handling
    if (PS4.L1()) {
      if (!wasL1Pressed) {
        manualOverride = true;  // Blokowanie joysticka
        moveServosSmooth(servoLeftLeg, servoLeftLeg.read(), LL, servoRightLeg, servoRightLeg.read(), LR, 35, 15); // Pozycja idąca lewa
        delay(150);
        servoLeftFoot.write(LFS + 15); // Obrót w prawo, ustawienie prędkości serwa 360
        wasL1Pressed = true;
      }
    } else if (wasL1Pressed) {
      servoLeftFoot.write(LFS); // Natychmiastowe zatrzymanie obrotu
      delay(150);
      returnToNeutral();
      manualOverride = false; // Odblokowanie joysticka
      wasL1Pressed = false;
    }

    // R1 button handling
    if (PS4.R1()) {
      if (!wasR1Pressed) {
        manualOverride = true;  // Blokowanie joysticka
        moveServosSmooth(servoRightLeg, servoRightLeg.read(), RR, servoLeftLeg, servoLeftLeg.read(), RL, 30, 15);  // Pozycja idąca prawo
        delay(150);
        servoRightFoot.write(RFS - 15); // Obrót w lewo, ustawienie prędkości serwa 360
        wasR1Pressed = true;
      }
    } else if (wasR1Pressed) {
      servoRightFoot.write(RFS); // Natychmiastowe zatrzymanie obrotu
      delay(150);
      returnToNeutral();
      manualOverride = false; // Odblokowanie joysticka
      wasR1Pressed = false;
    }

    // Triangle button handling
    if (PS4.Triangle()) {
      if (!wasTrianglePressed) {
        manualOverride = true;  // Blokowanie joysticka
        rightLegSwing();
        wasTrianglePressed = true;
      }
    } else if (wasTrianglePressed) {
      returnToNeutral();  // Powrót do neutralnej pozycji po zakończeniu ruchów
      manualOverride = false; // Odblokowanie joysticka
      wasTrianglePressed = false;
    }

    // Cross button handling
    if (PS4.Cross()) {
      if (!wasCrossPressed) {
        manualOverride = true;
        moonWalk();
        wasCrossPressed = true;
      }
    } else if (wasCrossPressed) {
      returnToNeutral();  // Powrót do neutralnej pozycji po zakończeniu ruchów
      manualOverride = false; // Odblokowanie joysticka
      wasCrossPressed = false;
    }

    // Circle button handling
    if (PS4.Circle()) {
      if (!wasCirclePressed) {
        wasCirclePressed = true;
        randomEyesMode = false;  // Wyłącz tryb losowy podczas ręcznego przewijania

        // Przejdź do kolejnego stanu oczu
        currentEyeState = (currentEyeState + 1) % 12;  // 12 to liczba dostępnych stanów oczu
        
        // Jeśli przeszliśmy przez wszystkie stany, włącz tryb losowy
        if (currentEyeState == 0) {
          randomEyesMode = true;
        }
        
        // Zastosuj wybrany stan oczu
        switch (currentEyeState) {
          case 0:
            eyes.normal();
            break;
          case 1:
            eyes.angry();
            break;
          case 2:
            eyes.happy();
            break;
          case 3:
            eyes.sad();
            break;
          case 4:
            eyes.suspicious();
            break;
          case 5:
            eyes.upset();
            break;
          case 6:
            eyes.blink();
            break;
          case 7:
            eyes.left();
            break;
          case 8:
            eyes.right();
            break;
          case 9:
            eyes.name();
            break;
          case 10:
            eyes.heart();
            break;
          case 11:
            eyes.error();
            break;
        }
      }
    } else {
      wasCirclePressed = false;
    }
  }

  static unsigned long lastActionTime = 0;
  const unsigned long actionInterval = 1000; // 2 sekundy między akcjami

  // Tylko wyświetlaj losowe oczy, jeśli tryb losowy jest włączony
  if (randomEyesMode && millis() - lastActionTime >= actionInterval) {
    int randomNumber = random(1, 100);
    
    if (randomNumber <= 25) {
      eyes.normal();
    } else if (randomNumber <= 50) {
      eyes.blink();
    } else {
      switch (random(1, 13)) { 
        case 1:
          eyes.left();
          break;
        case 2:
          eyes.right();
          break;
        case 3:
          eyes.sad();
          break;
        case 4:
          eyes.happy();
          break;
        case 5:
          eyes.angry();
          break;
        case 6:
          eyes.upset();
          break;
        case 7:
          eyes.name();
          break;
        case 8:
          eyes.suspicious();
          break;
        case 9:
          eyes.blink();
          break;
        case 10:
          eyes.heart();
          break;
        case 11:
          eyes.error();
          break;    
        default:
          eyes.normal();
          break;
      }
    }
    
    lastActionTime = millis();
  }
}