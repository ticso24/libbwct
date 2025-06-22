cmake_minimum_required(VERSION 3.31)

project("bwct"
	VERSION 0.18
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(src/config.h.in src/config.h)

find_package(OpenSSL)
find_package(ZLIB)
find_package(BZip2)

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
		src/tool.cc
)

