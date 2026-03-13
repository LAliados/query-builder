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
    enum Type {
        Distinct,
        All,
    };

    explicit operator std::string() const override {
        std::string ret;

        m_appendClause(ret, static_cast<std::string>(m_select));
        m_appendClause(ret, static_cast<std::string>(m_from));
        m_appendClause(ret, static_cast<std::string>(m_where));
        m_appendClause(ret, static_cast<std::string>(m_groupBy));
        m_appendClause(ret, static_cast<std::string>(m_having));
        m_appendClause(ret, static_cast<std::string>(m_orderBy));
        m_appendClause(ret, static_cast<std::string>(m_limit));
        m_appendClause(ret, static_cast<std::string>(m_offset));

        return ret;
    }

    class SelectOperator {
    public:
        explicit operator std::string() const {
            std::string str;
            str += "SELECT ";
            if (m_isDistinct) {
                str += "DISTINCT ";
            }
            if (m_isAllSelected || m_fields.empty()) {
                str += "*";
                return str;
            }
            for (auto it = m_fields.begin(); it != m_fields.end(); ++it) {
                str += '(';
                str += static_cast<std::string>(**it);
                str += ')';
                if (!(**it).getName().empty()) {
                    str += " AS \"";
                    str += (**it).getName();
                    str += "\"";
                }
                if (std::next(it) != m_fields.end()) {
                    str += ", ";
                }
            }
            return str;
        }

        template <typename T>
        auto operator<<(T&& field) -> std::enable_if_t<std::is_convertible_v<T, std::string> || Utility::like_field_v<T> ||
                                                           Utility::like_value_v<T>,
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
                m_appendField(str, *field);
                str += " CROSS JOIN ";
            }
            m_appendField(str, *(--m_fields.end()));
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
        static void m_appendField(std::string& str, const std::pair<bool, QueryObject*>& field) {
            if (field.first) {
                str += static_cast<std::string>(*field.second);
            } else {
                str += '(';
                str += static_cast<std::string>(*field.second);
                str += ')';
            }
            if (!field.second->getName().empty()) {
                str += " AS \"";
                str += field.second->getName();
                str += "\"";
            }
        }
        std::list<std::pair<bool, QueryObject*>> m_fields;
    };

    class WhereOperator {
    public:
        explicit operator std::string() const {
            if (m_predicates.empty()) {
                return "";
            }
            std::string str = "WHERE ";
            for (auto it = m_predicates.begin(); it != m_predicates.end(); ++it) {
                str += '(';
                str += static_cast<std::string>(**it);
                str += ')';
                if (std::next(it) != m_predicates.end()) {
                    str += " AND ";
                }
            }
            return str;
        }

        template <typename T>
        auto operator<<(T&& predicate)
            -> std::enable_if_t<std::is_convertible_v<T, std::string> || Utility::like_predicate_v<T>, decltype(*this)> {
            m_predicates.push_back(Utility::getQueryObject(std::forward<T>(predicate)));
            return *this;
        }

        template <typename T>
        auto operator<<(T&& iterableObject)
            -> std::enable_if_t<Utility::is_iterable_v<T> &&
                                    (std::is_convertible_v<decltype(*iterableObject.begin()), std::string> ||
                                     Utility::like_predicate_v<decltype(*iterableObject.begin())>),
                                decltype(*this)> {
            for (auto it = iterableObject.begin(); it != iterableObject.end(); it++) {
                m_predicates.push_back(Utility::getQueryObject(*it));
            }
            return *this;
        }

        ~WhereOperator() {
            for (auto ptr : m_predicates) {
                delete ptr;
            }
        }

    private:
        std::list<QueryObject*> m_predicates;
    };

    class GroupByOperator {
    public:
        explicit operator std::string() const {
            if (m_fields.empty()) {
                return "";
            }
            std::string str = "GROUP BY ";
            for (auto it = m_fields.begin(); it != m_fields.end(); ++it) {
                str += static_cast<std::string>(**it);
                if (std::next(it) != m_fields.end()) {
                    str += ", ";
                }
            }
            return str;
        }

        template <typename T>
        auto operator<<(T&& field) -> std::enable_if_t<std::is_convertible_v<T, std::string> || Utility::like_field_v<T>,
                                                       decltype(*this)> {
            m_fields.push_back(Utility::getQueryObject(std::forward<T>(field)));
            return *this;
        }

        ~GroupByOperator() {
            for (auto ptr : m_fields) {
                delete ptr;
            }
        }

    private:
        std::list<QueryObject*> m_fields;
    };

    class HavingOperator {
    public:
        explicit operator std::string() const {
            if (m_predicates.empty()) {
                return "";
            }
            std::string str = "HAVING ";
            for (auto it = m_predicates.begin(); it != m_predicates.end(); ++it) {
                str += '(';
                str += static_cast<std::string>(**it);
                str += ')';
                if (std::next(it) != m_predicates.end()) {
                    str += " AND ";
                }
            }
            return str;
        }

        template <typename T>
        auto operator<<(T&& predicate)
            -> std::enable_if_t<std::is_convertible_v<T, std::string> || Utility::like_predicate_v<T>, decltype(*this)> {
            m_predicates.push_back(Utility::getQueryObject(std::forward<T>(predicate)));
            return *this;
        }

        ~HavingOperator() {
            for (auto ptr : m_predicates) {
                delete ptr;
            }
        }

    private:
        std::list<QueryObject*> m_predicates;
    };

    class OrderByOperator {
    public:
        explicit operator std::string() const {
            if (m_fields.empty()) {
                return "";
            }
            std::string str = "ORDER BY ";
            for (auto it = m_fields.begin(); it != m_fields.end(); ++it) {
                str += static_cast<std::string>(*it->first);
                str += it->second ? " ASC" : " DESC";
                if (std::next(it) != m_fields.end()) {
                    str += ", ";
                }
            }
            return str;
        }

        template <typename T>
        auto operator<<(T&& field) -> std::enable_if_t<std::is_convertible_v<T, std::string> || Utility::like_field_v<T>,
                                                       decltype(*this)> {
            return asc(std::forward<T>(field));
        }

        template <typename T>
        auto asc(T&& field) -> std::enable_if_t<std::is_convertible_v<T, std::string> || Utility::like_field_v<T>,
                                                decltype(*this)> {
            m_fields.push_back({Utility::getQueryObject(std::forward<T>(field)), true});
            return *this;
        }

        template <typename T>
        auto desc(T&& field) -> std::enable_if_t<std::is_convertible_v<T, std::string> || Utility::like_field_v<T>,
                                                 decltype(*this)> {
            m_fields.push_back({Utility::getQueryObject(std::forward<T>(field)), false});
            return *this;
        }

        ~OrderByOperator() {
            for (auto& pair : m_fields) {
                delete pair.first;
            }
        }

    private:
        std::list<std::pair<QueryObject*, bool>> m_fields;
    };

    class LimitOperator {
    public:
        void set(size_t limit) {
            m_limit = limit;
            m_hasValue = true;
        }
        explicit operator std::string() const {
            if (!m_hasValue) {
                return "";
            }
            return "LIMIT " + std::to_string(m_limit);
        }

    private:
        size_t m_limit = 0;
        bool m_hasValue = false;
    };

    class OffsetOperator {
    public:
        void set(size_t offset) {
            m_offset = offset;
            m_hasValue = true;
        }
        explicit operator std::string() const {
            if (!m_hasValue) {
                return "";
            }
            return "OFFSET " + std::to_string(m_offset);
        }

    private:
        size_t m_offset = 0;
        bool m_hasValue = false;
    };

    SelectOperator& select() { return m_select; }
    FromOperator& from() { return m_from; }
    WhereOperator& where() { return m_where; }
    GroupByOperator& groupBy() { return m_groupBy; }
    HavingOperator& having() { return m_having; }
    OrderByOperator& orderBy() { return m_orderBy; }
    SelectQuery& limit(size_t value) {
        m_limit.set(value);
        return *this;
    }
    SelectQuery& offset(size_t value) {
        m_offset.set(value);
        return *this;
    }

private:
    static void m_appendClause(std::string& target, const std::string& clause) {
        if (clause.empty()) {
            return;
        }
        if (!target.empty()) {
            target += " ";
        }
        target += clause;
    }

    SelectOperator m_select;
    FromOperator m_from;
    WhereOperator m_where;
    GroupByOperator m_groupBy;
    HavingOperator m_having;
    OrderByOperator m_orderBy;
    LimitOperator m_limit;
    OffsetOperator m_offset;
};

inline std::ostream& operator<<(std::ostream& cout, const SelectQuery& query) {
    cout << static_cast<std::string>(query);
    return cout;
}

}  // namespace QueryBuilder
