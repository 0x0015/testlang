#include "parseUtil.hpp"

std::vector<std::string_view> strSplit(std::string_view s, std::string_view delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string_view token;
    std::vector<std::string_view> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string_view::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

