cmake_minimum_required(VERSION 3.24)
project(project_hermes)

set(CMAKE_CXX_STANDARD 17)

add_executable(chat_client include/chat_message.h src/chat_client.cpp)
add_executable(chat_server include/chat_message.h src/chat_server.cpp)

if (WIN32)
    target_link_libraries(chat_client ws2_32)
    target_link_libraries(chat_server -lws2_32 -lwsock32)

    set(WIN_INCLUDE_DIRECTORY C:\\Users\\wiktoralan.wisniowi\\Desktop\\SDK\\asio-1.24.0\\include)
    target_include_directories(chat_client PUBLIC ${WIN_INCLUDE_DIRECTORY})
    target_include_directories(chat_server PUBLIC ${WIN_INCLUDE_DIRECTORY})

else()

    # find_library...

endif ()

install(TARGETS chat_client chat_server RUNTIME DESTINATION /bin)