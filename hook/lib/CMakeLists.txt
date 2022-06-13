# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.cjson cjson-download/CMakeLists.txt)

execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cjson-download )
if(result)
  message(FATAL_ERROR "CMake step for cjson failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cjson-download )
if(result)
  message(FATAL_ERROR "Build step for cjson failed: ${result}")
endif()

# option(CJSON_OVERRIDE_BUILD_SHARED_LIBS "Override BUILD_SHARED_LIBS with CJSON_BUILD_SHARED_LIBS" ON)
# option(CJSON_BUILD_SHARED_LIBS "Overrides BUILD_SHARED_LIBS if CJSON_OVERRIDE_BUILD_SHARED_LIBS is enabled" OFF)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/cJSON-src
  ${CMAKE_CURRENT_BINARY_DIR}/cJSON-build
  )


# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${cjson_SOURCE_DIR}/include")
endif()

add_library(thermal SHARED
  thermal.c
  sysfs.c
  config_data.c
  )

set (header_version "0x0")
execute_process(COMMAND bash -c "git rev-parse --short=8 HEAD 2>/dev/null || echo 0"
  OUTPUT_VARIABLE header_version
  )
string(STRIP header_version header_version)
message(STATUS "header version: ${header_version}")

target_compile_definitions(thermal PUBLIC
  -DTHERMAL_HEADER_SHA1=0x${header_version}
  )

target_include_directories(thermal PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/cJSON-src
  )

target_link_libraries(thermal
  pthread
  cjson
  )
target_include_directories(thermal INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  )

file(GLOB_RECURSE THERMAL_LIB_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include false "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(thermal
  PROPERTIES VERSION ${THERMAL_VERSION_STRING}
  SOVERSION ${THERMAL_VERSION_MAJOR}
  PUBLIC_HEADER "${THERMAL_LIB_HEADERS}")

install(
  TARGETS thermal
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include
  )
