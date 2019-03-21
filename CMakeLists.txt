cmake_minimum_required(VERSION 3.13)
project(sneeze)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(sneeze ${Boost_INCLUDE_DIRS})
link_directories(sneeze ${Boost_LIBRARY_DIRS})
target_link_libraries(sneeze ${Boost_LIBRARIES})


add_executable(sneeze main.cpp include/Router.hpp include/HttpServer.hpp include/Connection.hpp)