cmake_minimum_required (VERSION 2.8)

project ("Echo")

SET (CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install")

add_subdirectory (ioecho)
add_subdirectory (server)
add_subdirectory (client)

