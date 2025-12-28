# CTest-based targets for DeerBourne.

add_executable(test_bourne_config test_bourne_config.c)
target_link_libraries(test_bourne_config PRIVATE bourne)
add_test(NAME bourne_config COMMAND test_bourne_config)

add_executable(test_deer_config test_deer_config.c)
target_link_libraries(test_deer_config PRIVATE deer_core)
add_test(NAME deer_config COMMAND test_deer_config)
