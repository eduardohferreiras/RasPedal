#!/bin/bash

lxterminal -e jackd -d alsa -r 48000 -i 2 -o 2
lxterminal -e g++ `pkg-config --cflags --libs gtkIOStream` -o RasPedal Raspedal_v1.c && ./RasPedal
