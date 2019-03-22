
#include "http_server.hpp"
#include "handler.hpp"

using namespace sneeze;

int main() {
    // HTTP 服务运行在 8080 端口，并启用1个线程
    Server<HTTP> server(8080, 1);
    start_server<Server<HTTP>>(server);
    return 0;
}
