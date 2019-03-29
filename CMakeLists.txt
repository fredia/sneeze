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

add_executable(sneeze ${dir_srcs} src/include/http_server.hpp src/include/connection.hpp src/include/http_cache.hpp src/include/request.hpp src/include/response.hpp src/include/gzip.hpp src/include/cookie.hpp src/include/session.hpp src/include/mime_type.hpp src/include/http_code.hpp src/include/session_manager.hpp src/include/upload_file.hpp src/include/url_utils.hpp src/include/multipart_reader.hpp src/include/multipart_parser.hpp src/include/io_service_pool.hpp src/include/http_router.hpp src/include/function_traits.hpp)
target_link_libraries(sneeze pthread boost_system uuid stdc++fs)
