#include <boost/asio/io_service.hpp>
#include "include/HttpServer.hpp"

int main()
{
    boost::asio::io_service service;
    sneeze::HTTPServer s(service);
    s.listen("0.0.0.0", "HTTP");

    service.run();
    return 0;
}