include_directories(.)

add_executable(TestGBA test_gba.cpp)
target_link_libraries(TestGBA PRIVATE GBA)
add_test(NAME TestGBA COMMAND TestGBA)

add_executable(TestGBA_data_processing test_data_processing.cpp)
target_link_libraries(TestGBA_data_processing PRIVATE GBA)
add_test(NAME TestGBA_data_processing COMMAND TestGBA_data_processing)

