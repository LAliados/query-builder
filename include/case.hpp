#pragma once
#include <type_traits>
#include <utility>
#include <vector>
#include "query_base.hpp"
#include "utility.hpp"

namespace QueryBuilder {
class Case : public QueryObject, public Base::Value {
public:
    template <typename T, std::enable_if<Utility::is_iterable_v<T> &&
                                             Utility::like_predicate_v<decltype(std::declval<T>().begin()->first)> &&
                                             Utility::like_query_v<decltype(std::declval<T>().begin()->second)>,
                                         bool> = true>
    explicit constexpr Case(T&& iterableObject) {
        for (auto it = iterableObject.begin(); it != iterableObject.end(); it++) {
            m_cases.push_back(Utility::getPairQueryObject(it->first, it->second));
        }
    }

    template <typename T, typename U,
              std::enable_if<
                  Utility::is_iterable_v<T> && Utility::like_query_v<decltype(std::declval<T>().begin()->first)> &&
                      Utility::like_query_v<decltype(std::declval<T>().begin()->second)> && Utility::like_query_v<U>,
                  bool> = true>
    explicit constexpr Case(T&& iterableObject, U&& elseObject) : Case(std::forward<T>(iterableObject)) {
        m_elseObject = &Utility::getQueryObject(std::forward<U>(elseObject));
    }

    template <typename T, typename U,
              std::enable_if_t<Utility::like_query_v<T> && Utility::like_query_v<U>, bool> = true>
    explicit constexpr Case(T&& whenObject, U&& thenObject) {
        m_cases.push_back(Utility::getPairQueryObject(std::forward<T>(whenObject), std::forward<U>(thenObject)));
    }

    template <
        typename T, typename U, typename K,
        std::enable_if_t<Utility::like_query_v<T> && Utility::like_query_v<U> && Utility::like_query_v<K>, bool> = true>
    explicit Case(T&& whenObject, U&& thenObject, K&& elseObject)
        : Case(std::forward<T>(whenObject), std::forward<U>(thenObject)) {
        m_elseObject = Utility::getQueryObject(std::forward<K>(elseObject));
    }

    explicit operator std::string() const override {
        std::string retString;
        if (m_cases.empty()) {
            return retString;
        }
        retString += "CASE";
        for (auto& pair : m_cases) {
            retString += " WHEN ";
            retString += static_cast<std::string>(*pair.first);
            retString += " THEN ";
            retString += static_cast<std::string>(*pair.second);
        }
        if (m_elseObject) {
            retString += " ELSE ";
            retString += static_cast<std::string>(*m_elseObject);
        }
        retString += " END";
        return retString;
    }

    ~Case() override {
        for (auto& pair : m_cases) {
            delete pair.first;
            delete pair.second;
        }
        delete m_elseObject;
    }

private:
    std::vector<std::pair<QueryObject*, QueryObject*>> m_cases;
    QueryObject* m_elseObject = nullptr;
};
}  // namespace QueryBuilder