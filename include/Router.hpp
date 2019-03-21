//
// Created by fredia on 19-3-21.
//
#pragma once
#include <unordered_map>
#include <functional>

namespace sneeze {

    class Request;

    class Response;

    class Router {
    public:
        Router();

        ~Router();

    private:
        typedef std::function<void(Request &, Response &)> handler_t;
        std::unordered_map<std::string, handler_t> disp_map_;
    };
}


