include_directories(.)

add_library(GBA STATIC gba.c)

add_executable(GBA_Emu main.c)

target_link_libraries(GBA_Emu PRIVATE GBA)

add_subdirectory(tests)

