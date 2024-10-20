cmake_minimum_required(VERSION 3.19..3.27)

project(
    GBA_Emulator
    VERSION 1.0
    DESCRIPTION ""
    LANGUAGES C
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR "In-source builds are not allowed."
    )
endif()

# Disable compiler extensions
# set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(CTest)

include(FetchContent)
#FetchContent_Declare(
#SFML
#GIT_REPOSITORY https://github.com/SFML/SFML.git
#GIT_TAG 2.6.x
#)
#FetchContent_MakeAvailable(SFML)

add_subdirectory(src)

