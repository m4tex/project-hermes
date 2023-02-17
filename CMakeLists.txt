cmake_minimum_required(VERSION 3.24)
project(project_hermes)

set(CMAKE_CXX_STANDARD 17)

add_executable(project_hermes src/main.cpp src/tcp_server.cpp include/tcp_server.h)

if (WIN32)
    set(WIN_INCLUDE_DIRECTORY C:\\Users\\wiktoralan.wisniowi\\Desktop\\SDK\\asio-1.24.0\\include)
    target_include_directories(project_hermes PUBLIC ${WIN_INCLUDE_DIRECTORY})
else()

    # find_library...

endif ()

