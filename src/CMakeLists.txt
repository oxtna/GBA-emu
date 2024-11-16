include_directories(.)

add_library(GBA STATIC emulator.cpp cpu.cpp memory.cpp instruction_types.cpp)

add_executable(GBA_Emu main.cpp)

target_link_libraries(GBA_Emu PRIVATE GBA)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

if(TARGET SDL2::SDL2main)
    target_link_libraries(GBA_Emu PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(GBA_Emu PRIVATE SDL2::SDL2)

add_subdirectory(tests)

