cmake_minimum_required(VERSION 3.10)

project(acchording CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_C_FLAGS_DEBUG "-ggdb -Wall -Wextra -Wshadow -pedantic")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -Wshadow -pedantic -DNDEBUG")

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB SOURCES "${SRC_DIR}/*.cpp")

add_executable(acchording ${SOURCES})

find_package(fmt CONFIG REQUIRED)
target_link_libraries(acchording PRIVATE fmt::fmt-header-only)
target_link_libraries(acchording PRIVATE -lhpdf)
target_link_libraries(acchording PRIVATE -lfontconfig)

install(TARGETS acchording)
