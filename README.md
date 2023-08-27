AstrayOS
========

> **astray**: [adverb or adjective] off the right path or route; straying

This is my project for writing a small operating system for a RaspberryPi. I'm initially targeting the the Pi 3 since [QEMU](https://www.qemu.org) supports that as of my start date, but I may move up to the 4.

I'm not sure exactly what this project will be, thus the name. But I have a couple of ideas (some may not pan out):

* Absolutely will be 64-bit
* Write more than the minimum in assembly
* Try using Pascal strings as much as possible
* Who needs POSIX?
* Standard C library? Maybe it doesn't need to be so standard
* Generally see how far I can get towards a somewhat functional non-graphical OS
* Get some kind of multi-threading or multi-processing working
* Eventually achieve simple network access (e.g. telnet)

I kind of like the idea of a microkernel, but I'm not sure I'll go that way. The Cortex-A53 in a Pi 3 has 4 privilege levels (plus possibly a secure and non-secure domain). Wouldn't it be interesting to run drivers in ring 1 but the core kernel in ring 2? OS/2 seems to have done that.

Reference material used:

* [rpi4-osdev](https://github.com/isometimes/rpi4-osdev) by Adam Greenwood-Byrne
* [raspi3-tutorial](https://github.com/bztsrc/raspi3-tutorial/) by Zoltan Baldaszti
* [OSDev.org wiki](https://wiki.osdev.org/Main_Page)
