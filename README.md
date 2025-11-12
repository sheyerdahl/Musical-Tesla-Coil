Welcome
======
A DRSSTC (Dual Resonant Solid State Tesla Coil) project that uses the ESP32-S3-WROOM-1 module as the microcontroller and wireless communication. 

Uses the SKM100GB125DN IGBT


Notes
-------
The first revision (REV 1.0) being my first PCB, had quite a few issues and oversights. REV 2.0 will fix these issues, although it will be untested.
Some of the issues that were fixed:

Lack of OCD Comparator
Lack of peak detector circuit for measuring current
Lack of ZCD Comparator
USB-C footprint being incorrect
Slow diodes being used in the GDT circuit
Lack of mounting holes
Using built-in ADC of ESP32, an external one would have significantly more ENOB. (Effective Number Of Bits)
No Reverse polarity protection, no over current protection.



Credits
-------
Most design decisions were made because of Mads Barnkob and his awesome [website](https://kaizerpowerelectronics.dk/tesla-coils/drsstc-design-guide/) and [Youtube videos](https://www.youtube.com/@KaizerPowerElectronicsDk).

Thanks to those who worked on the [UD3 Board](https://github.com/Netzpfuscher/UD3), some of their hardware design was copied to this one.

[JAVATC3D](https://www.classictesla.com/java/javatc3d/javatc3d.html)
