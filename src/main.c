#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/power.h>
#include <avr/sleep.h>

#define BUTTON_PIN PB3
#define LED_PIN PB0
#define BUTTON_PRESSED_PIN PB5
#define LONG_PRESS_DURATION 200U
#define LIMIT_WAIT 200U
#define MAX_PWM 255U
#define DEFAULT_PWM (MAX_PWM / 2) -1
volatile uint16_t press_duration = 0;
volatile uint8_t button_pressed = 0;
volatile uint8_t pwm_value = 0;
volatile uint8_t old_pwm_value = DEFAULT_PWM; // Set initial value to something in the middle?
volatile int8_t change = 1;                   // This will change to a negative value when dimming
volatile uint8_t wait_left = 0;               // Left to wait at the ends of the range
volatile uint8_t waiting = 0;                 // Are we waiting?

void setup()
{
    // Set LED and debug pins as outputs
    DDRB |= (1 << LED_PIN) | (1 << BUTTON_PRESSED_PIN);

    // Set button pin as input
    DDRB &= ~(1 << BUTTON_PIN);
    PORTB &= (1 << BUTTON_PIN); // Explicitly set resistor

    // Set up Timer1 for PWM on PB0
    TCCR1 |= (1 << PWM1A);      // Enable PWM mode for Timer1
    TCCR1 |= (1 << COM1A1);     // Non-inverting mode on OC1A (PB0)
    TCCR1 |= (1 << CS11) | (1 << CS10); // Prescaler of 64
    OCR1A = pwm_value;          // Initial PWM duty cycle

    // Set up Timer0 for timing interrupts
    TCCR0A |= (1 << WGM01);     // CTC mode
    TCCR0B |= (1 << CS02) | (1 << CS00); // Prescaler of 1024
    OCR0A = 200;            // Compare match value for approximately 10ms interrupt interval
    TIMSK |= (1 << OCIE0A);     // Enable Timer/Counter0 Output Compare Match A interrupt

    // Enable global interrupts
    sei();
}

void loop()
{
    if (!(PINB & (1 << BUTTON_PIN)))
    {
        // Button pressed
        if (!button_pressed)
        {
            button_pressed = 1;
            press_duration = 0;
            PORTB |= (1 << BUTTON_PRESSED_PIN); // Indicate button pressed
        }
        press_duration++;
        if (press_duration >= LONG_PRESS_DURATION)
        {
            // Long press
            if (waiting)
            {
                // Waiting either at the top limit or bottom limit
                if (!wait_left)
                {
                    // We are done waiting, reverse direction
                    change = -change;
                    pwm_value += change;
                    waiting = 0;
                }
                else
                {
                    wait_left--;
                }
            }
            else
            {
                if (pwm_value + change >= MAX_PWM)
                {
                    // We seem to be on the top limit
                    pwm_value = MAX_PWM;
                    waiting = 1;
                    wait_left = LIMIT_WAIT;
                }
                else if (pwm_value + change <= 0)
                {
                    // We seem to be on the bottom limit
                    pwm_value = 0;
                    waiting = 1;
                    wait_left = LIMIT_WAIT;
                }
                else
                {
                    pwm_value += change;
                }
                OCR1A = pwm_value; // Update PWM duty cycle
            }
        }
    }
    else
    {
        // If it was pressed, it the button is now released
        if (button_pressed)
        {
            button_pressed = 0;
            
            PORTB &= ~(1 << BUTTON_PRESSED_PIN); // Indicate button released

            if (press_duration < LONG_PRESS_DURATION)
            {   
                // So we want on or off
                if (pwm_value > 0)
                {
                    old_pwm_value = pwm_value;
                    pwm_value = 0;
                }
                else
                {
                    pwm_value = old_pwm_value;
                }
                
                OCR1A = pwm_value; // Update PWM duty cycle
            }
            else 
            {
                if (waiting) {
                    // If we were waiting at an extreme, here is for not waiting
                    waiting = 0;
                    wait_left = 0;
                }   
                // Change direction for the next long press
                change = -change;
            } 
            press_duration = 0; // Reset duration
        }
    }
}

ISR(TIMER0_COMPA_vect)
{
    loop();
}

int main(void)
{
    setup();
    while (1)
    {
        // Main loop code
    }
    return 0;
}
