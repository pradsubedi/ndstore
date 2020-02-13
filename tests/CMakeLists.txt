add_executable(ndstore_server server.c)
target_link_libraries(ndstore_server ndstore)

add_executable(test_reader test_reader.c test_get_run.c timer.c)
target_link_libraries(test_reader ndstore)

add_executable(test_writer test_writer.c test_put_run.c timer.c)
target_link_libraries(test_writer ndstore)


find_program (BASH_PROGRAM bash)

if (BASH_PROGRAM)
  add_test (Test_write ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test_script.sh 1)
  add_test (Test_read ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test_script.sh 2)
  add_test (Test_read_data_subset ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test_script.sh 3)
  add_test (Test_read_ts_subset ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/test_script.sh 4)
endif (BASH_PROGRAM)



