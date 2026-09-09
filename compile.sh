#!/bin/bash

if [[ "$1" == "clean" ]]; then
	rm -f stbi.o stbi_png.o miv_stbi.so miv_libwebp.so miv_zipkra.so
	exit
fi

if [ ! -f stbi.o ]; then
	gcc -fPIC -std=gnu11 -O3 stbi.c -c -o stbi.o
	gcc -fPIC -std=gnu11 -O3 stbi_png.c -c -o stbi_png.o
fi

if [[ "$1" == "release" ]]; then
	FLAG="-O3"
else
	FLAG="-g"
fi
gcc -fPIC -std=gnu11 $FLAG miv_stbi.c stbi.o -shared -o miv_stbi.so
gcc -fPIC -std=gnu11 $FLAG miv_libwebp.c -shared -o miv_libwebp.so -lwebp -lwebpdemux
gcc -fPIC -std=gnu11 $FLAG miv_zipkra.c stbi_png.o -shared -o miv_zipkra.so -lzip $(pkg-config --cflags --libs libxml-2.0)
