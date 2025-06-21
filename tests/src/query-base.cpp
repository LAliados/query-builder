#include <gtest/gtest.h>
#include <base.hpp>
#include <query_base.hpp>



using namespace QueryBuilder;


TEST(QUERY_BASE_TEST, VALUE_TEST) {
    EXPECT_TRUE(static_cast<std::string>(Value(0)) == "0") << "Value string is: " << static_cast<std::string>(Value(0));

    EXPECT_TRUE(static_cast<std::string>(Value(0.2)) == "0.200000")
        << "Value string is: " << static_cast<std::string>(Value(0.2));

    EXPECT_TRUE(static_cast<std::string>(Value(true)) == "TRUE")
        << "Value string is: " << static_cast<std::string>(Value(true));

    EXPECT_TRUE(static_cast<std::string>(Value(false)) == "FALSE")
        << "Value string is: " << static_cast<std::string>(Value(false));

    EXPECT_TRUE(static_cast<std::string>(Value("Some string")) == "\'Some string\'")
        << "Value string is: " << static_cast<std::string>(Value("Some string"));
}


class TableClass : public QueryObject, public Base::Table {};
class FieldClass : public QueryObject, public Base::Field {};
class ValueClass : public QueryObject, public Base::Value {};
class PredicateClass : public QueryObject, public Base::Predicate {};

TEST(QUERY_BASE_TEST, TYPE_TRAITS_TEST) {
    EXPECT_TRUE(Utility::like_query_v<Value>);
    EXPECT_TRUE(Utility::like_query_v<RawQuery>);
    EXPECT_TRUE(Utility::like_query_v<std::string>);
    EXPECT_TRUE(Utility::like_query_v<const char*>);
    EXPECT_FALSE(Utility::like_query_v<char>);
    EXPECT_FALSE(Utility::like_query_v<int>);
    EXPECT_FALSE(Utility::like_query_v<bool>);
    EXPECT_FALSE(Utility::like_query_v<void>);

    EXPECT_TRUE(Utility::is_UnverifiedQuery_v<decltype(UnverifiedQuery(Value(0)))>);
    EXPECT_TRUE(Utility::is_UnverifiedQuery_v<decltype(UnverifiedQuery(RawQuery("Some string")))>);
    EXPECT_TRUE(Utility::is_UnverifiedQuery_v<decltype(UnverifiedQuery(TableField("Some field")))>);
    EXPECT_FALSE(Utility::is_UnverifiedQuery_v<Value>);
    EXPECT_FALSE(Utility::is_UnverifiedQuery_v<RawQuery>);
    EXPECT_FALSE(Utility::is_UnverifiedQuery_v<int>);
    EXPECT_FALSE(Utility::is_UnverifiedQuery_v<void>);
    EXPECT_FALSE(Utility::is_UnverifiedQuery_v<std::string>);

    EXPECT_TRUE(Utility::like_table_v<TableClass>);
    EXPECT_TRUE(Utility::like_table_v<decltype(UnverifiedQuery(RawQuery("Some string")))>);
    EXPECT_FALSE(Utility::like_table_v<std::string>);
    EXPECT_FALSE(Utility::like_table_v<RawQuery>);

    EXPECT_TRUE(Utility::like_field_v<FieldClass>);
    EXPECT_TRUE(Utility::like_field_v<decltype(UnverifiedQuery(RawQuery("Some string")))>);
    EXPECT_FALSE(Utility::like_field_v<std::string>);
    EXPECT_FALSE(Utility::like_field_v<RawQuery>);

    EXPECT_TRUE(Utility::like_value_v<ValueClass>);
    EXPECT_TRUE(Utility::like_value_v<decltype(UnverifiedQuery(RawQuery("Some string")))>);
    EXPECT_FALSE(Utility::like_value_v<std::string>);
    EXPECT_FALSE(Utility::like_value_v<RawQuery>);

    EXPECT_TRUE(Utility::like_predicate_v<PredicateClass>);
    EXPECT_TRUE(Utility::like_predicate_v<decltype(UnverifiedQuery(RawQuery("Some string")))>);
    EXPECT_FALSE(Utility::like_predicate_v<std::string>);
    EXPECT_FALSE(Utility::like_predicate_v<RawQuery>);
}
