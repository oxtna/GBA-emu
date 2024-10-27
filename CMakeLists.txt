cmake_minimum_required(VERSION 3.19..3.27)

project(
    GBA_Emulator
    DESCRIPTION ""
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR "In-source builds are not allowed."
    )
endif()

# Disable compiler extensions
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CTest)

add_subdirectory(src)

