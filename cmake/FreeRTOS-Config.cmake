set(FreeRTOS-Config_DIR ${CMAKE_SOURCE_DIR}/lib/FreeRTOS-Config)

add_library(FreeRTOS-Config INTERFACE)
target_include_directories(FreeRTOS-Config SYSTEM
    INTERFACE
        ${FreeRTOS-Config_DIR}
)
target_compile_definitions(FreeRTOS-Config INTERFACE projCOVERAGE_TEST=0)
message(STATUS "Imported FreeRTOS-Config.")
