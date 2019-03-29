#include <iostream>
#include "http_server.hpp"

using namespace sneeze;

int main() {
    const int max_thread_num = 4;
    sneeze::http_server server(max_thread_num);

    bool r = server.listen("0.0.0.0", "8080");
    if (!r) {
        std::cerr << "listen failed";
        return -1;
    }

    server.set_http_handler<GET, POST, OPTIONS>("/json", [](request &req, response &res) {
        nlohmann::json json;
        res.add_header("Access-Control-Allow-Origin", "*");
        if (req.get_method() == "OPTIONS") {
            res.add_header("Access-Control-Allow-Headers", "Authorization");
            res.render_string("");
        } else {
            json["abc"] = "abc";
            json["success"] = true;
            json["number"] = 100.005;
            json["name"] = "中文";
            json["time_stamp"] = std::time(nullptr);
            res.render_json(json);
        }
    });
    server.run();
}
