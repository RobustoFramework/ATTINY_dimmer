#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_io.h>
#include <simavr/sim_interrupts.h>
#include <simavr/avr_ioport.h>
#include <simavr/sim_vcd_file.h>
#include <simavr/sim_cycle_timers.h>



#define ATTINY95_IO_OFFSET 0x22
#define OCR0A_ADDR ATTINY95_IO_OFFSET + 0x27 // OCR0A address for ATtiny85

avr_t *avr;
char *sim_log_prefix;  
avr_vcd_t vcd_file;

avr_irq_t *button_irq;
avr_irq_t *button_pressed_irq;

bool button_pressed = false;

void init_ui(void) {
    // NCurses

    if (initscr() == NULL) {
            printf("Error initializing ncurses.");
            exit(-1);
        }              // Initialize ncurses
    printw("1");
    cbreak();             // Disable line buffering
    printw("2");
    noecho();             // Don't echo key presses
    printw("3");
    nodelay(stdscr, TRUE); // Non-blocking input
    printw("4");
    keypad(stdscr, TRUE); // Enable special keys

}

void init_simavr(void) { 
    sim_log_prefix = "simavr";
    printw("Setup");


    // Initialize the AVR simulator
    avr = avr_make_mcu_by_name("attiny85");
    if (!avr) {
        printw("AVR '%s' not known", "attiny85");
        exit(1);
    }


    avr->frequency = 1000000L;
    printw("Avr_init");
    avr_init(avr);
    elf_firmware_t f;
    memset(&f, 0, sizeof(f));
    printw("Read firmware from %s", FIRMWARE_PATH);

    if (elf_read_firmware(FIRMWARE_PATH, &f) < 0) {
        printw("Failed to read firmware");
        exit(1);
    }
    printw("Load firmware..");
    avr_load_firmware(avr, &f);

    printw("Firmware loaded successfully.");
#if 0
    printw("Initializing VCD...");

    int vcd_result = avr_vcd_init(avr, "trace.vcd", &vcd_file, 10000);
    if (vcd_result != 0) {
        printw(stderr, "Failed to initialize VCD, error code: %d", vcd_result);
        exit(1);
    }


    printw("VCD initialized successfully.");
    avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 3), 1, "BTN");
    avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1), 1, "LED");
    avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 2), 1, "DEBUG");
    avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5), 1, "BUTTON_PRESSED");
    avr_vcd_start(&vcd_file);
#endif

    avr_run(avr);

    avr_irq_t *button_irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 3);
    avr_irq_t *button_pressed_irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5);

    if (!button_irq || !button_pressed_irq) {
        printw("Failed to get IRQ");
        exit(1);
    }

    printw("AVR simulator setup complete.");
}

void tearDown(void) {
    printw("\nTerminating AVR simulator...");
    //avr_vcd_stop(&vcd_file);
    avr_terminate(avr);
    printw("AVR simulator terminated.");
}

void run_avr_for_cycles(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; ++i) {
        avr_run(avr);
    }
}
#if 0
void parse_vcd(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printw(stderr, "Failed to open VCD file: %s", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printw("%s", line);
    }

    fclose(file);
}
#endif
uint8_t read_ocr0a(avr_t *avr) {
    return avr->data[OCR0A_ADDR];
}

void print_mem (void) {
    int16_t counter = 0;
    while (counter < 254) {
        if (avr->data[counter] > 0) {
            move(5,3);
            printw("(%i) %hu ", counter, avr->data[counter]);
        } else {
            move(5,3);
            printw("%hu ", avr->data[counter]);
        }
        
        counter++;
    }
    printw("");
}
void report() {
    move(10,3);
    printw("Button_state: %s, PWM : %hu", button_pressed_irq->value ? "Down": "Up", read_ocr0a(avr));

}

void read_button() {
    int ch;
    ch = getch();
    if (ch != ERR) {
        if (ch == 'A' || ch == 'a') {
            if (!button_pressed) {
                printw("A key was pressed.");
                avr_raise_irq(button_pressed_irq, 1);
            } 
            
        } else {
            if (button_pressed) {
                printw("A key was released.");
                avr_raise_irq(button_pressed_irq, 0);
            } 
        }
    
    }
    
    refresh();
    
}


int main(void) {
    init_ui();
    printw("UI initiated");
    
    init_simavr();
    printw("Simavr initiated");
    
    while (1) {
        report();
        run_avr_for_cycles(1000000);
        read_button();
    }
}
