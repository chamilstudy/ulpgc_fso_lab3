#!/bin/bash

gcc -c ../libs/retardo/retardo.c -o ../libs/retardo/retardo.o
ar -crs ../libs/libretardo.a ../libs/retardo/retardo.o

gcc -c ../libs/sala/sala.c -o ../libs/sala/sala.o -lretardo -L../libs -lpthread
ar -crs ../libs/libsala.a ../libs/sala/sala.o

#gcc misala.c -o misala -lsala -L../libs
gcc multihilo.c -o multihilo -lsala -lretardo -L../libs -lpthread