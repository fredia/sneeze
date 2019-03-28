//
// Created by fredia on 19-3-28.
//

#ifndef SNEEZE_COOKIE_HPP
#define SNEEZE_COOKIE_HPP

#include <ctime>
#include <string>

namespace sneeze {
    /**
     *  rfc标准 http://www.rfc-editor.org/rfc/rfc6265.txt
     *  中文解释 参考：https://www.cnblogs.com/sunzhenchao/p/3897890.html
     * */
    class cookie {
    public:
        cookie() = default;

        /**
         * name 和 value是必须的
         * @param name
         * @param value
         */
        cookie(const std::string &name, const std::string &value) : name_(name), value_(value) {

        }

        void set_version(int version) {
            version_ = version;
        }

        void set_name(const std::string &name) {
            name_ = name;
        }

        std::string get_name() const {
            return name_;
        }

        void set_value(const std::string &value) {
            value_ = value;
        }

        std::string get_value() const {
            return value_;
        }

        void set_secure(bool secure) {
            secure_ = secure;
        }

        void set_max_age(std::time_t seconds) {
            max_age_ = seconds;
        }

        void set_http_only(bool http_only) {
            http_only_ = http_only;
        }

        std::string to_string() const {
            std::string result;
            result.reserve(256);
            result.append(name_);
            result.append("=");
            if (version_ == 0) {
                // Netscape cookie
                result.append(value_);
                if (max_age_ != -1) {
                    result.append("; expires=");
                    result.append(get_gmt_time_str(max_age_));
                }
                if (secure_) {
                    result.append("; secure");
                }
                if (http_only_) {
                    result.append("; HttpOnly");
                }
            } else {
                result.append("\"");
                result.append(value_);
                result.append("\"");

                if (max_age_ != -1) {
                    result.append("; Max-Age=\"");
                    result.append(std::to_string(max_age_));
                    result.append("\"");
                }
                if (secure_) {
                    result.append("; secure");
                }
                if (http_only_) {
                    result.append("; HttpOnly");
                }
                result.append("; Version=\"1\"");
            }
            return result;
        }

    private:
        inline std::string get_gmt_time_str(std::time_t t) const {
            struct tm *GMTime = gmtime(&t);
            char buff[512] = {0};
            strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S %Z", GMTime);
            return buff;
        }


    private:
        int version_ = 0;
        std::string name_ = "";
        std::string value_ = "";
        //它指定了在网络上如何传输cookie值。
        // 默认情况下，cookie是不安全的，也就是说，他们是通过一个普通的、不安全的http链接传输的。
        // 但是如果将cookie标记为安全的，那么它将只在浏览器和服务器通过https或其他安全协议链接是才被传输。这个属性只能保证cookie是保密的
        bool secure_ = false;
        std::time_t max_age_ = -1; // 相对过期时间,以秒为单位。如果该属性的值不是数字，客户端将不做处理。
        bool http_only_ = false; //防止xss 获取到cookie
    };
}
#endif //SNEEZE_COOKIE_HPP
