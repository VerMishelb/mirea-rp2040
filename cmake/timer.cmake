set(timer_DIR ${CMAKE_SOURCE_DIR}/lib/timer)

add_library(timer STATIC)
target_sources(timer
    PRIVATE
        ${timer_DIR}/timer.c
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
        ${timer_DIR}
    FILES
        ${timer_DIR}/timer.h
)
# target_include_directories(timer
#     PUBLIC
#         ${timer_DIR}
# )

target_link_libraries(timer
    PUBLIC
        pico_stdlib
        hardware_pio
        hardware_spi
        hardware_dma
        hardware_clocks
)
add_library(TIMER_FILES ALIAS timer)
message(STATUS "Imported timer.")
