//
// Created by fredia on 19-3-29.
//

#ifndef SNEEZE_URL_UTILS_HPP
#define SNEEZE_URL_UTILS_HPP

#include <string_view>
#include <string>
#include <locale>
#include <codecvt>

namespace code_utils {
    inline static std::string url_encode(const std::string &value) noexcept {
        static auto hex_chars = "0123456789ABCDEF";

        std::string result;
        result.reserve(value.size()); // Minimum size of result

        for (auto &chr : value) {
            if (!((chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z') ||
                  chr == '-' || chr == '.' || chr == '_' || chr == '~'))
                result += std::string("%") + hex_chars[static_cast<unsigned char>(chr) >> 4] +
                          hex_chars[static_cast<unsigned char>(chr) & 15];
            else
                result += chr;
        }

        return result;
    }

    inline static std::string url_decode(const std::string &value) noexcept {
        std::string result;
        result.reserve(value.size() / 3 + (value.size() % 3)); // Minimum size of result

        for (std::size_t i = 0; i < value.size(); ++i) {
            auto &chr = value[i];
            if (chr == '%' && i + 2 < value.size()) {
                auto hex = value.substr(i + 1, 2);
                auto decoded_chr = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
                result += decoded_chr;
                i += 2;
            } else if (chr == '+')
                result += ' ';
            else
                result += chr;
        }

        return result;
    }

    inline static std::string u8wstring_to_string(const std::wstring &wstr) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(wstr);
    }

    inline static std::wstring u8string_to_wstring(const std::string &str) {
        std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
        return conv.from_bytes(str);
    }

    inline static std::string get_string_by_urldecode(std::string_view content) {
        return url_decode(std::string(content.data(), content.size()));
    }

    inline static bool is_url_encode(std::string_view str) {
        return str.find("%") != std::string_view::npos || str.find("+") != std::string_view::npos;
    }
}
#endif //SNEEZE_URL_UTILS_HPP
