cmake_minimum_required(VERSION 3.13)
project(sneeze)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/include)
aux_source_directory(src/main dir_srcs)

#添加头文件搜索路径
include_directories(/usr/include)

#添加库文件搜索路径
link_directories(/usr/lib)

find_package(Boost REQUIRED)

add_executable(sneeze ${dir_srcs} src/include/base_server.hpp src/include/http_server.hpp src/include/handler.hpp)
target_link_libraries(sneeze pthread boost_system)
