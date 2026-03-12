#include <gtest/gtest.h>
#include <case.hpp>
#include <math/aggregate.hpp>
#include <math/ariphmetic.hpp>
#include <query_base.hpp>
#include <select_query.hpp>
#include <table_operation.hpp>

using namespace QueryBuilder;

TEST(GENERAL_TEST, COMPLEX_SELECT_QUERY_TEST) {
    SelectQuery query;
    auto salary = max("salary").setName("max_salary");

    query.select() << "name" << TableField("users", "age") << salary;
    query.from() << leftJoin("users", "payments", TableField("users", "id") == TableField("payments", "user_id"));
    query.where() << (TableField("users", "active") == Value(true))
                  << (TableField("users", "country") == Value("RU"));
    query.groupBy() << "name" << TableField("users", "age");
    query.having() << (max("salary") > Value(1000));
    query.orderBy().asc("name").desc(TableField("users", "age"));
    query.limit(50).offset(10);

    const auto queryString = static_cast<std::string>(query);
    EXPECT_TRUE(queryString.find("SELECT") == 0);
    EXPECT_NE(queryString.find("FROM"), std::string::npos);
    EXPECT_NE(queryString.find("WHERE"), std::string::npos);
    EXPECT_NE(queryString.find("GROUP BY"), std::string::npos);
    EXPECT_NE(queryString.find("HAVING"), std::string::npos);
    EXPECT_NE(queryString.find("ORDER BY"), std::string::npos);
    EXPECT_NE(queryString.find("LIMIT 50"), std::string::npos);
    EXPECT_NE(queryString.find("OFFSET 10"), std::string::npos);
}

TEST(GENERAL_TEST, DEFAULT_SELECT_ALL_TEST) {
    SelectQuery query;
    EXPECT_EQ(static_cast<std::string>(query), "SELECT *");
}
