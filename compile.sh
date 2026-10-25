#!/bin/bash
set -eo pipefail
cd "$(dirname "$0")"

if [[ "$1" == "clean" ]]; then
	rm -f stbi.o stbi_png.o miv_stbi.so miv_libwebp.so miv_zipkra.so
	rm -rf "libs/libzip/build" "libs/libxml2-ee/build"
	exit
fi

INCLUDES=(-IMIV/plugins -Ilibs -Ilibs/libzip/lib -Ilibs/libzip/build -Ilibs/libxml2-ee/include -Ilibs/libxml2-ee/build/include)

if [ ! -f stbi.o ]; then
	gcc -fPIC -std=gnu11 -O3 stbi.c "${INCLUDES[@]}" -c -o stbi.o
	gcc -fPIC -std=gnu11 -O3 stbi_png.c "${INCLUDES[@]}" -c -o stbi_png.o

	## libzip (used by zipkra)
	cmake -S "libs/libzip" -B "libs/libzip/build" \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=OFF \
		-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
		-DENABLE_ZSTD=OFF \
		-DENABLE_LZMA=OFF \
		-DENABLE_BZIP2=OFF \
		-DENABLE_COMMONCRYPTO=OFF \
		-DENABLE_GNUTLS=OFF \
		-DENABLE_OPENSSL=OFF \
		-DENABLE_WINDOWS_CRYPTO=OFF \
		-DBUILD_TOOLS=OFF \
		-DBUILD_DOC=OFF \
		-DBUILD_EXAMPLES=OFF \
		-DBUILD_REGRESS=OFF \
		-DLIBZIP_DO_INSTALL=OFF \
		-G Ninja
	cmake --build "libs/libzip/build" --config Release --parallel

	## libxml2-ee (used by zipkra)
	meson setup "libs/libxml2-ee/build" "libs/libxml2-ee" \
		--buildtype=release \
		--default-library=static \
		-Db_staticpic=true \
		-Db_lto=true \
		-Db_ndebug=true \
		-Dminimum=true \
		-Davx=enabled \
		-Dsimd=enabled \
		-Dthreads=enabled \
		-Diconv=disabled \
		-Dicu=disabled \
		-Dzlib=disabled \
		-Ddocs=disabled \
		-Dhistory=disabled \
		-Dreadline=disabled \
		-Dc_args="-march=native -O3 -fno-semantic-interposition"
	meson compile -C "libs/libxml2-ee/build" xml2
fi

if [[ "$1" == "release" ]]; then
	FLAG="-O3"
else
	FLAG="-g"
fi

gcc -fPIC -std=gnu11 $FLAG miv_stbi.c stbi.o "${INCLUDES[@]}" -shared -o miv_stbi.so
gcc -fPIC -std=gnu11 $FLAG miv_libwebp.c "${INCLUDES[@]}" -shared -o miv_libwebp.so -lwebp -lwebpdemux
gcc -fPIC -std=gnu11 $FLAG miv_zipkra.c stbi_png.o "libs/libzip/build/lib/libzip.a" "libs/libxml2-ee/build/libxml2.a" "${INCLUDES[@]}" -lz -shared -o miv_zipkra.so
gcc -fPIC -std=gnu11 $FLAG miv_libtiff.c "${INCLUDES[@]}" -shared -o miv_libtiff.so -ltiff
