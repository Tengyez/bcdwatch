ISP programing
* Use any Arduino (but for this example I'm using UNO)
* Requires: jumperwire, breadboard, 10uF capacitor, usb type-B for com to arduino.
* While uploading isp to arduino unplug the capacitor then put it back in after you complete the upload.
* For more information read here: https://www.hackster.io/arjun/programming-attiny85-with-arduino-uno-afb829
* However, the tutorial have a flaw where he connect the capacitor anode to GND DONOT follow it.
Attiny85 to Arduino Uno 
pin 1 -> Digital Pin 10
pin 5 -> Digital Pin 11
pin 6 -> Digital Pin 12
pin 7 -> Digital Pin 13
pin 4 -> GND
pin 8 -> 5V
* Add a 10uF capacitor between RESET and GND in arduino to avoid arduino resetting.
