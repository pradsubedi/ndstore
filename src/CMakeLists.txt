# list of source files
set(ndstore-src bbox.c ss_data.c ndstore-client.c ndstore-server.c)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (ndstore-pkg "share/cmake/ndstore")

#
# library version set here (e.g. for shared libs).
#
set (NDSTORE_VERSION_MAJOR 0)
set (NDSTORE_VERSION_MINOR 1)
set (NDSTORE_VERSION_PATCH 0)
set (ndstore-vers "${NDSTORE_VERSION_MAJOR}.${NDSTORE_VERSION_MINOR}")
set (NDSTORE_VERSION "${ndstore-vers}.${NDSTORE_VERSION_PATCH}")

add_library(ndstore ${ndstore-src})
target_link_libraries (ndstore margo)
target_include_directories (ndstore PUBLIC $<INSTALL_INTERFACE:include>)

# local include's BEFORE, in case old incompatable .h files in prefix/include
target_include_directories (ndstore BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)

# for shared libs, establish the lib version
set_target_properties (ndstore
    PROPERTIES VERSION ${NDSTORE_VERSION}
    SOVERSION ${NDSTORE_VERSION_MAJOR})

#
# installation stuff (packaging and install commands)
#
write_basic_package_version_file(
    "ndstore-config-version.cmake"
    VERSION ${NDSTORE_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
configure_file (ndstore-config.cmake.in ndstore-config.cmake @ONLY)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (PRIVATE_LIBS "-lndstore")
configure_file ("ndstore.pc.in" "ndstore.pc" @ONLY)

#
# "make install" rules
#
install (TARGETS ndstore EXPORT ndstore-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
install (EXPORT ndstore-targets
         DESTINATION ${ndstore-pkg}
         FILE "ndstore-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ndstore-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/ndstore-config-version.cmake"
               "../cmake/xpkg-import.cmake"
         DESTINATION ${ndstore-pkg} )
install (DIRECTORY ../include/ndstore
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/ndstore.pc"
		DESTINATION "lib/pkgconfig/")
