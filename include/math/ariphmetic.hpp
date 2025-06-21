#pragma once
#include <base.hpp>
#include <cassert>
#include <type_traits>
#include "query_base.hpp"

namespace QueryBuilder {

class UnaryOperator : public QueryObject, public Base::Value {
public:
    UnaryOperator() = delete;
    explicit operator std::string() const override {
        std::string retString;
        retString += "(";
        switch (m_operation) {
            case Not:
                retString += "NOT ";
                break;
            default:
                assert(true || "Unsupported unary operation");
                break;
        }
        retString += static_cast<std::string>(*m_object);
        retString += ")";
        return retString;
    }

    enum Operation {
        Not,
    } m_operation;

    ~UnaryOperator() override { delete m_object; }

protected:
    constexpr UnaryOperator(QueryObject* object, Operation operation) : m_object(object) { m_operation = operation; }
    QueryObject* m_object;
};

class PredicateUnaryOperator : public UnaryOperator, public Base::Predicate {
protected:
    constexpr PredicateUnaryOperator(QueryObject* object, Operation operation) : UnaryOperator(object, operation){};

    ~PredicateUnaryOperator() override = default;
    //Fabrics
    template <typename T>
    friend constexpr inline auto Not(T&& object) -> std::enable_if_t<Utility::like_query_v<T>, PredicateUnaryOperator&>;
};

class BinaryOperator : public QueryObject, public Base::Value {
public:
    BinaryOperator() = delete;
    explicit operator std::string() const override {
        std::string retString;
        retString += "(";
        retString += static_cast<std::string>(*m_left);
        switch (m_operation) {
            case Add:
                retString += " + ";
                break;
            case Sub:
                retString += " - ";
                break;
            case Multiply:
                retString += " * ";
                break;
            case Divide:
                retString += " / ";
                break;
            case And:
                retString += " AND ";
                break;
            case Or:
                retString += " OR ";
                break;
            case More:
                retString += " > ";
                break;
            case Less:
                retString += " < ";
                break;
            case MoreEqual:
                retString += " >= ";
                break;
            case LessEqual:
                retString += " <= ";
                break;
            case Equal:
                retString += " = ";
                break;
            case NotEqual:
                retString += " != ";
                break;
            case Is:
                retString += " IS ";
                break;
            case Like:
                retString += " LIKE ";
            case In:
                retString += " IN ";
            default:
                assert(true || "Unsupported binary operator");
        }
        retString += static_cast<std::string>(*m_right);
        retString += ")";
        return retString;
    }

    enum Operation {
        Add,
        Sub,
        Multiply,
        Divide,
        And,
        Or,
        More,
        Less,
        MoreEqual,
        LessEqual,
        Equal,
        NotEqual,
        Is,
        Like,
        In,
    } m_operation;

    ~BinaryOperator() override {
        delete m_left;
        delete m_right;
    }

protected:
    constexpr BinaryOperator(QueryObject* leftObject, QueryObject* rightObject, Operation operation)
        : m_left(leftObject), m_right(rightObject) {
        m_operation = operation;
    }

    QueryObject* m_left;
    QueryObject* m_right;
};

class ArithmeticBinaryOperator : public BinaryOperator {
protected:
    constexpr ArithmeticBinaryOperator(QueryObject* leftObject, QueryObject* rightObject, Operation operation)
        : BinaryOperator(leftObject, rightObject, operation){};

    ~ArithmeticBinaryOperator() override = default;

    //Fabrics
#define FRIEND_FABRIC(func)                                            \
    template <typename T, typename U>                                  \
    friend constexpr inline auto func(T&& leftObject, U&& rightObject) \
        -> std::enable_if_t<Utility::like_query_v<T> && Utility::like_query_v<U>, ArithmeticBinaryOperator&>;
    FRIEND_FABRIC(add)
    FRIEND_FABRIC(sub)
    FRIEND_FABRIC(multiply)
    FRIEND_FABRIC(divide)
#undef FRIEND_FABRIC
};

class PredicateBinaryOperator : public BinaryOperator, public Base::Predicate {
protected:
    constexpr PredicateBinaryOperator(QueryObject* leftObject, QueryObject* rightObject, Operation operation)
        : BinaryOperator(leftObject, rightObject, operation){};

    ~PredicateBinaryOperator() override = default;

    //Fabrics
#define FRIEND_FABRIC(func)                                            \
    template <typename T, typename U>                                  \
    friend constexpr inline auto func(T&& leftObject, U&& rightObject) \
        -> std::enable_if_t<Utility::like_query_v<T> && Utility::like_query_v<U>, PredicateBinaryOperator&>;
    FRIEND_FABRIC(And)
    FRIEND_FABRIC(Or)
    FRIEND_FABRIC(more)
    FRIEND_FABRIC(less)
    FRIEND_FABRIC(moreOrEqual)
    FRIEND_FABRIC(lessOrEqual)
    FRIEND_FABRIC(equal)
    FRIEND_FABRIC(notEqual)
    FRIEND_FABRIC(is)
    FRIEND_FABRIC(like)
    FRIEND_FABRIC(in)
#undef FRIEND_FABRIC
};

class TrinaryOperator : public QueryObject, public Base::Value {
public:
    TrinaryOperator() = delete;
    explicit operator std::string() const override {
        std::string retString;
        retString += "(";
        retString += static_cast<std::string>(*m_object);
        switch (m_operation) {
            case Between:
                retString += " BETWEEN ";
                break;
            default:
                assert(true || "Unsupported trinary operator");
        }
        retString += static_cast<std::string>(*m_left);
        retString += " AND ";
        retString += static_cast<std::string>(*m_right);
        retString += ")";
        return retString;
    }

    enum Operation {
        Between,
    } m_operation;

    ~TrinaryOperator() override {
        delete m_object;
        delete m_left;
        delete m_right;
    }

protected:
    constexpr TrinaryOperator(QueryObject* object, QueryObject* leftObject, QueryObject* rightObject,
                              Operation operation)
        : m_object(object), m_left(leftObject), m_right(rightObject) {
        m_operation = operation;
    }
    QueryObject* m_object;
    QueryObject* m_left;
    QueryObject* m_right;
};

class PredicateTrinaryOperator : public TrinaryOperator, public Base::Predicate {
protected:
    constexpr PredicateTrinaryOperator(QueryObject* object, QueryObject* leftObject, QueryObject* rightObject,
                                       Operation operation)
        : TrinaryOperator(object, leftObject, rightObject, operation){};

    ~PredicateTrinaryOperator() override = default;

    //Fabrics
    template <typename T, typename U1, typename U2>
    friend constexpr inline auto between(T&& object, U1&& left, U2&& right)
        -> std::enable_if_t<Utility::like_query_v<T> && Utility::like_query_v<U1> && Utility::like_query_v<U2>,
                            PredicateTrinaryOperator&>;
};

#define ARITHMETIC_BINARY_OPERATION(name, operation)                                                           \
    template <typename T, typename U>                                                                          \
    [[nodiscard]] constexpr inline auto name(T&& leftObject, U&& rightObject)                                  \
        -> std::enable_if_t<Utility::like_query_v<T> && Utility::like_query_v<U>, ArithmeticBinaryOperator&> { \
        QueryObject* left;                                                                                     \
        QueryObject* right;                                                                                    \
        auto pair = Utility::getPairQueryObject(std::forward<T>(leftObject), std::forward<U>(rightObject));    \
        left = pair.first;                                                                                     \
        right = pair.second;                                                                                   \
        return *new ArithmeticBinaryOperator(left, right, BinaryOperator::operation);                          \
    };
#define ARITHMETIC_BINARY_OPERATOR(name, operation, op)                                                                \
    ARITHMETIC_BINARY_OPERATION(name, operation)                                                                       \
    template <typename T, typename U>                                                                                  \
    [[nodiscard]] constexpr inline auto operator op(T&& first, U&& second)                                             \
        ->std::enable_if_t<(std::is_convertible_v<T&, QueryObject&> && std::is_convertible_v<const U, std::string>) or \
                               (std::is_convertible_v<U&, QueryObject&> &&                                             \
                                std::is_convertible_v<const T, std::string>) or                                        \
                               std::is_convertible_v<T&, QueryObject&> && std::is_convertible_v<U&, QueryObject&>,     \
                           ArithmeticBinaryOperator&> {                                                                \
        return name(std::forward<T>(first), std::forward<U>(second));                                                  \
    }
#define PREDICATE_BINARY_OPERATION(name, operation)                                                           \
    template <typename T, typename U>                                                                         \
    [[nodiscard]] constexpr inline auto name(T&& leftObject, U&& rightObject)                                 \
        -> std::enable_if_t<Utility::like_query_v<T> && Utility::like_query_v<U>, PredicateBinaryOperator&> { \
        QueryObject* left;                                                                                    \
        QueryObject* right;                                                                                   \
        auto pair = Utility::getPairQueryObject(std::forward<T>(leftObject), std::forward<U>(rightObject));   \
        left = pair.first;                                                                                    \
        right = pair.second;                                                                                  \
        return *new PredicateBinaryOperator(left, right, BinaryOperator::operation);                          \
    };
#define PREDICATE_BINARY_OPERATOR(name, operation, op)                                                                 \
    PREDICATE_BINARY_OPERATION(name, operation)                                                                        \
    template <typename T, typename U>                                                                                  \
    [[nodiscard]] constexpr inline auto operator op(T&& first, U&& second)                                             \
        ->std::enable_if_t<(std::is_convertible_v<T&, QueryObject&> && std::is_convertible_v<const U, std::string>) or \
                               (std::is_convertible_v<U&, QueryObject&> &&                                             \
                                std::is_convertible_v<const T, std::string>) or                                        \
                               std::is_convertible_v<T&, QueryObject&> && std::is_convertible_v<U&, QueryObject&>,     \
                           PredicateBinaryOperator&> {                                                                 \
        return name(std::forward<T>(first), std::forward<U>(second));                                                  \
    }
ARITHMETIC_BINARY_OPERATOR(add, Add, +)
ARITHMETIC_BINARY_OPERATOR(sub, Sub, -)
ARITHMETIC_BINARY_OPERATOR(multiply, Multiply, *)
ARITHMETIC_BINARY_OPERATOR(divide, Divide, /)
PREDICATE_BINARY_OPERATOR(And, And, and)
PREDICATE_BINARY_OPERATOR(Or, Or, or)
PREDICATE_BINARY_OPERATOR(more, More, >)
PREDICATE_BINARY_OPERATOR(less, Less, <)
PREDICATE_BINARY_OPERATOR(moreOrEqual, MoreEqual, >=)
PREDICATE_BINARY_OPERATOR(lessOrEqual, LessEqual, <=)
PREDICATE_BINARY_OPERATOR(equal, Equal, ==)
PREDICATE_BINARY_OPERATOR(notEqual, NotEqual, !=)
PREDICATE_BINARY_OPERATION(is, Is)
PREDICATE_BINARY_OPERATION(like, Like)
PREDICATE_BINARY_OPERATION(in, In)

#undef ARITHMETIC_BINARY_OPERATION
#undef ARITHMETIC_BINARY_OPERATOR
#undef PREDICATE_BINARY_OPERATION
#undef PREDICATE_BINARY_OPERATOR

template <typename T>
[[nodiscard]] constexpr inline auto Not(T&& object)
    -> std::enable_if_t<Utility::like_query_v<T>, PredicateUnaryOperator&> {
    auto targetObject = Utility::getQueryObject(std::forward<T>(object));
    return *new PredicateUnaryOperator(targetObject, UnaryOperator::Not);
};
template <typename T>
[[nodiscard]] constexpr inline auto operator not(T&& object)
    -> std::enable_if_t<std::is_convertible_v<T&, QueryObject&>, PredicateUnaryOperator&> {
    return Not(std::forward<T>(object));
}

template <typename T, typename U1, typename U2>
[[nodiscard]] constexpr inline auto between(T&& object, U1&& left, U2&& right)
    -> std::enable_if_t<Utility::like_query_v<T> && Utility::like_query_v<U1> && Utility::like_query_v<U2>,
                        PredicateTrinaryOperator&> {
    auto targetObject = Utility::getQueryObject(std::forward<T>(object));
    auto pair = Utility::getPairQueryObject(std::forward<U1>(left), std::forward<U2>(right));
    return *new PredicateTrinaryOperator(targetObject, pair.first, pair.second, TrinaryOperator::Between);
};

}  // namespace QueryBuilder