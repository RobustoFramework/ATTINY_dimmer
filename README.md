# I2C Dimmer

This project creates a dimmer based on an ATTINY85.
Features:
* Holding button will gradually raise and lower power
* Waits on the extremes to make it easier to end up on full and low power
* Has a timeout on max power, will lower power to not overheat
* Short button pushes shuts off, or goes back to last setting

TODO: 
* Timeout on all light to avoid draining batteries
* Capacitative control (library exists)
* I2C control and sensing
* Sleeping an waking on I2C/off
* ???
* $ $ $!
