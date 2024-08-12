cmake_minimum_required(VERSION 3.4)
project(hrc)

add_executable(${PROJECT_NAME}
main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lexer/include)
target_link_libraries(${PROJECT_NAME} PRIVATE hrc_lexer hrc_version)