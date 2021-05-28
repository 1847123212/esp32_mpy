cmake_minimum_required(VERSION 3.5)

# use BOARD-Directory name for project id
set(BOARD "espressif_addax_1")
get_filename_component(PROJECT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT ${PROJECT})

# TOP is absolute path to root directory of TinyUSB git repo
set(TOP "../../submodule/esp-idf/components/tinyusb/tinyusb")
get_filename_component(TOP "${TOP}" REALPATH)

set(IDF_TARGET esp32s3)
set(FAMILY "esp32s3")

set(SDKCONFIG_DEFAULTS
    sdkconfig.base_s3
    sdkconfig.240mhz_s3
    sdkconfig.spiram_s3
    sdkconfig.freertos
    sdkconfig.usb
)

# Check for -DFAMILY=
if(FAMILY MATCHES "^esp32s[2-3]")
  cmake_minimum_required(VERSION 3.5)

  include(${TOP}/hw/bsp/${FAMILY}/family.cmake)
  project(${PROJECT})

else()
  message(FATAL_ERROR "Invalid FAMILY specified: ${FAMILY}")
endif()
