# Dimmer

This project creates a dimmer based on an ATTINY85.
Features:
* Holding button will gradually raise and lower power
* Waits on the extremes to make it easier to end up on full and low power
* Has a timeout on max power, will lower power to not overheat
* Short button pushes shuts off, or goes back to last setting
* Simavr simulation (MacOS/Linux only) - Build attiny85 first
  * SDL2 client (needs SDL2 installed)
    * Takes keypresses as input
    * Outputs resulting PWM values and duration
      * Takes info from memory directly
    * Dumps memory when "M" is pressed to make it easier to find variables
  * Unity testing
    * fails - test/main_test.c not aligned with the source currently 
    * assumes a very basic testing with buttons
    * however shows how to do create a .vcd file for debugging
  

TODO: 
* Timeout on all light to avoid draining batteries
* Capacitative control (library exists)
