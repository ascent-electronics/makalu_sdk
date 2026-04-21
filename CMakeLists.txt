cmake_minimum_required(VERSION 3.22)

file(GLOB_RECURSE MAKALU_SDK_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/can/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ota/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/flash/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/config/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/diag/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/util/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers/*.c
)

add_library(makalu_sdk STATIC ${MAKALU_SDK_SOURCES})

file(GLOB_RECURSE MAKALU_SDK_INCLUDE_DIRS LIST_DIRECTORIES true
        ${CMAKE_CURRENT_SOURCE_DIR}/can/bx
        ${CMAKE_CURRENT_SOURCE_DIR}/can/fd
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers
        ${CMAKE_CURRENT_SOURCE_DIR}/ota
        ${CMAKE_CURRENT_SOURCE_DIR}/flash
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        ${CMAKE_CURRENT_SOURCE_DIR}/diag
        ${CMAKE_CURRENT_SOURCE_DIR}/util

)

target_include_directories(makalu_sdk PUBLIC
        ${MAKALU_SDK_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../app/
)

target_link_libraries(makalu_sdk PUBLIC stm32cubemx)

