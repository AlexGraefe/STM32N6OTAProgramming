if(CONFIG_TCP_SOCKET)

    zephyr_include_directories(.)

    zephyr_library_sources(n6_communication.c)

endif()
