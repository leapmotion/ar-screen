# A CMake module to provide a macro for converting shader source
# into C header strings
#
# On Windows, it is assumed that Stringify.exe exists somewhere
# in the sln, likely declared in source/Stringify/CMakeLists.txt

if(NOT BUILD_WINDOWS)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/Stringify.sh.in ${CMAKE_BINARY_DIR}/Stringify.sh @ONLY)
endif()

function(stringify src_files varname)
  set(inc_files "")
  foreach(src_file ${src_files})
    string(REGEX REPLACE ".*[.](.*)" "\\1" extension ${src_file})
    string(REGEX REPLACE "(.*)[.].*" "\\1.gen.h" inc_file ${src_file})
    string(REGEX REPLACE "[\\/][^\\/]+" "" inc_dir ${src_file})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${inc_dir})
    list(APPEND inc_files ${inc_file})
    if(BUILD_WINDOWS)
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${inc_file}
        COMMAND Stringify ${CMAKE_CURRENT_SOURCE_DIR}/${src_file} ${CMAKE_CURRENT_BINARY_DIR}/${inc_file}
        DEPENDS Stringify ${CMAKE_CURRENT_SOURCE_DIR}/${src_file}
      )
    else()
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${inc_file}
        COMMAND ${CMAKE_BINARY_DIR}/Stringify.sh ${CMAKE_CURRENT_SOURCE_DIR}/${src_file} ${CMAKE_CURRENT_BINARY_DIR}/${inc_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src_file}
      )
    endif()
  endforeach()
  set(${varname} ${inc_files} PARENT_SCOPE)
endfunction()
