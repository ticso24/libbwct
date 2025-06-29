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

set(public_headers
	src/aarray.h
	src/array.h
	src/base.h
	src/bsd.h
	src/bwct.h
	src/compress.h
	src/config.h
	src/fdhelper.h
	src/json.h
	src/mstring.h
	src/network.h
	src/sarray.h
	src/ssl.h
	src/thread.h
	src/threadpool.h
	src/tool.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${public_headers}")

include(GNUInstallDirs)

# install the target and create export-set
install(TARGETS ${PROJECT_NAME}
    EXPORT "${PROJECT_NAME}Targets"
    # these get default values from GNUInstallDirs, no need to set them
    #RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # bin
    #LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # lib
    #ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # lib
    # except for public headers, as we want them to be inside a library folder
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME} # include/SomeLibrary
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # include
)

# generate and install export file
install(EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    NAMESPACE ${namespace}::
    DESTINATION cmake
)

include(CMakePackageConfigHelpers)

# generate the version file for the config file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)
# create config file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION cmake
)
# install config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION cmake
)
