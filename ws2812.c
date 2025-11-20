/*
 * File:   ws2812.c
 * Author: Manohar Thokala
 *
 * Created on November 19, 2025, 11:51 AM
 */




#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define F_CPU 20000000UL   //cpu clock frequency for delay.h
#define LED_PIN 5  // Neo pixel led pin, change it as needed.

/** 
 * @brief Timing Macros (for ATtiny @ 20 MHz)
 * Each NOP = 1 CPU cycle = 50 ns at 20 MHz.
 * WS2812 timing requirements:
 *   "0" bit: HIGH ~350 ns, LOW remainder of 1.25 탎
 *   "1" bit: HIGH ~700 ns, LOW remainder of 1.25 탎
 */
// Macros for exact timing at 20 MHz
#define NOP1  asm("nop")
#define NOP2  NOP1;NOP1
#define NOP4  NOP2;NOP2
#define NOP8  NOP4;NOP4

// HIGH time for logical '0'  (~350 ns)
#define T0H   NOP4;NOP2;NOP1         // 7 cycles ? 350ns
// HIGH time for logical '1'  (~700 ns)
#define T1H   NOP8;NOP4;NOP2         // 14 cycles ? 700ns


/**
 * @Function ws2812_send_byte
 * @Purpose  Send a single byte (8 bits) to WS2812 in MSB-first order
 * @Input    byte ? 8-bit data (R/G/B component)
 * @Notes   
 *     WS2812 expects GRB format.
 *     Bit-banging must meet strict nanosecond timing.
 *     After sending each bit, total period must be ~1.25 탎.
 */
void ws2812_send_byte(uint8_t byte)
{
    for(uint8_t i = 0; i < 8; i++) // Check MSB ? Determine if bit is '1'
    {
        if((byte & (1 << 7)))
        {
           
            // Transmit binary '1'
            PORTA.OUTSET = (1 << LED_PIN);  // Set pin HIGH
            T1H;                             // Hold HIGH for ~700 ns
            PORTA.OUTCLR = (1 << LED_PIN);  // Pull LOW
        }
        else
        {
            // Transmit binary '0'
            PORTA.OUTSET = (1 << LED_PIN);  // Set pin HIGH
            T0H;                             // Hold HIGH for ~350 ns
            PORTA.OUTCLR = (1 << LED_PIN);  // Pull LOW
        }
        byte <<= 1;   // Shift to next bit
        // Maintain WS2812's total bit time (1.25 탎 per bit)
        // Here we add ~1 탎 LOW to stabilize and remain safe.
        _delay_us(1);
    }
}

/**
 * @Function ws2812_send_rgb
 * @Purpose  Send color data for one WS2812 LED
 * @Inputs   r = red value   (0?255)
 *           g = green value (0?255)
 *           b = blue value  (0?255)
 * @Notes   
 *    WS2812 uses GRB order (not RGB)
 */
void ws2812_send_rgb(uint8_t g, uint8_t r, uint8_t b)
{
    ws2812_send_byte(g);   // First send GREEN
    ws2812_send_byte(r);   // Then RED
    ws2812_send_byte(b);   // Then BLUE
}

/**
 * @Function ws2812_show
 * @Purpose  Generate reset/latch pulse to update LED color
 * @Notes   
 *    WS2812 latches data when line is LOW for ? 50 탎.
 *    60 탎 used for safety margin.
 */
void ws2812_update()
{
    _delay_us(60);   // Reset/latch time
}


/**
 * @Function ws2812_init
 * @Purpose  Configure WS2812 data pin as output
 * @Notes   Pin must start LOW when idle.
 *    
 */
void ws2812_init()
{
    PORTA.DIRSET = (1 << LED_PIN);  // Set PA5 as output
    PORTA.OUTCLR = (1 << LED_PIN);  // Ensure idle LOW
}
