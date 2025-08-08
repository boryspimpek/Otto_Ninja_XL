# ESP32 PS4 Robot Controller

A robot control program for **ESP32** using a **PS4 controller**, OLED display (SH1106), DFPlayer Mini for sound effects, and servo motors for movement.  
The robot can perform various dance moves, display animated eyes, and play random audio tracks.

## Features
- **PS4 Controller Support** – Bluetooth control of robot actions.
- **Servo Motor Control** – Smooth movements for legs and continuous rotation feet.
- **Dance Moves** – Moonwalk, steps, rolls, swings.
- **Animated Eyes** – Different expressions shown on an OLED display.
- **DFPlayer Mini Integration** – Plays sound effects and music tracks.
- **Random Eye Mode** – Automatic idle animations when not in manual mode.

## Hardware
- ESP32
- 2× 180° servo motors (legs)
- 2× 360° continuous rotation servo motors (feet)
- SH1106 OLED display
- DFPlayer Mini + speaker
- PS4 controller (Bluetooth)
- Wires, power supply, etc.

## Controls
| Button         | Action                       |
|----------------|------------------------------|
| Left Stick     | Left foot movement           |
| Right Stick    | Right foot movement          |
| **Up**         | Toggle roll mode             |
| **Square**     | Steps movement               |
| **Triangle**   | Right leg swing               |
| **Cross**      | Moonwalk                      |
| **L1**         | Left turn                     |
| **R1**         | Right turn                    |
| **Circle**     | Change eye expression         |

## Setup
1. Install the following Arduino libraries:
   - `ESP32Servo`
   - `PS4Controller`
   - `U8g2`
   - `DFRobotDFPlayerMini`
2. Connect hardware according to the pin definitions in the code.
3. Upload the code to your ESP32.
4. Pair your PS4 controller with the ESP32.
5. Enjoy controlling your robot!

## Pinout (Default)
| Component         | Pin |
|-------------------|-----|
| Left Foot Servo   | 13  |
| Left Leg Servo    | 12  |
| Right Foot Servo  | 14  |
| Right Leg Servo   | 27  |
| DFPlayer RX       | 16  |
| DFPlayer TX       | 17  |

## License
MIT License
