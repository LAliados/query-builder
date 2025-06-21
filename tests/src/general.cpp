#include <gtest/gtest.h>
#include <case.hpp>
#include <math/aggregate.hpp>
#include <math/ariphmetic.hpp>
#include <query_base.hpp>
#include <select_query.hpp>
#include <table_operation.hpp>
using namespace QueryBuilder;

TEST(GENERAL_TEST, GENERAL_TEST) {
    SelectQuery query;
    query.select() << "Field1" << TableField("Table1", "Field1") << Value(1) << Value("Value1")
                   << Case(Value(1) < Value(2), "Field2", "Field3") << max("Field4") << add("Field5", "Field6")
                   << SelectQuery::Distinct;
    query.from() << leftJoin("Table1", "Table2", TableField("Table1", "id") == TableField("Table2", "id"));
    std::cout << query << '\n';
}