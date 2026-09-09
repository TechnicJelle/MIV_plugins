#!/bin/bash

if [ ! -f stbi.o ]; then
	gcc -fPIC -std=c11 -O3 stbi.c -c -o stbi.o
fi

if [[ "$1" == "release" ]]; then
	gcc -fPIC -std=c11 -O3 miv_stbi.c stbi.o -shared -o miv_stbi.so
	gcc -fPIC -std=c11 -O3 miv_libwebp.c -shared -o miv_libwebp.so -lwebp -lwebpdemux
else
	gcc -fPIC -std=c11 -g miv_stbi.c stbi.o -shared -o miv_stbi.so
	gcc -fPIC -std=c11 -g miv_libwebp.c -shared -o miv_libwebp.so -lwebp -lwebpdemux
fi
