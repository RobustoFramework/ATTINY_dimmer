# Dimmer

This project creates a dimmer based on an ATTINY85 using PlatformIO.

Features:
* Holding button long will gradually raise and lower power
* Waits on the extremes to make it easier to end up on full and low power
* Has a timeout on max power, will lower power to not overheat
* Short button pushes shuts off, or goes back to last setting<br/><br/>

* Simavr simulation (MacOS/Linux, only tested on MacOs)
  * Installing 
    * Install SDL2 on host machine 
  * Running: 
    * First build the .elf: ```pio run  -e attiny85```
    * Then run the client: ```pio run -t exec -e client -vvv```
  * Features
    * Takes keypresses as input (try holding down any key)
    * Outputs resulting PWM values and duration in own thread
      * Takes info from memory directly
  * Dumps memory when "m" is pressed to make it easier to find variables
* Unity testing
  * _Fails_ - test/main_test.c not aligned with the source currently 
  * assumes very basic testing code with buttons that react
  * however shows how to do create a .vcd file for debugging


TODO: 
* Timeout on all light to avoid draining batteries
* Capacitative control (library exists)
