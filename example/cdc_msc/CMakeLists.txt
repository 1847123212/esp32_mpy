cmake_minimum_required(VERSION 3.5)

# use BOARD-Directory name for project id
set(BOARD "espressif_addax_1")
get_filename_component(PROJECT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT ${BOARD}-${PROJECT})

# TOP is absolute path to root directory of TinyUSB git repo
set(TOP "../../submodule/esp-idf/components/tinyusb/tinyusb")
get_filename_component(TOP "${TOP}" REALPATH)

set(FAMILY "esp32s3")

# Check for -DFAMILY=
if(FAMILY MATCHES "^esp32s[2-3]")
  cmake_minimum_required(VERSION 3.5)

  include(${TOP}/hw/bsp/${FAMILY}/family.cmake)
  project(${PROJECT})

else()
  message(FATAL_ERROR "Invalid FAMILY specified: ${FAMILY}")
endif()
