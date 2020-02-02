project (client)

file (GLOB client_SRC src/*.cpp)
file (GLOB client_HDR include/*.h)

include_directories (
    include
    ../ioecho/include
    ../include
    )

add_executable (client ${client_SRC} ${client_HDR})

target_link_libraries (client ioecho)

install (TARGETS client DESTINATION .)
