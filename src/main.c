#include <avr/io.h>
#include <avr/interrupt.h>

#define BUTTON_PIN PB3
#define LED_PIN PB1
#define DEBUG_PIN PB2
#define BUTTON_PRESSED_PIN PB5
#define LONG_PRESS_DURATION 100
#define LIMIT_WAIT 5
#define MAX_PWM 255

volatile uint16_t press_duration = 0;
volatile uint8_t button_pressed = 0;
volatile int8_t pwm_value = 0;
volatile uint8_t old_pwm_value = MAX_PWM / 2; // Set initial value to something in the middle?
volatile int8_t change = 5; // This will change to a negative value when dimming
volatile uint8_t wait_left = 0; // Left to wait at the ends of the range
volatile uint8_t waiting = 0; // Are we waiting?

void setup()
{
    // Set LED and debug pins as outputs
    DDRB |= (1 << LED_PIN) | (1 << DEBUG_PIN) | (1 << BUTTON_PRESSED_PIN);

    // Set button pin as input
    DDRB &= ~(1 << BUTTON_PIN);
    PORTB &= ~(1 << BUTTON_PIN); // Explicitly set resistor

    // Set up Timer0 for PWM
    TCCR0A |= (1 << WGM01) | (1 << WGM00); // Fast PWM mode
    TCCR0A |= (1 << COM0A1);               // Clear OC0A on compare match
    TCCR0B |= (1 << CS01) | (1 << CS00);   // Prescaler 64
    OCR0A = pwm_value;                     // Initial PWM duty cycle

    // Enable global interrupts
    sei();
}

void loop()
{
    if (PINB & (1 << BUTTON_PIN))
    {
        // Button pressed
        if (!button_pressed)
        {
            button_pressed = 1;
            press_duration = 0;
            PORTB |= (1 << BUTTON_PRESSED_PIN); // Indicate button pressed
            PORTB |= (1 << DEBUG_PIN);          // Set DEBUG pin high
        }
        press_duration++;
        if (press_duration >= LONG_PRESS_DURATION)
        { 
            // Long press
            if (waiting) {
                if (!wait_left) {
                    // We are done waiting, reverse direction
                    change = -change;
                    pwm_value += change;
                    waiting = 0;
                }
                else {
                    wait_left--;
                }
            } else {

                if (pwm_value + change > MAX_PWM)
                {
                    // We seem to be on the top limit
                    pwm_value = MAX_PWM;
                    waiting = 1;
                    wait_left = LIMIT_WAIT;
                    
                } else
                if (pwm_value + change < 0)
                {
                    // We seem to be on the bottom limit
                    pwm_value = 0;
                    waiting = 1;
                    wait_left = LIMIT_WAIT;
                } else {
                    pwm_value += change;
                }
                OCR0A = pwm_value; // Update PWM duty cycle
            }
                  
        }
    }
    else
    {
        // If it was pressed, it the button is now released
        if (button_pressed)
        {
            button_pressed = 0;
            wait_left = 0;
            PORTB &= ~(1 << BUTTON_PRESSED_PIN); // Indicate button released
            PORTB &= ~(1 << DEBUG_PIN);          // Set DEBUG pin low

            if (press_duration < LONG_PRESS_DURATION)
            {                            // Short press
                PORTB ^= (1 << LED_PIN); // Toggle LED
                // So we want on or off
                if (pwm_value > 0)
                {
                    old_pwm_value = pwm_value;
                    pwm_value = 0;
                } else {
                    pwm_value = old_pwm_value;
                }
                OCR0A = pwm_value; // Update PWM duty cycle
            } else {
                // Change direction for the next log press
                change = -change;
            }
            press_duration = 0; // Reset duration
        }
    }
}

int main(void)
{
    setup();
    while (1)
    {
        loop();
    }

    return 0;
}