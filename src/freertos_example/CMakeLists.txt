if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Skipping FreeRTOS examples as FREERTOS_KERNEL_PATH not defined")
    return()
endif()


set(TGNAM freertos_example)

add_executable(${TGNAM})
target_sources(${TGNAM} PRIVATE main.c)
# All possible libraries can be found here:
# pico-sdk\src\cmake\rp2_common.cmake, or pico-sdk\docs\index.h, or RP-009085-KB-1-raspberry-pi-pico-c-sdk.pdf p.352
# FreeRTOS-Kernel\portable\ThirdParty\GCC\RP2040\library.cmake
target_link_libraries(${TGNAM}
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4 # -Heap4 or -Static
    FreeRTOS-Config
    pico_stdlib
)
# target_include_directories(${TGNAM} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/.)
target_compile_definitions(${TGNAM} PRIVATE
    configNUMBER_OF_CORES=1
)


pico_enable_stdio_usb(${TGNAM} 1)
pico_enable_stdio_uart(${TGNAM} 0)
pico_add_extra_outputs(${TGNAM})