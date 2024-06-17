#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h> 
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_io.h>
#include <simavr/sim_interrupts.h>
#include <simavr/avr_ioport.h>
#include <simavr/sim_cycle_timers.h>

#include <SDL2/SDL.h>




#define OCR0A_ADDR 108 // OCR0A address for ATtiny85

avr_t *avr;
char *sim_log_prefix;

avr_irq_t *button_irq;
avr_irq_t *button_pressed_irq;

bool buttondown = false;

SDL_Window *win;
// Main loop
SDL_Event e;

bool button_pressed = false;
 pthread_t thread_id;

volatile bool keep_running = true;

void init_simavr(void)
{
    sim_log_prefix = "simavr";
    printf("Setup\n");

    // Initialize the AVR simulator
    avr = avr_make_mcu_by_name("attiny85");
    if (!avr)
    {
        printf("AVR '%s' not known", "attiny85\n");
        exit(1);
    }

    avr->frequency = 1000000L;
    printf("Avr_init\n");
    avr_init(avr);
    elf_firmware_t f;
    memset(&f, 0, sizeof(f));
    printf("Read firmware from %s", FIRMWARE_PATH);

    if (elf_read_firmware(FIRMWARE_PATH, &f) < 0)
    {
        printf("Failed to read firmware\n");
        exit(1);
    }
    printf("Load firmware..\n");
    avr_load_firmware(avr, &f);

    printf("Firmware loaded successfully.\n");

    avr_run(avr);

    button_irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 3);
    button_pressed_irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5);
    
    if (!button_irq || !button_pressed_irq)
    {
        printf("Failed to get IRQ\n");
        exit(1);
    }

    avr_raise_irq(button_irq, 1);

    printf("AVR simulator setup complete.\n");
}

void quit_client(int signal)
{
    keep_running = false;
    pthread_join(thread_id, NULL);
    

    avr_terminate(avr);
    printf("AVR simulator terminated.\n");

    // Cleanup and quit SDL
    printf("Terminated SDL.\n");
    SDL_DestroyWindow(win);
    SDL_Quit();
    printf("SDL terminated. Quitting.\n");
    exit(signal);
}

void run_avr_for_cycles(uint32_t cycles)
{
    for (uint32_t i = 0; i < cycles; ++i)
    {
        avr_run(avr);
    }
}

uint16_t read_press_duration(avr_t *avr)
{
    
    uint16_t result = 0;
    memcpy(&result, avr->data + 101, sizeof(result));
    return result;
}

uint8_t read_ocr0a(avr_t *avr)
{
    return avr->data[OCR0A_ADDR];
}

void print_mem(void)
{
    int16_t counter = 0;
    while (counter < 254)
    {
        if (avr->data[counter] > 0)
        {
            printf("(%i) %hu ", counter, avr->data[counter]);
        }
        else
        {
            printf("%hu ", avr->data[counter]);
        }

        counter++;
    }
    printf("End.\n");
}
void report()
{
    printf("Button_state: %s, PWM : %hu, duration: %u ticks\n", button_pressed_irq->value ? "Down" : "Up", read_ocr0a(avr),  read_press_duration(avr));
    // Note the apparent necessity to end outputs with a newline for them to flush.
}
void init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        exit(1);
    }

    // Create a window
    win = SDL_CreateWindow("Key Press Recorder", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (win == NULL)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
}

void read_button()
{

    // Poll events
    while (SDL_PollEvent(&e))
    {
        // Check if the event is a quit event
        if (e.type == SDL_QUIT)
        {
            quit_client(0);
        }
        // Check if the event is a key down event
        if (e.key.keysym.sym == 109)
        {
            if (e.type == SDL_KEYDOWN) {
                print_mem();
            }
        } else
        if (e.type == SDL_KEYDOWN && !buttondown)
        {
            avr_raise_irq(button_irq, 0);
            printf("The key you pressed was %s, %i\n", SDL_GetKeyName(e.key.keysym.sym), e.key.keysym.sym);
            buttondown = true;
        }
        else if (e.type == SDL_KEYUP && buttondown)
        {
            avr_raise_irq(button_irq, 1);
            printf("The key let go was %s\n", SDL_GetKeyName(e.key.keysym.sym));

            buttondown = false;
        }
    }
}

// Background thread function
void* avr_runner(void* arg) {
    while (keep_running) {
        run_avr_for_cycles(100000);
        //usleep(10);  // Optional small delay to avoid busy-waiting
    }
    return NULL;
}


#ifndef __APPLE__
int main(int argc, char** argv)
#else
int main(void)
#endif
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    printf("Simavr initiating\n");

    init_simavr();
    printf("Simavr initiated\n");


     // Create the background thread
    if (pthread_create(&thread_id, NULL, avr_runner, NULL) != 0) {
        perror("Failed to create AVR runner thread");
        return 1;
    }

    init_sdl();
    printf("SDL initiated\n");
    while (1)
    {
        report();
        SDL_Delay(402);
        read_button();
    }
}
