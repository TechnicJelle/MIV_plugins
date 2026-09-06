#!/bin/bash

if [[ "$1" == "release" ]]; then
	gcc -fPIC -O3 miv_stbi.c -shared -o miv_stbi.so
else
	gcc -fPIC miv_stbi.c -shared -o miv_stbi.so
fi
