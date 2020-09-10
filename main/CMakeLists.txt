idf_component_register(SRCS "commands.cpp" "tcp_session.cpp" "main.cpp" "uart_server.cpp" "tcp_session.cpp" "LED_Display.cpp" "Task.cpp" "storage.cpp" "wifi.cpp"
                         INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -Wno-missing-field-initializers -Wno-unused-but-set-variable)