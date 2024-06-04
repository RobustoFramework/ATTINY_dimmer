#include <avr/io.h>
#include <avr/interrupt.h>

#define BUTTON_PIN PB3
#define LED_PIN PB1
#define DEBUG_PIN PB2
#define BUTTON_PRESSED_PIN PB5
#define MAX_PRESS_DURATION 1000
#define MAX_PWM 255

volatile uint16_t press_duration = 0;
volatile uint8_t button_pressed = 0;
volatile uint8_t pwm_value = 0;
volatile uint8_t test_value = 99;

void setup() {
    // Set LED and debug pins as outputs
    DDRB |= (1 << LED_PIN) | (1 << DEBUG_PIN) | (1 << BUTTON_PRESSED_PIN);

    // Set button pin as input
    DDRB &= ~(1 << BUTTON_PIN);
    PORTB &= ~(1 << BUTTON_PIN); // Explicitly set resistor

    // Set up Timer0 for PWM
    TCCR0A |= (1 << WGM01) | (1 << WGM00); // Fast PWM mode
    TCCR0A |= (1 << COM0A1); // Clear OC0A on compare match
    TCCR0B |= (1 << CS01) | (1 << CS00); // Prescaler 64
    OCR0A = pwm_value; // Initial PWM duty cycle

    // Enable global interrupts
    sei();
}

void loop() {
    if (PINB & (1 << BUTTON_PIN)) {
        // Button pressed
        if (!button_pressed) {
            button_pressed = 1;
            press_duration = 0;
            PORTB |= (1 << BUTTON_PRESSED_PIN); // Indicate button pressed
            PORTB |= (1 << DEBUG_PIN); // Set DEBUG pin high
        }
        press_duration++;
        if (press_duration > MAX_PRESS_DURATION) {
            press_duration = 0;
        }
    } else {
        // Button released
        if (button_pressed) {
            button_pressed = 0;
            PORTB &= ~(1 << BUTTON_PRESSED_PIN); // Indicate button released
            PORTB &= ~(1 << DEBUG_PIN); // Set DEBUG pin low

            if (press_duration < 100) { // Short press
                PORTB ^= (1 << LED_PIN); // Toggle LED
            } else if (press_duration >= 100) { // Long press
                pwm_value += 10;
                if (pwm_value > MAX_PWM) {
                    pwm_value = 2; // Reset PWM value if it exceeds max
                }
                OCR0A = pwm_value; // Update PWM duty cycle
            }
            press_duration = 0; // Reset duration
        }
    }
}

int main(void) {
    setup();

    while (1) {
        loop();
    }

    return 0;
}