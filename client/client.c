#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sim_avr.h>
#include <sim_elf.h>
#include <sim_io.h>
#include <sim_interrupts.h>
#include <avr_ioport.h>
#include <sim_vcd_file.h>
#include <sim_cycle_timers.h>



#define ATTINY95_IO_OFFSET 0x22
#define OCR0A_ADDR ATTINY95_IO_OFFSET + 0x27 // OCR0A address for ATtiny85

avr_t *avr;
char *sim_log_prefix;  
avr_vcd_t vcd_file;

avr_irq_t *button_irq;
avr_irq_t *button_pressed_irq;

bool button_pressed = false;


void init_simavr(void) { 
    sim_log_prefix = "simavr";
    printf("Setup");


    // Initialize the AVR simulator
    avr = avr_make_mcu_by_name("attiny85");
    if (!avr) {
        printf("AVR '%s' not known", "attiny85");
        exit(1);
    }


    avr->frequency = 1000000L;
    printf("Avr_init");
    avr_init(avr);
    elf_firmware_t f;
    memset(&f, 0, sizeof(f));
    printf("Read firmware from %s", "C:/Users/Nickl/.platformio/packages/tool-simavr/include/simav");

    if (elf_read_firmware("C:/Users/Nickl/.platformio/packages/tool-simavr/include/simav", &f) < 0) {
        printf("Failed to read firmware");
        exit(1);
    }
    printf("Load firmware..");
    avr_load_firmware(avr, &f);

    printf("Firmware loaded successfully.");
#if 0
    printf("Initializing VCD...");

    int vcd_result = avr_vcd_init(avr, "trace.vcd", &vcd_file, 10000);
    if (vcd_result != 0) {
        printf(stderr, "Failed to initialize VCD, error code: %d", vcd_result);
        exit(1);
    }


    printf("VCD initialized successfully.");
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
        printf("Failed to get IRQ");
        exit(1);
    }

    printf("AVR simulator setup complete.");
}

void tearDown(void) {
    printf("\nTerminating AVR simulator...");
    //avr_vcd_stop(&vcd_file);
    avr_terminate(avr);
    printf("AVR simulator terminated.");
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
        printf(stderr, "Failed to open VCD file: %s", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
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
            printf("(%i) %hu ", counter, avr->data[counter]);
        } else {
            printf("%hu ", avr->data[counter]);
        }
        
        counter++;
    }
    printf("");
}
void report() {
    printf("Button_state: %s, PWM : %hu", button_pressed_irq->value ? "Down": "Up", read_ocr0a(avr));

}

void read_button() {
    int ch;
    ch = getch();
    if (ch != -1) {
        if (ch == 'A' || ch == 'a') {
            if (!button_pressed) {
                printf("A key was pressed.");
                avr_raise_irq(button_pressed_irq, 1);
            } 
            
        } else {
            if (button_pressed) {
                printf("A key was released.");
                avr_raise_irq(button_pressed_irq, 0);
            } 
        }
    
    }
    
    
}


int main(void) {
    
    printf("Simavr initiating");
    init_simavr();
    printf("Simavr initiated");
    
    while (1) {
        report();
        run_avr_for_cycles(1000000);
        read_button();
    }
}
