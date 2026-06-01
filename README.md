# BCD-Wristwatch

A pcb wristwatch using a display of 13 led using binary coded decimal 1248.

## Description

This is a binary clock where the body is made of a circuit board which for me is a nice gimick and a conversation starter.
I specifically design it to need two shift register because if I just use the attmega the pcb surface would be bland and boring.
I also try avoid as many smd as possible except the led because I want the user to be able to debug and fix by just looking a it.
The entire goal of this project isn't to invent a better watch, for me it's a functional art piece customized to my own style.

## Getting Started

### Usage

* Press the button once to show the time, hold it 3 seconds to set the time.
* Set the time by press once to increase by one, hold 3 seconds to move to another digit.
* To charge simply plug in the usb-c. (around 5 days per charge for a 100mah battery)
* Green led mean battery ok and red means it's running low.
* You can learn to read the clock from this great tutorial: https://www.wikihow.com/Read-a-Binary-Clock

### Assembling

* The text on the silk screen corresponse to the schematic so soldering should be easy.
* Connect the lipo battery and put it under the pcb.
* Slide in the wrist strap through the pcb hook.

### Firmware

* The code is in C++ made for Attiny85.
* Program it using an ISP via arduino ide.
* !!! Important !!! This can only be done once because we blow the Attiny85 reset fuse.
```
code blocks for commands
```
### ISP programing
* Use any Arduino (but for this example I'm using UNO)
* Requires: jumperwire, breadboard, 10uF capacitor, usb type-B for com to arduino.
```
Attiny85 to Arduino Uno 
pin 1 -> Digital Pin 10
pin 5 -> Digital Pin 11
pin 6 -> Digital Pin 12
pin 7 -> Digital Pin 13
pin 4 -> GND
pin 8 -> 5V
```

## Materials

This is the BOM of the pcb, most of them are a through hole component.
For the Lipo and wriststrap you can use whatever components you want.
```
| No. | Component | Qty |
| 1 | 100nF Capacitor | 5 |
| 2 | 12pF Capacitor | 2 |
| 3 | 4.7uF Capacitor | 2 |
| 4 | JST B2B-PH-K-S Connector | 1 |
| 5 | SMD LED (13 blue, 1 red, 1 green) | 15 |
| 6 | 330Ω Resistor | 15 |
| 7 | 10kΩ Resistor | 1 |
| 8 | 20kΩ Resistor | 1 |
| 9 | 5.1kΩ Resistor | 2 |
| 10 | SMD Tactile Switch | 1 |
| 11 | 74HC595N Shift Register | 2 |
| 12 | ATtiny85-20PU | 1 |
| 13 | MCP73831T-2DCI/OT Charger | 1 |
| 14 | TYPE-C 6P USB Connector | 1 |
| 15 | 32.768kHz Crystal | 1 |
| 16 | LiPo Battery ~100mAh *(DNP)* | 1 |
| 17 | Wrist Strap 20cm *(DNP)* | 1 |
```

## PCB Pictures
<img width="1080" height="507" alt="ดีไซน์ที่ยังไม่ได้ตั้งชื่อ (1)" src="https://github.com/user-attachments/assets/0d99cc3a-6f9a-4cf3-a172-4e37ef07583a" />

## Schematic 
<img width="882" height="624" alt="Screenshot 2026-05-30 175603" src="https://github.com/user-attachments/assets/dedcd873-76aa-435e-88ce-f44eb7891cb1" />

# Zine Poster
<img width="1410" height="2000" alt="bcd watch zine poster" src="https://github.com/user-attachments/assets/ae278b3e-89d9-401f-9b2a-376a19c017e0" />


