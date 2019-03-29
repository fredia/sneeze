//
// Created by fredia on 19-3-28.
//

#ifndef SNEEZE_RESPONSE_HPP
#define SNEEZE_RESPONSE_HPP

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <string_view>
#include <fstream>
#include "http_cache.hpp"
#include "mime_type.hpp"
#include "session.hpp"
#include "http_code.hpp"
#include "session_manager.hpp"
#include "nlohmann_json.hpp"

namespace sneeze {
    class response {
    public:
        std::vector<boost::asio::const_buffer> get_response_buffer(std::string &&body) {
            set_content(std::move(body));

            auto buffers = to_buffers();
            return buffers;
        }

        std::vector<boost::asio::const_buffer> to_buffers() {
            std::vector<boost::asio::const_buffer> buffers;
            add_header("Host", "sneeze");
            if (session_ != nullptr && session_->is_need_update()) {
                auto cookie_str = session_->get_cookie().to_string();
                add_header("Set-Cookie", cookie_str.c_str());
                session_->set_need_update(false);
            }
            buffers.reserve(headers_.size() * 4 + 5);
            buffers.emplace_back(to_buffer(status_));
            for (auto const &h : headers_) {
                buffers.emplace_back(boost::asio::buffer(h.first));
                buffers.emplace_back(boost::asio::buffer(name_value_separator));
                buffers.emplace_back(boost::asio::buffer(h.second));
                buffers.emplace_back(boost::asio::buffer(crlf));
            }

            buffers.push_back(boost::asio::buffer(crlf));

            if (body_type_ == content_type::string) {
                buffers.emplace_back(boost::asio::buffer(content_.data(), content_.size()));
            }

            if (http_cache::get().need_cache(raw_url_)) {
                cache_data.clear();
                for (auto &buf : buffers) {
                    cache_data.push_back(
                            std::string(boost::asio::buffer_cast<const char *>(buf), boost::asio::buffer_size(buf)));
                }
            }

            return buffers;
        }

        void add_header(std::string &&key, std::string &&value) {
            headers_.emplace_back(std::move(key), std::move(value));
        }

        void set_status(status_type status) {
            status_ = status;
        }

        status_type get_status() const {
            return status_;
        }

        void set_delay(bool delay) {
            delay_ = delay;
        }

        void set_status_and_content(status_type status) {
            status_ = status;
            set_content(to_string(status).data());
        }

        void set_status_and_content(status_type status, std::string &&content,
                                    res_content_type res_type = res_content_type::none,
                                    content_encoding encoding = content_encoding::none) {
            status_ = status;
            if (res_type != res_content_type::none) {
                auto iter = res_mime_map.find(res_type);
                if (iter != res_mime_map.end()) {
                    add_header("Content-type", std::string(iter->second.data(), iter->second.size()));
                }
            }
#ifdef CINATRA_ENABLE_GZIP
            if (encoding == content_encoding::gzip) {
                std::string encode_str;
                bool r = gzip_codec::compress(std::string_view(content.data(), content.length()), encode_str, true);
                if (!r) {
                    set_status_and_content(status_type::internal_server_error, "gzip compress error");
                }
                else {
                    add_header("Content-Encoding", "gzip");
                    set_content(std::move(encode_str));
                }
            }
            else
#endif
            set_content(std::move(content));
        }

        void set_status_and_content(status_type status, std::string &&content, std::string &&res_content_type_str,
                                    content_encoding encoding = content_encoding::none) {
            status_ = status;
            add_header("Content-type", std::move(res_content_type_str));
#ifdef CINATRA_ENABLE_GZIP
            if (encoding == content_encoding::gzip) {
                std::string encode_str;
                bool r = gzip_codec::compress(std::string_view(content.data(), content.length()), encode_str, true);
                if (!r) {
                    set_status_and_content(status_type::internal_server_error, "gzip compress error");
                }
                else {
                    add_header("Content-Encoding", "gzip");
                    set_content(std::move(encode_str));
                }
            }
            else
#endif
            set_content(std::move(content));
        }

        bool need_delay() const {
            return delay_;
        }

        void reset() {
            status_ = status_type::init;
            proc_continue_ = true;
            headers_.clear();
            content_.clear();
            session_ = nullptr;
            cache_data.clear();
        }

        void set_continue(bool con) {
            proc_continue_ = con;
        }

        bool need_continue() const {
            return proc_continue_;
        }

        void set_content(std::string &&content) {
            char temp[20] = {};
            //itoa
            sprintf(temp, "%d", (int) content.size());
            add_header("Content-Length", temp);

            body_type_ = content_type::string;
            content_ = std::move(content);
            counter_++;
        }

        void set_chunked() {
            add_header("Transfer-Encoding", "chunked");
        }

        inline std::string to_hex_string(std::size_t value) {
            std::ostringstream stream;
            stream << std::hex << value;
            return stream.str();
        }

        std::vector<boost::asio::const_buffer> to_chunked_buffers(const char *chunk_data, size_t length, bool eof) {
            std::vector<boost::asio::const_buffer> buffers;

            if (length > 0) {
                // convert bytes transferred count to a hex string.
                chunk_size_ = to_hex_string(length);

                // Construct chunk based on rfc2616 section 3.6.1
                buffers.push_back(boost::asio::buffer(chunk_size_));
                buffers.push_back(boost::asio::buffer(crlf));
                buffers.push_back(boost::asio::buffer(chunk_data, length));
                buffers.push_back(boost::asio::buffer(crlf));
            }

            //append last-chunk
            if (eof) {
                buffers.push_back(boost::asio::buffer(last_chunk));
                buffers.push_back(boost::asio::buffer(crlf));
            }

            return buffers;
        }

        std::shared_ptr<session>
        start_session(const std::string &name, std::time_t expire = -1, std::string_view domain = "",
                      const std::string &path = "/") {
            session_ = session_manager::get().create_session(domain, name, expire, path);
            return session_;
        }

        std::shared_ptr<session> start_session() {
            session_ = session_manager::get().create_session(domain_, CSESSIONID);
            return session_;
        }

        void set_domain(std::string_view domain) {
            domain_ = domain;
        }

        std::string_view get_domain() {
            return domain_;
        }

        void set_path(std::string_view path) {
            path_ = path;
        }

        std::string_view get_path() {
            return path_;
        }

        void set_url(std::string_view url) {
            raw_url_ = url;
        }

        std::string_view get_url(std::string_view url) {
            return raw_url_;
        }

        void render_json(const nlohmann::json &json_data) {
#ifdef  CINATRA_ENABLE_GZIP
            set_status_and_content(status_type::ok,json_data.dump(),res_content_type::json,content_encoding::gzip);
#else
            set_status_and_content(status_type::ok, json_data.dump(), res_content_type::json, content_encoding::none);
#endif
        }

        void render_string(std::string &&content) {
#ifdef  CINATRA_ENABLE_GZIP
            set_status_and_content(status_type::ok,std::move(content),res_content_type::string,content_encoding::gzip);
#else
            set_status_and_content(status_type::ok, std::move(content), res_content_type::string,
                                   content_encoding::none);
#endif
        }

        std::vector<std::string> raw_content() {
            return cache_data;
        }

        void redirect(const std::string &url, bool is_forever = false) {
            add_header("Location", url.c_str());
            is_forever == false ? set_status_and_content(status_type::moved_temporarily) : set_status_and_content(
                    status_type::moved_permanently);
        }

        void set_session(std::weak_ptr<session> sessionref) {
            if (sessionref.lock()) {
                session_ = sessionref.lock();
            }
        }

        static int get_counter() {
            return counter_;
        }

        static void increase_counter() {
            counter_++;
        }

        static void reset_counter() {
            counter_ = 0;
        }

    private:
        std::string_view raw_url_;
        std::vector<std::pair<std::string, std::string>> headers_; //http header key可以重复，所以不用map存储
        std::vector<std::string> cache_data;
        std::string content_;
        content_type body_type_ = content_type::unknown;
        status_type status_ = status_type::init;
        bool proc_continue_ = true;
        std::string chunk_size_;
        bool delay_ = false;
        std::string_view domain_;
        std::string_view path_;
        std::shared_ptr<session> session_ = nullptr;
        inline static std::atomic_int counter_ = 0;
    };
}
#endif //SNEEZE_RESPONSE_HPP
