cmake_minimum_required(VERSION 3.0.0)
project(Tuan VERSION 0.1.0)

find_package(Boost COMPONENTS system REQUIRED)
# include(CTest)
# enable_testing()

include_directories ("${PROJECT_SOURCE_DIR}")
add_executable(server server.cpp Protocol.cpp Socket.cpp)

target_link_libraries(server ${Boost_LIBRARIES} ${EXTRA_LIBS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
