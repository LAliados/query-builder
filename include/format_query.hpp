#pragma once
#include <list>
#include <stdexcept>
#include <type_traits>
#include "query_base.hpp"

namespace QueryBuilder {

class FormatRawQuery : public RawQuery {
public:
    template <typename... Ts>
    explicit constexpr FormatRawQuery(const char* format, Ts&&... values) : m_format(format) {
        m_proccessFormatting(m_format.begin(), values...);
    }

    explicit operator std::string() const override {
        std::string ret;
        auto bufferIt = m_buffer.begin();
        auto objectsIt = m_objects.begin();
        for (; bufferIt != m_buffer.end() && objectsIt != m_objects.end(); bufferIt++, objectsIt++) {
            ret += *bufferIt;
            ret += static_cast<std::string>(**objectsIt);
        }
        for (; bufferIt != m_buffer.end(); bufferIt++) {
            ret += *bufferIt;
        }
        for (; objectsIt != m_objects.end(); objectsIt++) {
            ret += static_cast<std::string>(**objectsIt);
        }
        return ret;
    }

    ~FormatRawQuery() override {
        for (auto ptr : m_objects) {
            delete ptr;
        }
    }

private:
    template <typename... Ts>
    constexpr void m_proccessFormatting(std::string::iterator current_iterator, Ts&&... values) {
        for (auto it = current_iterator; it != m_format.end(); it++) {
            if (*it == '{') {
                if (it + 1 == m_format.end()) {
                    throw std::invalid_argument("Incorrect format line");
                }
                if (*(it + 1) == '}') {
                    m_buffer.emplace_back(current_iterator, it);
                    m_formatQuery(it + 2, values...);
                    return;
                } else if (*(it + 1) == '{') {
                    it++;
                    continue;
                } else {
                    throw std::invalid_argument("Incorrect format line");
                }
            } else if (*it == '}') {
                if (it + 1 == m_format.end()) {
                    throw std::invalid_argument("Incorrect format line");
                }
                if (*(it + 1) == '}') {
                    it++;
                    continue;
                } else {
                    throw std::invalid_argument("Incorrect format line");
                }
            }
        }
        if constexpr (sizeof...(values) > 0) {
            throw std::invalid_argument("Incorrect format line");
        }
        m_buffer.emplace_back(current_iterator, m_format.end());
    }
    template <typename T, typename... Ts>
    constexpr auto m_formatQuery(std::string::iterator current_iterator, T&& value, Ts&&... values)
        -> std::enable_if_t<std::is_convertible_v<const T&, std::string>, void> {
        m_objects.push_back(Utility::getQueryObject(value));
        m_proccessFormatting(current_iterator, values...);
    };
    template <typename T, typename... Ts>
    constexpr auto m_formatQuery(std::string::iterator current_iterator, T&& value, Ts&&... values)
        -> std::enable_if_t<std::is_convertible_v<const T&, const QueryObject&>> {
        m_objects.push_back(Utility::getQueryObject(std::forward<T>(value)));
        m_proccessFormatting(current_iterator, values...);
    }

    template <typename T, typename... Ts>
    constexpr auto m_formatQuery(std::string::iterator current_iterator, T&& value, Ts&&... values)
        -> std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, void> {
        m_objects.push_back(Utility::getQueryObject(Value(value)));
        m_proccessFormatting(current_iterator, values...);
    };

    static void m_formatQuery(std::string::iterator current_iterator) {
        throw std::invalid_argument("Incorrect format line");
    }
    std::list<std::string> m_buffer;
    std::list<QueryObject*> m_objects;
    std::string m_format;
};

}  // namespace QueryBuilder