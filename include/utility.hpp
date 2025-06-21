#pragma once
#include <string>
#include <type_traits>

namespace QueryBuilder::Utility {
static constexpr std::string fixString(const std::string& str) {
    std::string result;
    result.reserve(str.length() * 2);

    for (char c : str) {
        switch (c) {
            case '\'':
                result += "''";
                break;
            case '\"':
                result += "\"\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '%':
                result += "\\%";
                break;
            case '_':
                result += "\\_";
                break;
            default:
                result += c;
        }
    }

    return result;
}

template <typename T, typename Void = void>
struct is_iterable : std::false_type {};

template <typename T>
struct is_iterable<T, std::void_t<decltype(std::declval<T>().begin() != std::declval<T>().end())>> : std::true_type {};

template <typename T>
constexpr bool is_iterable_v = is_iterable<T>::value;



template <typename T, typename Void = void>
struct like_string : std::false_type {};

template <typename T>
struct like_string<T, std::enable_if_t<std::is_convertible_v<T, std::string>, void>> : std::true_type {};

template <typename T>
constexpr bool like_string_v = like_string<T>::value;

}  // namespace QueryBuilder::Utility