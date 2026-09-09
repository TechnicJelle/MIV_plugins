#!/bin/bash

if [ ! -f stbi.o ]; then
	gcc -fPIC -std=gnu11 -O3 stbi.c -c -o stbi.o
	gcc -fPIC -std=gnu11 -O3 stbi_png.c -c -o stbi_png.o
fi

if [[ "$1" == "release" ]]; then
	gcc -fPIC -std=gnu11 -O3 miv_stbi.c stbi.o -shared -o miv_stbi.so
	gcc -fPIC -std=gnu11 -O3 miv_libwebp.c -shared -o miv_libwebp.so -lwebp -lwebpdemux
	gcc -fPIC -std=gnu11 -O3 miv_zipkra.c stbi_png.o -shared -o miv_zipkra.so -lzip $(pkg-config --cflags --libs libxml-2.0)
else
	gcc -fPIC -std=gnu11 -g miv_stbi.c stbi.o -shared -o miv_stbi.so
	gcc -fPIC -std=gnu11 -g miv_libwebp.c -shared -o miv_libwebp.so -lwebp -lwebpdemux
	gcc -fPIC -std=gnu11 -g miv_zipkra.c stbi_png.o -shared -o miv_zipkra.so -lzip $(pkg-config --cflags --libs libxml-2.0)
fi
