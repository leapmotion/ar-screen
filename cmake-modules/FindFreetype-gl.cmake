#.rst
# FindFreetype-gl
# ------------
#
# Created by Raffi Bedikian.
# Locate and configure Freetype-gl
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   Freetype-gl::Freetype-gl
#
# Variables
# ^^^^^^^^^
#   Freetype-gl_ROOT_DIR
#   Freetype-gl_FOUND
#   Freetype-gl_INCLUDE_DIR
#   Freetype-gl_LIBRARIES
#

find_path(Freetype-gl_ROOT_DIR
          NAMES include/freetype-gl.h
          PATH_SUFFIXES Freetype-gl-${Freetype-gl_FIND_VERSION}
                        Freetype-gl)

set(Freetype-gl_INCLUDE_DIR ${Freetype-gl_ROOT_DIR}/include)

if(MSVC)
  find_library(Freetype-gl_LIBRARY_RELEASE "freetype-gl.lib" HINTS "${Freetype-gl_ROOT_DIR}" PATH_SUFFIXES lib)
  find_library(Freetype-gl_LIBRARY_DEBUG "freetype-gl-d.lib" HINTS "${Freetype-gl_ROOT_DIR}" PATH_SUFFIXES lib)
else(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_library(Freetype-gl_LIBRARY_RELEASE "libfreetype-gl.a" HINTS "${Freetype-gl_ROOT_DIR}" PATH_SUFFIXES lib)
  find_library(Freetype-gl_LIBRARY_DEBUG "libfreetype-gl.a" HINTS "${Freetype-gl_ROOT_DIR}" PATH_SUFFIXES lib)
endif()
include(SelectConfigurations)
select_configurations(Freetype-gl LIBRARY LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Freetype-gl DEFAULT_MSG Freetype-gl_ROOT_DIR Freetype-gl_INCLUDE_DIR Freetype-gl_LIBRARY_RELEASE Freetype-gl_LIBRARY_DEBUG)

include(CreateImportTargetHelpers)

generate_import_target(Freetype-gl STATIC)
