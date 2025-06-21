#pragma once
#include <string>
#include <type_traits>
#include "base.hpp"
#include "utility.hpp"

namespace QueryBuilder {

class QueryObject {
public:
    QueryObject() = default;
    explicit constexpr QueryObject(const std::string& name) : m_name(Utility::fixString(name)) {}
    [[nodiscard]] constexpr const std::string& getName() const { return m_name; };
    constexpr QueryObject& setName(const std::string& name) {
        m_name = Utility::fixString(name);
        return *this;
    };
    virtual explicit operator std::string() const = 0;

    virtual ~QueryObject() = default;

private:
    std::string m_name;
};

class Value : public QueryObject, public Base::Value {
public:
    template <typename T, std::enable_if_t<std::is_convertible_v<T, std::string>, bool> = true>
    explicit constexpr Value(const T& str) : m_str("\'" + Utility::fixString(str) + "\'"){};

    template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
    explicit constexpr Value(T val) : m_str(std::to_string(val)) {}

    explicit constexpr Value(bool val) : m_str((std::string) + (val ? "TRUE" : "FALSE")) {}

    explicit constexpr operator std::string() const override { return m_str; }

    ~Value() override = default;

private:
    std::string m_str;
};

class RawQuery : public QueryObject {
public:
    explicit constexpr RawQuery() = default;
    explicit constexpr RawQuery(const std::string& str) noexcept : m_str(str){};
    explicit constexpr RawQuery(const char* str) noexcept : m_str(str){};
    explicit constexpr operator std::string() const override { return m_str; }

    ~RawQuery() override = default;

private:
    std::string m_str;
};

static constinit RawQuery Null{"NULL"};


//Like some_table.id
class TableField : public QueryObject, public Base::Field {
public:
    explicit constexpr TableField(const std::string& fieldName) { this->fieldName = Utility::fixString(fieldName); };
    explicit constexpr TableField(const char* fieldName) { this->fieldName = Utility::fixString(fieldName); }
    constexpr TableField(const std::string& tableName, const std::string& fieldName) {
        this->tableName = Utility::fixString(tableName);
        this->fieldName = Utility::fixString(fieldName);
    };
    explicit constexpr operator std::string() const override {
        std::string str;
        if (!tableName.empty()) {
            str += "\"";
            str += tableName;
            str += "\"";
            str += ".";
        }
        str += "\"";
        str += fieldName;
        str += "\"";
        return str;
    }

    ~TableField() override = default;

private:
    std::string tableName;
    std::string fieldName;
};

namespace Utility {


template <typename T, typename Void = void>
struct like_query : std::false_type {};

template <typename T>
struct like_query<
    T, std::enable_if_t<std::is_convertible_v<const T&, const QueryObject&> || Utility::like_string_v<T>, void>>
    : std::true_type {};
template <typename T>
constexpr bool like_query_v = like_query<T>::value;


template <typename T>
static inline constexpr auto getQueryObject(T&& object) -> std::enable_if_t<like_query_v<T>, QueryObject*>;

}  // namespace Utility



template <typename T, std::enable_if_t<std::is_convertible_v<const T&, const QueryObject&>, bool> = true>
class UnverifiedQueryType : public QueryObject {
public:
    typedef std::remove_reference_t<T> Type;

private:
    std::conditional_t<std::is_lvalue_reference_v<T>, Type, RawQuery>* m_object;


    static decltype(m_object) getObject(Type& object) { return &object; }
    static decltype(m_object) getObject(Type&& object) { return new RawQuery(static_cast<std::string>(object)); }

public:
    explicit constexpr UnverifiedQueryType(T&& base) { m_object = getObject(std::forward<T>(base)); }
    operator auto &() { return *m_object; };
    explicit operator std::string() const override { return static_cast<std::string>(*m_object); }
    ~UnverifiedQueryType() override { delete m_object; }
};

template <typename T>
auto UnverifiedQuery(T&& object)
    -> std::enable_if_t<Utility::like_query_v<T>, UnverifiedQueryType<decltype(std::forward<T>(object))>> {
    return UnverifiedQueryType<decltype(std::forward<T>(object))>(std::forward<T>(object));
};



namespace Utility {

template <typename T>
consteval bool _isUnverifiedQuery(UnverifiedQueryType<T>&&) {
    return true;
}
template <typename T, typename Void = void>
struct is_UnverifiedQuery : std::false_type {};
template <typename T>
struct is_UnverifiedQuery<T, std::void_t<decltype(_isUnverifiedQuery(std::declval<T>()))>> : std::true_type {};
template <typename T>
constexpr bool is_UnverifiedQuery_v = is_UnverifiedQuery<T>::value;



template <typename T, typename Void = void>
struct like_table : std::false_type {};
template <typename T>
struct like_table<
    T, std::enable_if_t<
           like_query_v<T> && (std::is_convertible_v<const T&, const Base::Table&> || is_UnverifiedQuery_v<T>), void>>
    : std::true_type {};
template <typename T>
constexpr bool like_table_v = like_table<T>::value;



template <typename T, typename Void = void>
struct like_field : std::false_type {};
template <typename T>
struct like_field<
    T, std::enable_if_t<
           like_query_v<T> && (std::is_convertible_v<const T&, const Base::Field&> || is_UnverifiedQuery_v<T>), void>>
    : std::true_type {};
template <typename T>
constexpr bool like_field_v = like_field<T>::value;



template <typename T, typename Void = void>
struct like_value : std::false_type {};
template <typename T>
struct like_value<
    T, std::enable_if_t<
           like_query_v<T> && (std::is_convertible_v<const T&, const Base::Value&> || is_UnverifiedQuery_v<T>), void>>
    : std::true_type {};
template <typename T>
constexpr bool like_value_v = like_value<T>::value;



template <typename T, typename Void = void>
struct like_predicate : std::false_type {};
template <typename T>
struct like_predicate<T, std::enable_if_t<like_query_v<T> && (std::is_convertible_v<const T&, const Base::Predicate&> ||
                                                              is_UnverifiedQuery_v<T>),
                                          void>> : std::true_type {};
template <typename T>
constexpr bool like_predicate_v = like_predicate<T>::value;



template <typename T>
static inline constexpr auto castToQueryObject(T&& object) -> std::enable_if_t<like_query_v<T>, QueryObject*> {
    return new RawQuery(static_cast<std::string>(object));
}

template <typename T>
static inline constexpr auto castToQueryObject(T& object) -> std::enable_if_t<like_query_v<T>, QueryObject*> {
    return &object;
}

template <typename T, typename U>
static inline constexpr auto getPairQueryObject(T&& leftObject, U&& rightObject)
    -> std::enable_if_t<like_query_v<T> && like_query_v<U>, std::pair<QueryObject*, QueryObject*>> {
    std::pair<QueryObject*, QueryObject*> retPair;
    if constexpr (std::is_convertible_v<T&, QueryObject&>) {
        retPair.first = castToQueryObject(std::forward<T>(leftObject));
    } else {
        retPair.first = new TableField(leftObject);
    }
    if constexpr (std::is_convertible_v<U&, QueryObject&>) {
        retPair.second = castToQueryObject(std::forward<U>(rightObject));
    } else {
        retPair.second = new TableField(rightObject);
    }
    return retPair;
}

template <typename T>
static inline constexpr auto getQueryObject(T&& object) -> std::enable_if_t<like_query_v<T>, QueryObject*> {
    QueryObject* retObject;
    if constexpr (std::is_convertible_v<T&, QueryObject&>) {
        retObject = castToQueryObject(std::forward<T>(object));
    } else {
        retObject = new TableField(object);
    }
    return retObject;
}

}  // namespace Utility

}  // namespace QueryBuilder