#pragma once
#include <iostream>
#include <list>
#include <string>
#include <type_traits>
#include <utility>
#include "query_base.hpp"
#include "utility.hpp"

namespace QueryBuilder {

class SelectQuery : public QueryObject, public Base::Table {
public:
    explicit operator std::string() const override {
        std::string ret;
        ret += static_cast<std::string>(m_select);

        ret += static_cast<std::string>(m_from);

        return ret;
    }
    enum Type {
        Distinct,
        All,
    };

    class SelectOperator {
    public:
        explicit operator std::string() const {
            std::string str;
            str += "SELECT ";
            if (m_isDistinct) {
                str += "DISTINCT ";
            }
            if (m_isAllSelected) {
                str += "* ";
                return str;
            }
            for (const auto& field : m_fields) {
                str += '(';
                str += static_cast<std::string>(*field);
                str += ')';
                if (!field->getName().empty()) {
                    str += " AS \"";
                    str += field->getName();
                    str += "\"";
                }
                str += ", ";
            }
            if (*(str.end() - 2) == ',')
                *(str.end() - 2) = ' ';
            return str;
        }

        template <typename T>
        auto operator<<(T&& field) -> std::enable_if_t<std::is_convertible_v<T, std::string> ||
                                                           Utility::like_field_v<T> || Utility::like_value_v<T>,
                                                       decltype(*this)> {
            m_fields.push_back(Utility::getQueryObject(std::forward<T>(field)));
            return *this;
        }


        template <typename T>
        auto operator<<(T&& iterableObject)
            -> std::enable_if_t<Utility::is_iterable_v<T> &&
                                    (std::is_convertible_v<decltype(*iterableObject.begin()), std::string> ||
                                     Utility::like_field_v<decltype(*iterableObject.begin())> ||
                                     Utility::like_value_v<decltype(*iterableObject.begin())>),
                                decltype(*this)> {
            for (auto it = iterableObject.begin(); it != iterableObject.end(); it++) {
                m_fields.push_back(Utility::getQueryObject(*it));
            }
            return *this;
        }

        auto operator<<(Type type) -> decltype(*this) {
            switch (type) {
                case Distinct:
                    m_isDistinct = true;
                    break;
                case All:
                    m_isAllSelected = true;
                    break;
            }
            return *this;
        };

        ~SelectOperator() {
            for (auto ptr : m_fields) {
                delete ptr;
            }
        }

    private:
        bool m_isAllSelected = false;
        bool m_isDistinct = false;
        std::list<QueryObject*> m_fields;
    };

    class FromOperator {
    public:
        explicit operator std::string() const {
            std::string str;
            if (m_fields.empty()) {
                return str;
            }
            str += "FROM ";
            if (m_fields.size() > 1) {
                str += "(";
            }
            for (auto field = m_fields.begin(); field != --m_fields.end(); ++field) {
                if (field->first) {
                    str += static_cast<std::string>(*field->second);
                } else {
                    str += '(';
                    str += static_cast<std::string>(*field->second);
                    str += ')';
                }
                if (!field->second->getName().empty()) {
                    str += " AS \"";
                    str += field->second->getName();
                    str += "\"";
                }
                str += " CROSS JOIN ";
            }
            {
                auto field = --m_fields.end();
                if (field->first) {
                    str += static_cast<std::string>(*field->second);
                } else {
                    str += '(';
                    str += static_cast<std::string>(*field->second);
                    str += ')';
                }
                if (!field->second->getName().empty()) {
                    str += " AS \"";
                    str += field->second->getName();
                    str += "\"";
                }
            }
            if (m_fields.size() > 1) {
                str += ")";
            }
            return str;
        }
        template <typename T>
        auto operator<<(T&& field)
            -> std::enable_if_t<std::is_convertible_v<T, std::string> || Utility::like_table_v<T>, decltype(*this)> {
            if constexpr (std::is_convertible_v<T, std::string>) {
                m_fields.push_back({true, Utility::getQueryObject(std::forward<T>(field))});
            } else {
                m_fields.push_back({false, Utility::getQueryObject(std::forward<T>(field))});
            }
            return *this;
        }


        template <typename T>
        auto operator<<(T&& iterableObject)
            -> std::enable_if_t<Utility::is_iterable_v<T> &&
                                    (std::is_convertible_v<decltype(*iterableObject.begin()), std::string> ||
                                     Utility::like_table_v<decltype(*iterableObject.begin())>),
                                decltype(*this)> {
            for (auto it = iterableObject.begin(); it != iterableObject.end(); it++) {
                if constexpr (std::is_convertible_v<decltype(*iterableObject.begin()), std::string>) {
                    m_fields.push_back({true, Utility::getQueryObject(*it)});
                } else {
                    m_fields.push_back({false, Utility::getQueryObject(*it)});
                }
            }
            return *this;
        }

        ~FromOperator() {
            for (auto& pair : m_fields) {
                delete pair.second;
            }
        }

    private:
        std::list<std::pair<bool, QueryObject*>> m_fields;
    };



    SelectOperator& select() { return m_select; }
    FromOperator& from() { return m_from; }


private:
    void m_pasteSelect(std::string& str) const;
    SelectOperator m_select;
    FromOperator m_from;
};

std::ostream& operator<<(std::ostream& cout, const SelectQuery& query) {
    cout << static_cast<std::string>(query);
    return cout;
}

}  // namespace QueryBuilder