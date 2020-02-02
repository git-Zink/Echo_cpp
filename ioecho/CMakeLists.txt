project (ioecho)

file (GLOB ioecho_SRC src/*.cpp)
file (GLOB ioecho_HDR include/*.h)

include_directories (
    include
    ../include
    )

add_library (ioecho SHARED ${ioecho_SRC} ${ioecho_HDR})

install (TARGETS ioecho DESTINATION .)
