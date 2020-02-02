project (server)

file (GLOB server_SRC src/*.cpp)
file (GLOB server_HDR include/*.h)

include_directories (
    include
    ../ioecho/include
    ../include
    )

add_executable (server ${server_SRC} ${server_HDR})

target_link_libraries (server ioecho)

install (TARGETS server DESTINATION .)
