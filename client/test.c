#include <simavr/sim_avr.h>
#include <libelf.h>

int main() {
    avr_t *avr = avr_make_mcu_by_name("atmega328p");
    if (avr) {
        avr_init(avr);
        return 0;
    }
    return 1;
}
cd