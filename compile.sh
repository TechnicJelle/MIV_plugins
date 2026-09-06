#!/bin/bash

if [[ "$1" == "release" ]]; then
	gcc -fPIC -std=c11 -O3 miv_stbi.c -shared -o miv_stbi.so
else
	gcc -fPIC -std=c11 miv_stbi.c -shared -o miv_stbi.so
fi
