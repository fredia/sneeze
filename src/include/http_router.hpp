//
// Created by fredia on 19-3-29.
//

#ifndef SNEEZE_HTTP_ROUTER_HPP
#define SNEEZE_HTTP_ROUTER_HPP

#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include "request.hpp"
#include "response.hpp"
#include "mime_type.hpp"
#include "session.hpp"
#include "function_traits.hpp"

namespace sneeze {
    class http_router {
    public:
        template<http_method... Is, typename Function>
        std::enable_if_t<!std::is_member_function_pointer_v<Function>>
        register_handler(std::string_view name, Function &&f) {
            if (name.data() == "/*") {
                assert("register error");
                return;
            }

            if (name.back() == '*') {
                pathinfo_mem.push_back(std::string_view(name.data(), name.length() - 1));
            }

            if constexpr(sizeof...(Is) > 0) {
                auto arr = get_arr<Is...>(name);

                for (auto &s : arr) {
                    register_nonmember_func(name, s, std::forward<Function>(f));
                }
            } else {
                register_nonmember_func(name, std::string(name.data(), name.length()), std::forward<Function>(f));
            }
        }

        template<http_method... Is, class T, class Type, typename T1>
        std::enable_if_t<std::is_same_v<T *, T1>>
        register_handler(std::string_view name, Type (T::* f)(request &, response &), T1 t) {
            register_handler_impl<Is...>(name, f, t);
        }

        template<http_method... Is, class T, class Type>
        void register_handler(std::string_view name, Type(T::* f)(request &, response &)) {
            register_handler_impl<Is...>(name, f, (T *) nullptr);
        }

        void remove_handler(std::string name) {
            this->map_invokers_.erase(name);
        }

        //elimate exception, resut type bool: true, success, false, failed
        bool route(std::string_view method, std::string_view url, request &req, response &res) {
            std::string key(method.data(), method.length());
            bool is_static_res_flag = false;
            if (url.rfind('.') == std::string_view::npos) {
                url = url.length() > 1 && url.back() == '/' ? url.substr(0, url.length() - 1) : url;
                auto pos = url.rfind("index");
                if (pos != std::string_view::npos)
                    key += url.substr(0, pos == 1 ? 1 : pos - 1);
                else
                    key += std::string(url.data(), url.length());
            } else {
                key += std::string(STAIC_RES.data(), STAIC_RES.length());
                is_static_res_flag = true;
            }

            auto it = map_invokers_.find(key);
            if (it == map_invokers_.end()) {
                return get_wildcard_function(key, req, res);
            }
            if (is_static_res_flag == false)
                session_manager::get().check_expire();
            it->second(req, res);
            return true;
        }

    private:
        template<http_method N>
        constexpr void get_str(std::string &s, std::string_view name) {
            s = type_to_name(std::integral_constant<http_method, N>{});
            s += std::string(name.data(), name.length());
        }

        template<http_method... Is>
        constexpr auto get_arr(std::string_view name) {
            std::array<std::string, sizeof...(Is)> arr = {};
            size_t index = 0;
            (get_str<Is>(arr[index++], name), ...);

            return arr;
        }

        bool get_wildcard_function(const std::string &key, request &req, response &res) {
            for (auto &pair : wildcard_invokers_) {
                if (key.find(pair.first) != std::string::npos) {
                    pair.second(req, res);
                    return true;
                }
            }
            return false;
        }

        template<http_method... Is, class T, class Type, typename T1>
        void register_handler_impl(std::string_view name, Type T::* f, T1 t) {
            if constexpr(sizeof...(Is) > 0) {
                auto arr = get_arr<Is...>(name);

                for (auto &s : arr) {
                    register_member_func(name, s, f, t);
                }
            } else {
                register_member_func(name, std::string(name.data(), name.length()), f, t);
            }
        }

        template<typename Function>
        void register_nonmember_func(std::string_view raw_name, const std::string &name, Function f) {
            if (raw_name.back() == '*') {
                this->wildcard_invokers_[name.substr(0, name.size() - 2)] = std::bind(
                        &http_router::invoke<Function>, this,
                        std::placeholders::_1, std::placeholders::_2, std::move(f));
            } else {
                this->map_invokers_[name] = std::bind(&http_router::invoke<Function>, this,
                                                      std::placeholders::_1, std::placeholders::_2, std::move(f));
            }
        }

        template<typename Function>
        void invoke(request &req, response &res, Function f) {
            using result_type = std::result_of_t<Function(request &, response &)>;


            if constexpr(std::is_void_v<result_type>) {
                //business
                f(req, res);
            } else {
                //business
                result_type result = f(req, res);
            }
        }

        template<typename Function, typename Self>
        void
        register_member_func(std::string_view raw_name, const std::string &name, Function f, Self self) {
            if (raw_name.back() == '*') {
                this->wildcard_invokers_[name.substr(0, name.size() - 2)] = std::bind(
                        &http_router::invoke_mem<Function, Self>, this,
                        std::placeholders::_1, std::placeholders::_2, f, self);
            } else {
                this->map_invokers_[name] = std::bind(&http_router::invoke_mem<Function, Self>, this,
                                                      std::placeholders::_1, std::placeholders::_2, f, self);
            }
        }

        template<typename Function, typename Self>
        void invoke_mem(request &req, response &res, Function f, Self self) {
            using result_type = typename function_traits<Function>::result_type;

            using nonpointer_type = std::remove_pointer_t<Self>;
            if constexpr(std::is_void_v<result_type>) {
                //business
                if (self)
                    (*self.*f)(req, res);
                else
                    (nonpointer_type{}.*f)(req, res);
            } else {
                //business
                result_type result;
                if (self)
                    result = (*self.*f)(req, res);
                else
                    result = (nonpointer_type{}.*f)(req, res);
            }
        }


        typedef std::function<void(request &, response &)> invoker_function;
        std::map<std::string, invoker_function> map_invokers_;
        std::unordered_map<std::string, invoker_function> wildcard_invokers_; //for url/*
    };
}


#endif //SNEEZE_HTTP_ROUTER_HPP
