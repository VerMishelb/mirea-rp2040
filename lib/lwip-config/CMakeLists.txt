add_library(lwipopts INTERFACE)
target_sources(lwipopts INTERFACE FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_LIST_DIR} FILES lwipopts.h)
# include_directories(${CMAKE_CURRENT_LIST_DIR})