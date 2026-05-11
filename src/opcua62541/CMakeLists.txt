set(TARGET_NAME w5x00_opcua_62541)

add_executable(${TARGET_NAME}
        w5x00_opcua.c
        w5x00_open62541_net.c
        opcua_model.c
        )

target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
	FreeRTOS-Kernel-Heap4
        # FREERTOS_FILES
        ETHERNET_FILES
        IOLIBRARY_FILES
        TIMER_FILES
        open62541
        )

# open62541 can generate a lot of warnings in amalgamated mode. Keep the project
# warnings useful by not treating them as fatal.
target_compile_options(${TARGET_NAME} PRIVATE
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
