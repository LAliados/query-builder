#pragma once
#include <cassert>
#include <type_traits>
#include "query_base.hpp"
#include "utility.hpp"

namespace QueryBuilder {
class JoinType : public QueryObject, public Base::Table {
    enum Type {
        Inner,
        Full,
        Left,
        Right,
        Cross,
        Natural,
        Self,
    };
    template <typename T, typename U,
              std::enable_if_t<(Utility::like_string_v<T> || Utility::like_table_v<T>) &&
                                   (Utility::like_string_v<U> || Utility::like_table_v<U>),
                               bool> = true>
    JoinType(T&& left, U&& right, Type type) {
        if constexpr (Utility::like_string_v<T>) {
            m_isLeftSimple = true;
        } else {
            m_isLeftSimple = false;
        }
        m_left = Utility::getQueryObject(std::forward<T>(left));
        if constexpr (Utility::like_string_v<U>) {
            m_isRightSimple = true;
        } else {
            m_isRightSimple = false;
        }
        m_right = Utility::getQueryObject(std::forward<U>(right));
        m_type = type;
        m_predicate = nullptr;
    }

    template <typename T, typename U, typename K,
              std::enable_if_t<(Utility::like_string_v<T> || Utility::like_table_v<T>) &&
                                   (Utility::like_string_v<U> || Utility::like_table_v<U>) &&
                                   (Utility::like_string_v<K> || Utility::like_predicate_v<K>),
                               bool> = true>
    JoinType(T&& left, U&& right, K&& predicate, Type type)
        : JoinType(std::forward<T>(left), std::forward<U>(right), type) {
        m_predicate = Utility::getQueryObject(std::forward<K>(predicate));
    }

public:
    JoinType() = delete;
    explicit operator std::string() const override {
        std::string str;
        if (m_isLeftSimple) {
            str += static_cast<std::string>(*m_left);
        } else {
            str += "(";
            str += static_cast<std::string>(*m_left);
            str += ")";
        }
        switch (m_type) {
            case Inner:
                str += " INNER JOIN ";
                break;
            case Full:
                str += " FULL JOIN ";
                break;
            case Left:
                str += " LEFT JOIN ";
                break;
            case Right:
                str += " RIGHT JOIN ";
                break;
            case Cross:
                str += " CROSS JOIN ";
                break;
            case Natural:
                str += " NATURAL JOIN ";
                break;
            case Self:
                str += " SELF JOIN ";
                break;
            default:
                assert(true && "Undefined join type");
        }
        if (m_isRightSimple) {
            str += static_cast<std::string>(*m_right);
        } else {
            str += "(";
            str += static_cast<std::string>(*m_right);
            str += ")";
        }
        if (m_predicate) {
            str += " ON ";
            str += "(";
            str += static_cast<std::string>(*m_predicate);
            str += ")";
        }
        return str;
    }

    ~JoinType() override {
        delete m_left;
        delete m_right;
        delete m_predicate;
    }

private:
    Type m_type;
    QueryObject* m_left;
    bool m_isLeftSimple;
    QueryObject* m_right;
    bool m_isRightSimple;
    QueryObject* m_predicate;

    //Fabrics
#define FRIEND_BASE_JOIN_OPERATOR(name)                                                                          \
    template <typename T, typename U>                                                                            \
    friend auto name(T&& left,                                                                                   \
                     U&& right) -> std::enable_if_t<(Utility::like_string_v<T> || Utility::like_table_v<T>) &&   \
                                                        (Utility::like_string_v<U> || Utility::like_table_v<U>), \
                                                    JoinType&>;
    FRIEND_BASE_JOIN_OPERATOR(crossJoin)
    FRIEND_BASE_JOIN_OPERATOR(naturalJoin)
    FRIEND_BASE_JOIN_OPERATOR(selfJoin)
#undef FRIEND_BASE_JOIN_OPERATOR

#define FRIEND_ADVANCED_JOIN_OPERATOR(name)                                                                 \
    template <typename T, typename U, typename K>                                                           \
    friend auto name(                                                                                       \
        T&& left, U&& right,                                                                                \
        K&& predicate) -> std::enable_if_t<(Utility::like_string_v<T> || Utility::like_table_v<T>) &&       \
                                               (Utility::like_string_v<U> || Utility::like_table_v<U>) &&   \
                                               (Utility::like_string_v<K> || Utility::like_predicate_v<K>), \
                                           JoinType&>;
    FRIEND_ADVANCED_JOIN_OPERATOR(join)
    FRIEND_ADVANCED_JOIN_OPERATOR(innerJoin)
    FRIEND_ADVANCED_JOIN_OPERATOR(leftJoin)
    FRIEND_ADVANCED_JOIN_OPERATOR(rightJoin)
    FRIEND_ADVANCED_JOIN_OPERATOR(fullJoin)
#undef FRIEND_ADVANCED_JOIN_OPERATOR
};

#define BASE_JOIN_OPERATOR(name, type)                                                                        \
    template <typename T, typename U>                                                                         \
    [[nodiscard]] auto name(                                                                                  \
        T&& left, U&& right) -> std::enable_if_t<(Utility::like_string_v<T> || Utility::like_table_v<T>) &&   \
                                                     (Utility::like_string_v<U> || Utility::like_table_v<U>), \
                                                 JoinType&> {                                                 \
        return *new JoinType(std::forward<T>(left), std::forward<U>(right), JoinType::type);                  \
    }
BASE_JOIN_OPERATOR(crossJoin, Cross)
BASE_JOIN_OPERATOR(naturalJoin, Natural)
BASE_JOIN_OPERATOR(selfJoin, Self)
#undef BASE_JOIN_OPERATOR


#define ADVANCED_JOIN_OPERATOR(name, type)                                                                  \
    template <typename T, typename U, typename K>                                                           \
    [[nodiscard]] auto name(                                                                                \
        T&& left, U&& right,                                                                                \
        K&& predicate) -> std::enable_if_t<(Utility::like_string_v<T> || Utility::like_table_v<T>) &&       \
                                               (Utility::like_string_v<U> || Utility::like_table_v<U>) &&   \
                                               (Utility::like_string_v<K> || Utility::like_predicate_v<K>), \
                                           JoinType&> {                                                     \
        return *new JoinType(std::forward<T>(left), std::forward<U>(right), std::forward<K>(predicate),     \
                             JoinType::type);                                                               \
    }
ADVANCED_JOIN_OPERATOR(join, Inner)
ADVANCED_JOIN_OPERATOR(innerJoin, Inner)
ADVANCED_JOIN_OPERATOR(leftJoin, Left)
ADVANCED_JOIN_OPERATOR(rightJoin, Right)
ADVANCED_JOIN_OPERATOR(fullJoin, Full)
#undef ADVANCED_JOIN_OPERATOR
}  // namespace QueryBuilder