#.rst
# FindNdkProfiler
# ------------
#
# Created by Hyoung Ham.
# Locate and configure NdkProfiler
#
# Interface Targets
# ^^^^^^^^^^^^^^^^^
#   NdkProfiler::NdkProfiler
#
# Variables
# ^^^^^^^^^
#  NdkProfiler_ROOT_DIR
#  NdkProfiler_FOUND
#  NdkProfiler_INCLUDE_DIR
#  NdkProfiler_LIBRARY


find_path(NdkProfiler_ROOT_DIR
  NAMES include/ndkprofiler/prof.h
  PATH_SUFFIXES ndkprofiler
)

set(NdkProfiler_INCLUDE_DIR ${NdkProfiler_ROOT_DIR}/include)

find_library(NdkProfiler_LIBRARY "libandroid-ndk-profiler.a" HINTS ${NdkProfiler_ROOT_DIR}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NdkProfiler DEFAULT_MSG NdkProfiler_INCLUDE_DIR NdkProfiler_LIBRARY)

include(CreateImportTargetHelpers)
generate_import_target(NdkProfiler STATIC)
