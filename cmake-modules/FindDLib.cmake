#.rst
# FindDLib
# ------------
#
# Created by Walter Gray.
# Locate and configure DLib
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   DLib::DLib
#
# Variables
# ^^^^^^^^^
#  DLib_ROOT_DIR
#  DLib_FOUND
#  DLib_INCLUDE_DIR
#  DLib_LIBRARY

find_path(DLib_ROOT_DIR
          NAMES include/dlib/image_keypoint/hog.h
          PATH_SUFFIXES dlib-${DLib_FIND_VERSION}
                        dlib)

set(DLib_INCLUDE_DIR ${DLib_ROOT_DIR}/include)

find_library(DLib_LIBRARY dlib HINTS "${DLib_ROOT_DIR}/lib")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DLib DEFAULT_MSG DLib_INCLUDE_DIR DLib_LIBRARY)

include(CreateImportTargetHelpers)
generate_import_target(DLib STATIC)
