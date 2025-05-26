project("bwct"
	# ...
)

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

