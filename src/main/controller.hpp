//
// Created by fredia on 19-4-11.
//

#ifndef SNEEZE_CONTROLLER_CPP_HPP
#define SNEEZE_CONTROLLER_CPP_HPP

#include <iostream>
#include "http_server.hpp"

using namespace sneeze;

class controller {
public:
    void upload_book_file_handler(request &req, response &res) {

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
            res.render_string("OK");
        }
    }

    void upload_book_detail_handler(request &req, response &res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        if (req.get_method() == "OPTIONS") {
            res.add_header("Access-Control-Allow-Headers", "Authorization");
            res.render_string("");
        } else {

            std::string s= req.get_query_value("name");
            std::cout<<s<<std::endl;
            std::string path = "path";
            res.render_string("OK");
        }
    }
};

#endif //SNEEZE_CONTROLLER_CPP_HPP
