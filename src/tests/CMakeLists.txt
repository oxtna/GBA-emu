include_directories(.)

add_executable(Test_GBA test_gba.cpp)
target_link_libraries(Test_GBA PRIVATE GBA)
add_test(NAME Test_GBA COMMAND Test_GBA)

add_executable(Test_data_processing test_data_processing.cpp)
target_link_libraries(Test_data_processing PRIVATE GBA)
add_test(NAME Test_data_processing COMMAND Test_data_processing)

