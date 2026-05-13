#ifndef MSNGR__PROFILE__UTILS__JSON_UTILS_HPP_
#define MSNGR__PROFILE__UTILS__JSON_UTILS_HPP_

#include <nlohmann/json.hpp>
#include <chrono>
#include <string>
#include <vector>
#include <cstdint>

namespace msngr::profile::utils {

inline std::chrono::system_clock::time_point ParseTimestamp(const std::string& timestamp) {
    std::tm tm = {};
    std::istringstream ss(timestamp);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail()) {
        throw std::runtime_error("Invalid timestamp format");
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

inline std::string FormatTimestamp(const std::chrono::system_clock::time_point& tp) {
    auto time_t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm;
    gmtime_r(&time_t, &tm);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

inline std::vector<uint8_t> DecodeBase64(const std::string& encoded) {
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::vector<uint8_t> decoded;
    int val = 0, valb = -6;
    
    for (uint8_t c : encoded) {
        if (c == '=') break;
        size_t pos = base64_chars.find(c);
        if (pos == std::string::npos) continue;
        
        val = (val << 6) + pos;
        valb += 6;
        if (valb >= 0) {
            decoded.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    
    return decoded;
}

} // namespace msngr::profile::utils

#endif  // MSNGR__PROFILE__UTILS__JSON_UTILS_HPP_