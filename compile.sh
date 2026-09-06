#!/bin/bash

if [ ! -f stbi.o ]; then
	gcc -fPIC -std=c11 -O3 stbi.c -c -o stbi.o
fi

if [[ "$1" == "release" ]]; then
	gcc -fPIC -std=c11 -O3 miv_stbi.c stbi.o -shared -o miv_stbi.so
else
	gcc -fPIC -std=c11 miv_stbi.c stbi.o -shared -o miv_stbi.so
fi
