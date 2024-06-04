#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>
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

void setUp(void) { 
    sim_log_prefix = "simavr";
    printf("Setup\n");


    // Initialize the AVR simulator
    avr = avr_make_mcu_by_name("attiny85");
    if (!avr) {
        fprintf(stderr, "AVR '%s' not known", "attiny85\n");
        exit(1);
    }


    avr->frequency = 1000000L;
    printf("Avr_init\n");
    avr_init(avr);
    elf_firmware_t f;
    memset(&f, 0, sizeof(f));
    printf("Read firmware from %s\n", FIRMWARE_PATH);

    if (elf_read_firmware(FIRMWARE_PATH, &f) < 0) {
        fprintf(stderr, "Failed to read firmware\n");
        exit(1);
    }
    printf("Load firmware..\n");
    avr_load_firmware(avr, &f);

    printf("Firmware loaded successfully.\n");
    printf("Initializing VCD...\n");

    int vcd_result = avr_vcd_init(avr, "trace.vcd", &vcd_file, 10000);
    if (vcd_result != 0) {
        fprintf(stderr, "Failed to initialize VCD, error code: %d\n", vcd_result);
        exit(1);
    }


    printf("VCD initialized successfully.\n");
    avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 3), 1, "BTN");
    avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1), 1, "LED");
    avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 2), 1, "DEBUG");
    avr_vcd_add_signal(&vcd_file, avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5), 1, "BUTTON_PRESSED");
    avr_vcd_start(&vcd_file);

    printf("AVR simulator setup complete.\n");
}

void tearDown(void) {
    printf("\nTerminating AVR simulator...\n");
    avr_vcd_stop(&vcd_file);
    avr_terminate(avr);
    printf("AVR simulator terminated.\n");
}

void run_avr_for_cycles(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; ++i) {
        avr_run(avr);
    }
}

void parse_vcd(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open VCD file: %s\n", filename);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
}

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
    printf("\n");
}

void test_pin_change(void) {
    run_avr_for_cycles(100);
    printf("\n");
    print_mem();

    printf("Starting test_pin_change...\n");

    avr_irq_t *button_irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 3);
    avr_irq_t *led_irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1);
    avr_irq_t *debug_irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 2);
    avr_irq_t *button_pressed_irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5);

    if (!button_irq || !led_irq || !debug_irq || !button_pressed_irq) {
        fprintf(stderr, "Failed to get IRQ\n");
        exit(1);
    }

    // Check initial LED and DEBUG states
    uint8_t initial_led_state = led_irq->value;
    uint8_t initial_debug_state = debug_irq->value;
    uint8_t initial_button_pressed_state = button_pressed_irq->value;
    uint8_t initial_pwm_value = read_ocr0a(avr);  // Read initial PWM value
    printf("Initial LED state: %hu\n", initial_led_state);
    printf("Initial DEBUG state: %hu\n", initial_debug_state);
    printf("Initial BUTTON_PRESSED state: %hu\n", initial_button_pressed_state);
    printf("Initial PWM value: %hu\n", initial_pwm_value);

    // Simulate button press (PB3 low)
    printf("Pressing button (PB3 high)\n");
    avr_raise_irq(button_irq, 1);
    
    run_avr_for_cycles(1000000);  // Run enough cycles to process the button press
    print_mem();
    // Check LED, DEBUG, and PWM states after button press
    uint8_t led_state = led_irq->value;
    uint8_t debug_state = debug_irq->value;
    uint8_t button_pressed_state = button_pressed_irq->value;
    uint8_t pwm_value = read_ocr0a(avr);  // Read PWM value after button press
    printf("LED state after button press: %hu\n", led_state);
    printf("DEBUG state after button press: %hu\n", debug_state);
    printf("BUTTON_PRESSED state after button press: %hu\n", button_pressed_state);
    printf("PWM value after button press: %hu\n", pwm_value);

    TEST_ASSERT_EQUAL(0, led_state);
    TEST_ASSERT_EQUAL(1, debug_state);
    TEST_ASSERT_EQUAL(1, button_pressed_state);



    // Simulate button release (PB3 high)
    printf("Releasing button (PB3 low)\n");
    avr_raise_irq(button_irq, 0);
    run_avr_for_cycles(20000);  // Run enough cycles to process the button release
    // Check LED, DEBUG, and PWM states after button release
    led_state = led_irq->value;
    debug_state = debug_irq->value;
    button_pressed_state = button_pressed_irq->value;
    pwm_value = read_ocr0a(avr);  // Read PWM value after button release
    printf("LED state after button release: %hu\n", led_state);
    printf("DEBUG state after button release: %hu\n", debug_state);
    printf("BUTTON_PRESSED state after button release: %hu\n", button_pressed_state);
    printf("PWM value after button release: %hu\n", pwm_value);
    TEST_ASSERT_EQUAL(0, debug_state);
    TEST_ASSERT_EQUAL(0, button_pressed_state);
    printf("\n");
    print_mem();
    printf("Completed test_pin_change.\n");

}

int main(void) {
    setbuf(stdout, NULL);
    printf("In main\n");

    UNITY_BEGIN();
    RUN_TEST(test_pin_change);
   // printf("Parsing VCD file...\n");
   // parse_vcd("trace.vcd");
    return UNITY_END();
}
