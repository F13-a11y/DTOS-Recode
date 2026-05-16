#pragma once
#include <string>
#include <sstream>
#include <bitset>
#include <iomanip>

// ====================== 32-bit ======================

// bin -> int (32-bit)
inline int bin2int32(const std::string& binStr) {
    int value = 0;
    for (char c : binStr) {
        value <<= 1;
        if (c == '1') value += 1;
    }
    return value;
}

// int -> bin (32-bit, leading zeros)
inline std::string int2bin32(int value) {
    std::bitset<32> bits(value);
    return bits.to_string();
}

// int -> bin (32-bit, trimmed)
inline std::string int2bin32_trim(int value) {
    std::bitset<32> bits(value);
    std::string s = bits.to_string();
    return s.erase(0, s.find_first_not_of('0'));
}

// int -> hex (32-bit, leading zeros)
inline std::string int2hex32(int value) {
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setw(8) << std::setfill('0') << std::hex << value;
    return ss.str();
}

// int -> hex (32-bit, normal)
inline std::string int2hex32_trim(int value) {
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::hex << value;
    return ss.str();
}

// hex -> int (32-bit)
inline int hex2int32(const std::string& hexStr) {
    int value;
    std::stringstream ss;
    ss << std::hex << hexStr;
    ss >> value;
    return value;
}

// hex -> bin (32-bit)
inline std::string hex2bin32(const std::string& hexStr) {
    int value = hex2int32(hexStr);
    return int2bin32(value);
}

// bin -> hex (32-bit)
inline std::string bin2hex32(const std::string& binStr) {
    int value = bin2int32(binStr);
    return int2hex32_trim(value);
}

// ====================== 64-bit ======================

// bin -> int (64-bit)
inline long long bin2int64(const std::string& binStr) {
    long long value = 0;
    for (char c : binStr) {
        value <<= 1;
        if (c == '1') value += 1;
    }
    return value;
}

// int -> bin (64-bit, leading zeros)
inline std::string int2bin64(long long value) {
    std::bitset<64> bits(value);
    return bits.to_string();
}

// int -> bin (64-bit, trimmed)
inline std::string int2bin64_trim(long long value) {
    std::bitset<64> bits(value);
    std::string s = bits.to_string();
    return s.erase(0, s.find_first_not_of('0'));
}

// int -> hex (64-bit, leading zeros)
inline std::string int2hex64(long long value) {
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::setw(16) << std::setfill('0') << std::hex << value;
    return ss.str();
}

// int -> hex (64-bit, normal)
inline std::string int2hex64_trim(long long value) {
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::hex << value;
    return ss.str();
}

// hex -> int (64-bit)
inline long long hex2int64(const std::string& hexStr) {
    long long value;
    std::stringstream ss;
    ss << std::hex << hexStr;
    ss >> value;
    return value;
}

// hex -> bin (64-bit)
inline std::string hex2bin64(const std::string& hexStr) {
    long long value = hex2int64(hexStr);
    return int2bin64(value);
}

// bin -> hex (64-bit)
inline std::string bin2hex64(const std::string& binStr) {
    long long value = bin2int64(binStr);
    return int2hex64_trim(value);
}
