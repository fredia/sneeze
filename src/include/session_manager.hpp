//
// Created by fredia on 19-3-29.
//

#ifndef SNEEZE_SESSION_MANAGER_HPP
#define SNEEZE_SESSION_MANAGER_HPP

#include "session.hpp"
#include <uuid/uuid.h>

namespace sneeze {
    class session_manager {
    public:
        static session_manager &get() {
            static session_manager instance;
            return instance;
        }

        inline static std::string get_uuid_random() {
            uuid_t uuid_t_;
            uuid_generate_random(uuid_t_);
            char buf[1024];
            uuid_unparse(uuid_t_, buf);
            std::string uuid_str = std::string(buf);
            return uuid_str;
        }

        std::shared_ptr<session> create_session(const std::string &name, std::size_t expire,
                                                const std::string &path = "/", const std::string &domain = "") {
            std::string uuid_str = get_uuid_random();
            auto s = std::make_shared<session>(name, uuid_str, expire, path, domain);

            {
                std::unique_lock<std::mutex> lock(mtx_);
                map_.emplace(std::move(uuid_str), s);
            }

            return s;
        }

        std::shared_ptr<session> create_session(std::string_view host, const std::string &name,
                                                std::time_t expire = -1, const std::string &path = "/") {
            auto pos = host.find(":");
            if (pos != std::string_view::npos) {
                host = host.substr(0, pos);
            }

            return create_session(name, expire, path, std::string(host.data(), host.length()));
        }

        std::weak_ptr<session> get_session(const std::string &id) {
            std::unique_lock<std::mutex> lock(mtx_);
            auto it = map_.find(id);
            return (it != map_.end()) ? it->second : nullptr;
        }

        void del_session(const std::string &id) {
            std::unique_lock<std::mutex> lock(mtx_);
            auto it = map_.find(id);
            if (it != map_.end())
                map_.erase(it);
        }

        void check_expire() {
            if (map_.empty())
                return;

            auto now = std::time(nullptr);
            std::unique_lock<std::mutex> lock(mtx_);
            for (auto it = map_.begin(); it != map_.end();) {
                if (now - it->second->time_stamp() >= max_age_) {
                    it = map_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void set_max_inactive_interval(int seconds) {
            max_age_ = seconds;
        }

    private:
        session_manager() = default;

        session_manager(const session_manager &) = delete;

        session_manager(session_manager &&) = delete;

        std::map<std::string, std::shared_ptr<session>> map_;
        std::mutex mtx_;
        int max_age_ = 0;
    };
}
#endif //SNEEZE_SESSION_MANAGER_HPP
