#pragma once
#include <base.hpp>
#include <type_traits>
#include "query_base.hpp"

namespace QueryBuilder {
class AggregateFunction : public QueryObject, public Base::Field {
public:
    AggregateFunction() = delete;
    explicit operator std::string() const override {
        std::string retString;
        switch (m_operation) {
            case Avg:
                retString += "AVG";
                break;
            case Count:
                retString += "COUNT";
                break;
            case Min:
                retString += "MIN";
                break;
            case Max:
                retString += "MAX";
                break;
            case Sum:
                retString += "SUM";
                break;
        }
        retString += "(";
        retString += static_cast<std::string>(*m_object);
        retString += ")";
        return retString;
    }

    ~AggregateFunction() override { delete m_object; }

private:
    enum Operation {
        Avg,
        Count,
        Min,
        Max,
        Sum,
    };
    explicit constexpr AggregateFunction(QueryObject* object, Operation operation)
        : m_object(object), m_operation(operation) {}

    QueryObject* m_object;
    Operation m_operation;

//Fabrics
#define FRIEND_FABRIC(name) \
    template <typename T>   \
    friend auto name(T&& object) -> std::enable_if_t<Utility::like_query_v<T>, AggregateFunction&>;
    FRIEND_FABRIC(avg);
    FRIEND_FABRIC(count);
    FRIEND_FABRIC(min);
    FRIEND_FABRIC(max);
    FRIEND_FABRIC(sum);
#undef FRIEND_FABRIC
};

#define AGGREGATE_FUNCTION(name, type)                                                                      \
    template <typename T>                                                                                   \
    [[nodiscard]] auto name(T&& object) -> std::enable_if_t<Utility::like_query_v<T>, AggregateFunction&> { \
        QueryObject* targetObject = Utility::getQueryObject(std::forward<T>(object));                       \
        return *new AggregateFunction(targetObject, AggregateFunction::type);                               \
    }
AGGREGATE_FUNCTION(avg, Avg)
AGGREGATE_FUNCTION(count, Count)
AGGREGATE_FUNCTION(min, Min)
AGGREGATE_FUNCTION(max, Max)
AGGREGATE_FUNCTION(sum, Sum)

#undef AGGREGATE_FUNCTION

}  // namespace QueryBuilder