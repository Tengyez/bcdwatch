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
