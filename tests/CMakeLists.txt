add_executable(test_reader test_reader.c test_get_run.c timer.c)
target_link_libraries(test_reader ndstore)

add_executable(test_writer test_writer.c test_put_run.c timer.c)
target_link_libraries(test_writer ndstore)


add_executable(ndstore_server server.c)
target_link_libraries(ndstore_server ndstore)
