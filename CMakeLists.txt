cmake_minimum_required(VERSION 3.10)
project(sp_lab3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c)

FLEX_TARGET(Lexer lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
        ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
        PROPERTIES LANGUAGE CXX
)

add_executable(sp_lab3
        main.cpp
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS}
)