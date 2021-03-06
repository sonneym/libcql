#
# Install zlib from source
#

if (NOT zlib_NAME)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.10)

include (ExternalProject)

set(ABBREV "zlib")
set(${ABBREV}_NAME         ${ABBREV})
set(${ABBREV}_INCLUDE_DIRS ${EXT_PREFIX}/include)
set(APP_DEPENDENCIES ${APP_DEPENDENCIES} ${ABBREV})

message("Installing ${zlib_NAME} into ext build area: ${EXT_PREFIX} ...")

ExternalProject_Add(zlib
  PREFIX ${EXT_PREFIX}
  URL http://zlib.net/zlib-1.2.7.tar.gz
  URL_MD5 "60df6a37c56e7c1366cca812414f7b85"
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ./configure -p=${EXT_PREFIX} --static --64
  BUILD_COMMAND make
  INSTALL_COMMAND make install prefix=${EXT_PREFIX}
  BUILD_IN_SOURCE 1
  )

set(${ABBREV}_STATIC_LIBRARIES ${EXT_PREFIX}/lib/libz.a)

set_target_properties(${zlib_NAME} PROPERTIES EXCLUDE_FROM_ALL ON)

endif (NOT zlib_NAME)