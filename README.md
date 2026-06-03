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
* Please flash the attiny before you solder it.
* Solder all the smd first, then add the resistor and capacitor based on the silk screen.
* Hold down those with tape then solder it. Finally solder the attiny, shift-register and power components.
* Connect the lipo battery and put it under the pcb.
* Slide in the wrist strap through the pcb hook.
  
### Firmware

* The code is in C++ made for Attiny85.
* Program it using an ISP via arduino ide.
* !!! Important !!! This can only be done once because we blow the Attiny85 reset fuse.
```
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/eeprom.h>
#include <stdbool.h>

#define DS_PIN  PB0   
#define SH_PIN  PB1   
#define ST_PIN  PB2   

#define EE_MAGIC        ((uint8_t*)0)
#define EE_HOUR         ((uint8_t*)1)
#define EE_MINUTE       ((uint8_t*)2)
#define MAGIC_VAL       0xA5

__attribute__((section(".noinit"))) uint16_t ram_last_press_time;
__attribute__((section(".noinit"))) bool     ram_setting_mode;
__attribute__((section(".noinit"))) uint8_t  ram_current_digit;

volatile uint8_t  g_hour           = 12;
volatile uint8_t  g_minute         = 0;
volatile uint8_t  g_second         = 0;
volatile uint16_t g_ms_counter     = 0;
volatile uint16_t g_blink_counter  = 0;
volatile bool     g_blink_state    = false; 
volatile uint8_t  g_fraction_accum = 0;
volatile uint8_t  g_timeout_seconds = 0;

uint16_t timeTo16Bits(uint8_t hour, uint8_t minute) {
    uint8_t ht = hour   / 10; 
    uint8_t ho = hour   % 10; 
    uint8_t mt = minute / 10; 
    uint8_t mo = minute % 10; 

    uint8_t u7_data = 0; 
    uint8_t u5_data = 0; 

    u7_data |= (ht & 0x03);
    u7_data |= ((ho & 0x0F) << 2);

    u5_data |= (mt & 0x07);
    u5_data |= ((mo & 0x0F) << 3);

    return ((uint16_t)u7_data << 8) | u5_data;
}

void shiftOut16(uint16_t data) {
    PORTB &= ~(1 << ST_PIN); 

    for (int8_t i = 15; i >= 0; i--) {
        if (data & ((uint16_t)1 << i))
            PORTB |=  (1 << DS_PIN);
        else
            PORTB &= ~(1 << DS_PIN);

        PORTB |=  (1 << SH_PIN); 
        PORTB &= ~(1 << SH_PIN);
    }

    PORTB |=  (1 << ST_PIN); 
    PORTB &= ~(1 << ST_PIN);
}

ISR(TIMER0_COMPA_vect) {
    uint8_t ticks = 1;
    
    if (++g_fraction_accum >= 24) {
        g_fraction_accum = 0;
        ticks = 2; 
    }

    g_ms_counter += ticks;
    if (g_ms_counter >= 1000) {
        g_ms_counter -= 1000;
        
        if (++g_second >= 60) {
            g_second = 0;
            if (++g_minute >= 60) {
                g_minute = 0;
                if (++g_hour >= 24) {
                    g_hour = 0;
                }
            }
        }

        if (ram_setting_mode) {
            if (++g_timeout_seconds >= 3) {
                g_timeout_seconds = 0;
                ram_setting_mode = false;
                ram_current_digit = 0;
                eeprom_update_byte(EE_MAGIC,  MAGIC_VAL);
                eeprom_update_byte(EE_HOUR,   g_hour);
                eeprom_update_byte(EE_MINUTE, g_minute);
            }
        }
    }

    if (++g_blink_counter >= 250) { 
        g_blink_state = !g_blink_state;
        g_blink_counter = 0;
    }
}

void timer0_init() {
    TCCR0A = (1 << WGM01);               
    TCCR0B = (1 << CS00);                
    OCR0A  = 31;                         
    TIMSK |= (1 << OCIE0A);              
}

void setup() {
    DDRB |= (1 << DS_PIN) | (1 << SH_PIN) | (1 << ST_PIN);
    
    uint8_t mcusr_bak = MCUSR;
    MCUSR = 0;

    if (eeprom_read_byte(EE_MAGIC) == MAGIC_VAL) {
        g_hour   = eeprom_read_byte(EE_HOUR);
        g_minute = eeprom_read_byte(EE_MINUTE);
    }

    if (mcusr_bak & (1 << EXTRF)) {
        uint16_t current_time_sec = (g_hour * 3600) + (g_minute * 60) + g_second;
        uint16_t time_diff = current_time_sec - ram_last_press_time;
        ram_last_press_time = current_time_sec;

        if (!ram_setting_mode) {
            if (time_diff >= 3) {
                ram_setting_mode = true;
                ram_current_digit = 0; 
            }
        } else {
            g_timeout_seconds = 0; 
            
            if (time_diff >= 3) {
                if (ram_current_digit == 0) {
                    ram_current_digit = 2;
                } else if (ram_current_digit == 2) {
                    ram_current_digit = 3;
                } else if (ram_current_digit == 3) {
                    ram_current_digit = 0;
                }
            } else {
                uint8_t ht = g_hour   / 10;
                uint8_t ho = g_hour   % 10;
                uint8_t mt = g_minute / 10;
                uint8_t mo = g_minute % 10;

                switch (ram_current_digit) {
                    case 0: 
                        ht = (ht + 1) % 3; 
                        if (ht == 2 && ho > 3) ho = 3; 
                        break;
                    case 2: 
                        mt = (mt + 1) % 6; 
                        break;
                    case 3: 
                        mo = (mo + 1) % 10; 
                        break;
                }
                g_hour = ht * 10 + ho;
                g_minute = mt * 10 + mo;
            }
        }
    } else {
        ram_setting_mode = false;
        ram_current_digit = 0;
        ram_last_press_time = 0;
    }

    timer0_init();
    sei();
}

void loop() {
    uint8_t h, m;
    bool blink;
    
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        h = g_hour;
        m = g_minute;
        blink = g_blink_state;
    }

    uint16_t display_bits = timeTo16Bits(h, m);

    if (ram_setting_mode && !blink) {
        uint16_t mask = 0;
        switch (ram_current_digit) {
            case 0: mask = 0x0300; break;  
            case 2: mask = 0x0007; break;  
            case 3: mask = 0x00F8; break;  
        }
        display_bits &= ~mask;
    }

    shiftOut16(display_bits);
    
    for (volatile uint8_t i = 0; i < 40; i++);
}
```
### ISP programing
* Use any Arduino (but for this example I'm using UNO)
* Requires: jumperwire, breadboard, 10uF capacitor, usb type-B for com to arduino.
* While uploading isp to arduino unplug the capacitor then put it back in after you complete the upload.
* For more information read here: https://www.hackster.io/arjun/programming-attiny85-with-arduino-uno-afb829
* However, the tutorial have a flaw where he connect the capacitor anode to GND DONOT follow it.
```
Attiny85 to Arduino Uno 
pin 1 -> Digital Pin 10
pin 5 -> Digital Pin 11
pin 6 -> Digital Pin 12
pin 7 -> Digital Pin 13
pin 4 -> GND
pin 8 -> 5V
* Add a 10uF capacitor between RESET and GND in arduino to avoid arduino resetting.
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


