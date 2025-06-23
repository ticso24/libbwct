cmake_minimum_required(VERSION 3.31)

project("bwct"
	VERSION 0.22
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(ZLIB)
if(ZLIB_FOUND)
	set(HAVE_LIBZ 1)
endif(ZLIB_FOUND)

find_package(BZip2)
if(BZIP2_FOUND)
	set(HAVE_LIBBZ2 1)
endif(BZIP2_FOUND)

find_package(OpenSSL)
if(OPENSSL_FOUND)
	set(HAVE_OPENSSL 1)
endif(OPENSSL_FOUND)

configure_file(src/config.h.in src/config.h)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
	PRIVATE
		src/aarray.cc
		src/array.cc
		src/bsd.cc
		src/compress.cc
		src/fdhelper.cc
		src/json.cc
		src/mstring.cc
		src/network.cc
		src/sarray.cc
		src/ssl.cc
		src/thread.cc
		src/threadpool.cc
		src/tool.cc
)

