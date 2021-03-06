#include <iostream>
#include "http_server.hpp"
#include "controller.hpp"

using namespace sneeze;
using namespace std;

int main() {
    const int max_thread_num = 4;
    sneeze::http_server server(max_thread_num);

    bool r = server.listen("0.0.0.0", "8081");
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
            std::cout << req.get_url() << std::endl;
            std::cout << req.get_query_value("name") << std::endl;
            json["abc"] = "abc";
            json["success"] = true;
            json["number"] = 100.005;
            json["name"] = "中文";
            json["time_stamp"] = std::time(nullptr);
            res.render_json(json);
            std::cout << std::this_thread::get_id() << std::endl;
        }
    });

    server.set_http_handler<GET, POST, OPTIONS>("/upload_multipart", [](request &req, response &res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        if (req.get_method() == "OPTIONS") {
            res.add_header("Access-Control-Allow-Headers", "Authorization");
            res.render_string("");
        } else {
            assert(req.get_content_type() == content_type::multipart);
            auto text = req.get_query_value("text");
            std::cout << text << std::endl;
            auto &files = req.get_upload_files();
            for (auto &file : files) {
                std::cout << file.get_file_path() << " " << file.get_file_size() << std::endl;
            }
            res.render_string("multipart finished");
        }
    });

    server.set_http_handler<GET, POST, OPTIONS>("/params", [](request &req, response &res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        if (req.get_method() == "OPTIONS") {
            res.add_header("Access-Control-Allow-Headers", "Authorization");
            res.render_string("");
        } else if (req.get_method() == "POST") {
            string name = req.get_query_value("name").data();
            res.render_string("OK");
        }
    });

    controller ctl;
    server.set_http_handler<GET, OPTIONS>("/test_con", &controller::upload_book_detail_handler, &ctl);
    server.run();
}
