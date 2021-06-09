# Set location of base MicroPython directory.
get_filename_component(MICROPY_DIR ${PROJECT_DIR}/../../submodule/micropython ABSOLUTE)

option(CONFIG_OPENMV_SUPPORT "option for openmv" ON)

# Include core source components.
include(${MICROPY_DIR}/py/py.cmake)
include(${PROJECT_DIR}/omv/omv.cmake)

if(NOT CMAKE_BUILD_EARLY_EXPANSION)
    include(${MICROPY_DIR}/py/usermod.cmake)
    include(${MICROPY_DIR}/extmod/extmod.cmake)
endif()

set(MICROPY_QSTRDEFS_PORT
    ${PROJECT_DIR}/src/adapter/qstrdefsport.h
)

set(MICROPY_SOURCE_LIB
    ${MICROPY_DIR}/lib/littlefs/lfs1.c
    ${MICROPY_DIR}/lib/littlefs/lfs1_util.c
    ${MICROPY_DIR}/lib/littlefs/lfs2.c
    ${MICROPY_DIR}/lib/littlefs/lfs2_util.c
    ${MICROPY_DIR}/lib/mbedtls_errors/mp_mbedtls_errors.c
    ${MICROPY_DIR}/lib/mp-readline/readline.c
    ${MICROPY_DIR}/lib/netutils/netutils.c
    ${MICROPY_DIR}/lib/oofatfs/ff.c
    ${MICROPY_DIR}/lib/oofatfs/ffunicode.c
    ${MICROPY_DIR}/lib/timeutils/timeutils.c
    ${MICROPY_DIR}/lib/utils/interrupt_char.c
    ${MICROPY_DIR}/lib/utils/sys_stdio_mphal.c
    ${MICROPY_DIR}/lib/utils/pyexec.c
)

set(MICROPY_SOURCE_DRIVERS
    ${MICROPY_DIR}/drivers/bus/softspi.c
    ${MICROPY_DIR}/drivers/dht/dht.c
)

set(MICROPY_SOURCE_PORT
    ${PROJECT_DIR}/src/main.c
    ${PROJECT_DIR}/src/ef_sqrt.c
    ${PROJECT_DIR}/src/adapter/uart.c
    ${PROJECT_DIR}/src/adapter/usb.c
    ${PROJECT_DIR}/src/adapter/gccollect.c
    ${PROJECT_DIR}/src/adapter/mphalport.c
    ${PROJECT_DIR}/src/adapter/fatfs_port.c
    ${PROJECT_DIR}/src/adapter/help.c
    ${PROJECT_DIR}/src/adapter/modutime.c
    ${PROJECT_DIR}/src/adapter/moduos.c
    ${PROJECT_DIR}/src/adapter/machine_timer.c
    ${PROJECT_DIR}/src/adapter/machine_pin.c
    ${PROJECT_DIR}/src/adapter/machine_touchpad.c
    #${PROJECT_DIR}/src/adapter/machine_adc.c
    #${PROJECT_DIR}/src/adapter/machine_dac.c
    ${PROJECT_DIR}/src/adapter/machine_i2c.c
    ${PROJECT_DIR}/src/adapter/machine_pwm.c
    ${PROJECT_DIR}/src/adapter/machine_uart.c
    ${PROJECT_DIR}/src/adapter/modmachine.c
    ${PROJECT_DIR}/src/adapter/modnetwork.c
    ${PROJECT_DIR}/src/adapter/network_lan.c
    ${PROJECT_DIR}/src/adapter/network_ppp.c
    ${PROJECT_DIR}/src/adapter/mpnimbleport.c
    ${PROJECT_DIR}/src/adapter/modsocket.c
    ${PROJECT_DIR}/src/adapter/modesp.c
    ${PROJECT_DIR}/src/adapter/esp32_nvs.c
    ${PROJECT_DIR}/src/adapter/esp32_partition.c
    ${PROJECT_DIR}/src/adapter/esp32_rmt.c
    ${PROJECT_DIR}/src/adapter/esp32_ulp.c
    ${PROJECT_DIR}/src/adapter/modesp32.c
    ${PROJECT_DIR}/src/adapter/espneopixel.c
    ${PROJECT_DIR}/src/adapter/machine_hw_spi.c
    ${PROJECT_DIR}/src/adapter/machine_wdt.c
    ${PROJECT_DIR}/src/adapter/mpthreadport.c
    ${PROJECT_DIR}/src/adapter/machine_rtc.c
    ${PROJECT_DIR}/src/adapter/machine_sdcard.c
)

set(MICROPY_SOURCE_QSTR
    ${MICROPY_SOURCE_PY}
    ${MICROPY_SOURCE_EXTMOD}
    ${MICROPY_SOURCE_USERMOD}
    ${MICROPY_SOURCE_LIB}
    ${MICROPY_SOURCE_PORT}
    ${OMV_MOD_SRCS}
)

set(IDF_COMPONENTS
    app_update
    bootloader_support
    bt
    driver
    esp_common
    esp_eth
    esp_event
    esp_ringbuf
    esp_rom
    esp_wifi
    freertos
    heap
    log
    lwip
    mbedtls
    mdns
    newlib
    nvs_flash
    sdmmc
    soc
    spi_flash
    tcpip_adapter
    ulp
    vfs
    xtensa
)

if(IDF_VERSION_MINOR GREATER_EQUAL 1)
    list(APPEND IDF_COMPONENTS esp_netif)
endif()

if(IDF_VERSION_MINOR GREATER_EQUAL 2)
    list(APPEND IDF_COMPONENTS esp_system)
    list(APPEND IDF_COMPONENTS esp_timer)
endif()

if(IDF_VERSION_MINOR GREATER_EQUAL 3)
    list(APPEND IDF_COMPONENTS esp_hw_support)
    list(APPEND IDF_COMPONENTS esp_pm)
    list(APPEND IDF_COMPONENTS hal)
endif()

if(IDF_TARGET STREQUAL "esp32")
    list(APPEND IDF_COMPONENTS esp32)
elseif(IDF_TARGET STREQUAL "esp32s2")
    list(APPEND IDF_COMPONENTS esp32s2)
    list(APPEND IDF_COMPONENTS tinyusb)
elseif(IDF_TARGET STREQUAL "esp32s3")
    list(APPEND IDF_COMPONENTS esp32s3)
    list(APPEND IDF_COMPONENTS tinyusb)
    list(APPEND IDF_COMPONENTS esp32-camera)
endif()

# Register the main IDF component.
idf_component_register(
    SRCS
        ${MICROPY_SOURCE_PY}
        ${MICROPY_SOURCE_EXTMOD}
        ${MICROPY_SOURCE_LIB}
        ${MICROPY_SOURCE_DRIVERS}
        ${MICROPY_SOURCE_PORT}
        ${OMV_SRCS}
    INCLUDE_DIRS
        ${MICROPY_INC_CORE}
        ${MICROPY_INC_USERMOD}
        ${MICROPY_PORT_DIR}
        ${MICROPY_PORT_DIR}/src/adapter/
        ${OMV_INCLUDEDIRS}
        ${MICROPY_BOARD_DIR}
        ${CMAKE_BINARY_DIR}
        ${MICROPY_DIR}/py  
    REQUIRES
        ${IDF_COMPONENTS}
)

# Set the MicroPython target as the current (main) IDF component target.
set(MICROPY_TARGET ${COMPONENT_TARGET})

# Define mpy-cross flags, for use with frozen code.
set(MICROPY_CROSS_FLAGS -march=xtensawin)

# Set compile options for this port.
target_compile_definitions(${MICROPY_TARGET} PUBLIC
    CONFIG_OPENMV_SUPPORT=1
    ${MICROPY_DEF_CORE}
    MICROPY_ESP_IDF_4=1
    MICROPY_VFS_FAT=1
    MICROPY_VFS_LFS2=1
    FFCONF_H=\"${MICROPY_OOFATFS_DIR}/ffconf.h\"
    LFS1_NO_MALLOC LFS1_NO_DEBUG LFS1_NO_WARN LFS1_NO_ERROR LFS1_NO_ASSERT
    LFS2_NO_MALLOC LFS2_NO_DEBUG LFS2_NO_WARN LFS2_NO_ERROR LFS2_NO_ASSERT
)

# Disable some warnings to keep the build output clean.
target_compile_options(${MICROPY_TARGET} PUBLIC
    -Wno-clobbered
    -Wno-deprecated-declarations
    -Wno-missing-field-initializers
    -Wno-error=type-limits
    -Wno-error=implicit-function-declaration
    -Wno-error=maybe-uninitialized 
    -Wno-error=unused-but-set-parameter
    -Wno-error=uninitialized
    -Wno-error=format=
)

# Add additional extmod and usermod components.
target_link_libraries(${COMPONENT_LIB} "-u __ieee754_sqrtf")
target_link_libraries(${MICROPY_TARGET} micropy_extmod_btree)
target_link_libraries(${MICROPY_TARGET} usermod)


# Collect all of the include directories and compile definitions for the IDF components.
foreach(comp ${IDF_COMPONENTS})
    micropy_gather_target_properties(__idf_${comp})
endforeach()

if(IDF_VERSION_MINOR GREATER_EQUAL 2)
    # These paths cannot currently be found by the IDF_COMPONENTS search loop above,
    # so add them explicitly.
    list(APPEND MICROPY_CPP_INC_EXTRA ${IDF_PATH}/components/soc/soc/${IDF_TARGET}/include)
    list(APPEND MICROPY_CPP_INC_EXTRA ${IDF_PATH}/components/soc/soc/include)
    if(IDF_VERSION_MINOR GREATER_EQUAL 3)
        list(APPEND MICROPY_CPP_INC_EXTRA ${IDF_PATH}/components/tinyusb/additions/include)
        list(APPEND MICROPY_CPP_INC_EXTRA ${IDF_PATH}/components/tinyusb/tinyusb/src)
    endif()
endif()

# Include the main MicroPython cmake rules.
include(${MICROPY_DIR}/py/mkrules.cmake)
