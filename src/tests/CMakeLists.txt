include_directories(.)

add_executable(TestGBA test_gba.cpp)
target_link_libraries(TestGBA PRIVATE GBA)
add_test(NAME TestGBA COMMAND TestGBA)

