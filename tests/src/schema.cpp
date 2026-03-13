#include <gtest/gtest.h>

#include <math/ariphmetic.hpp>
#include <schema.hpp>
#include <select_query.hpp>

using namespace QueryBuilder;

namespace {

struct UsersTableName {
    static constexpr std::string_view value = "users";
};
struct SessionsTableName {
    static constexpr std::string_view value = "sessions";
};

struct IdName {
    static constexpr std::string_view value = "id";
};
struct UserIdName {
    static constexpr std::string_view value = "user_id";
};
struct EmailName {
    static constexpr std::string_view value = "email";
};
struct MetaName {
    static constexpr std::string_view value = "meta";
};

struct BigIntType {
    static constexpr std::string_view value = "BIGINT";
};
struct TextType {
    static constexpr std::string_view value = "TEXT";
};
struct JsonbType {
    static constexpr std::string_view value = "JSONB";
};

struct JsonDefault {
    static constexpr std::string_view value = "'{}'::jsonb";
};
struct JsonCheck {
    static constexpr std::string_view value = "jsonb_typeof(meta) = 'object'";
};
struct FillFactorKey {
    static constexpr std::string_view value = "fillfactor";
};
struct FillFactorVal {
    static constexpr std::string_view value = "80";
};
struct HashPartitionExpr {
    static constexpr std::string_view value = "HASH (\"user_id\")";
};

struct Utf8Collation {
    static constexpr std::string_view value = "\"C\"";
};
struct LowerEmailDefault {
    static constexpr std::string_view value = "lower('TEST@EXAMPLE.COM')";
};
struct StoredExpr {
    static constexpr std::string_view value = "id + 1";
};

using UserIdField = Schema::Field<IdName, BigIntType, Schema::PrimaryKey, Schema::GeneratedAlwaysIdentity>;
using UserEmailField = Schema::Field<EmailName, TextType, Schema::NotNull, Schema::Unique>;
using UserTable = Schema::Table<UsersTableName, UserIdField, UserEmailField>;

using SessionUserIdField =
    Schema::Field<UserIdName, BigIntType, Schema::NotNull,
                  Schema::References<UserTable, UserIdField, Schema::RefAction::Cascade, Schema::RefAction::Restrict>>;
using SessionMetaField = Schema::Field<MetaName, JsonbType, Schema::DefaultExpr<JsonDefault>, Schema::CheckExpr<JsonCheck>>;
using SessionsBaseTable = Schema::Table<SessionsTableName, Schema::Field<IdName, BigIntType, Schema::PrimaryKey>,
                                        SessionUserIdField, SessionMetaField>;
using SessionsCreateTable =
    Schema::CreateTableStatement<SessionsBaseTable, Schema::IfNotExists, Schema::Unlogged,
                                 Schema::StorageParam<FillFactorKey, FillFactorVal>,
                                 Schema::OnCommit<Schema::OnCommitAction::PreserveRows>,
                                 Schema::PartitionBy<HashPartitionExpr>>;

using RichField = Schema::Field<EmailName, TextType, Schema::Collate<Utf8Collation>,
                                Schema::DefaultExpr<LowerEmailDefault>, Schema::GeneratedStored<StoredExpr>>;

static_assert(Utility::like_schema_field_v<UserIdField>);
static_assert(std::is_same_v<decltype(UserTable::ref<IdName>()), Schema::FieldReference<UsersTableName, IdName>>);
static_assert(Schema::is_string_tag_v<UsersTableName>);
static_assert(!Schema::is_string_tag_v<int>);
static_assert(Schema::unique_names<IdName, UserIdName, EmailName>::value);

}  // namespace

TEST(SCHEMA_TEST, TEMPLATE_TABLE_AND_REFERENCE_RENDERING) {
    const auto userRefSql = static_cast<std::string>(UserTable::ref<IdName>());
    EXPECT_EQ(userRefSql, "\"users\".\"id\"");

    const auto sessionsSql = static_cast<std::string>(SessionsCreateTable{});
    EXPECT_TRUE(sessionsSql.find("CREATE UNLOGGED TABLE IF NOT EXISTS \"sessions\"") != std::string::npos);
    EXPECT_TRUE(sessionsSql.find("\"user_id\" BIGINT NOT NULL REFERENCES \"users\"(\"id\") MATCH SIMPLE ON DELETE CASCADE ON UPDATE RESTRICT") !=
                std::string::npos);
    EXPECT_TRUE(sessionsSql.find("\"meta\" JSONB DEFAULT '{}'::jsonb CHECK (jsonb_typeof(meta) = 'object')") != std::string::npos);
    EXPECT_TRUE(sessionsSql.find("WITH (fillfactor=80)") != std::string::npos);
    EXPECT_TRUE(sessionsSql.find("ON COMMIT PRESERVE ROWS") != std::string::npos);
    EXPECT_TRUE(sessionsSql.find("PARTITION BY HASH (\"user_id\")") != std::string::npos);
}

TEST(SCHEMA_TEST, FIELD_OPTIONS_RENDERING) {
    const auto richFieldSql = static_cast<std::string>(RichField{});
    EXPECT_TRUE(richFieldSql.find("\"email\" TEXT") != std::string::npos);
    EXPECT_TRUE(richFieldSql.find("COLLATE \"C\"") != std::string::npos);
    EXPECT_TRUE(richFieldSql.find("DEFAULT lower('TEST@EXAMPLE.COM')") != std::string::npos);
    EXPECT_TRUE(richFieldSql.find("GENERATED ALWAYS AS (id + 1) STORED") != std::string::npos);
}


TEST(SCHEMA_TEST, FOREIGN_KEY_RELATION_BETWEEN_TABLES) {
    using UserPkField = Schema::Field<IdName, BigIntType, Schema::PrimaryKey>;
    using Users = Schema::Table<UsersTableName, UserPkField>;

    using SessionFkField =
        Schema::Field<UserIdName, BigIntType, Schema::NotNull,
                      Schema::References<Users, UserPkField, Schema::RefAction::Cascade, Schema::RefAction::Restrict>>;
    using Sessions = Schema::Table<SessionsTableName, Schema::Field<IdName, BigIntType, Schema::PrimaryKey>, SessionFkField>;

    using SessionsDDL = Schema::CreateTableStatement<Sessions, Schema::IfNotExists>;

    static_assert(std::is_same_v<decltype(Users::ref<IdName>()), Schema::FieldReference<UsersTableName, IdName>>);

    const auto ddlSql = static_cast<std::string>(SessionsDDL{});
    EXPECT_TRUE(ddlSql.find("CREATE TABLE IF NOT EXISTS \"sessions\"") != std::string::npos);
    EXPECT_TRUE(ddlSql.find("\"user_id\" BIGINT NOT NULL REFERENCES \"users\"(\"id\") MATCH SIMPLE ON DELETE CASCADE ON UPDATE RESTRICT") !=
                std::string::npos);

    SelectQuery query;
    query.select() << Sessions::ref<UserIdName>() << Users::ref<IdName>();
    query.from() << Sessions{} << Users{};
    query.where() << (Sessions::ref<UserIdName>() == Users::ref<IdName>());

    const auto relationSql = static_cast<std::string>(query);
    EXPECT_TRUE(relationSql.find("WHERE ((\"sessions\".\"user_id\") = (\"users\".\"id\"))") != std::string::npos);
}


TEST(SCHEMA_TEST, LITERAL_BASED_SCHEMA_API_WITHOUT_NAME_STRUCTS) {
    using Users = Schema::TableL<"users", Schema::FieldL<"id", "BIGINT", Schema::PrimaryKey>,
                                 Schema::FieldL<"email", "TEXT", Schema::NotNull, Schema::Unique>>;

    using Sessions = Schema::TableL<
        "sessions", Schema::FieldL<"id", "BIGINT", Schema::PrimaryKey>,
        Schema::FieldL<"user_id", "BIGINT", Schema::NotNull,
                       Schema::References<Users, Schema::FieldL<"id", "BIGINT", Schema::PrimaryKey>,
                                          Schema::RefAction::Cascade, Schema::RefAction::Restrict>>,
        Schema::FieldL<"meta", "JSONB", Schema::DefaultExprL<"'{}'::jsonb">,
                       Schema::CheckExprL<"jsonb_typeof(meta) = 'object'">>>;

    using SessionsDDL = Schema::CreateTableStatement<Sessions, Schema::IfNotExists, Schema::Unlogged,
                                                     Schema::StorageParamL<"fillfactor", "80">,
                                                     Schema::PartitionByL<"HASH (\"user_id\")">>;

    const auto ddlSql = static_cast<std::string>(SessionsDDL{});
    EXPECT_TRUE(ddlSql.find("CREATE UNLOGGED TABLE IF NOT EXISTS \"sessions\"") != std::string::npos);
    EXPECT_TRUE(ddlSql.find("REFERENCES \"users\"(\"id\")") != std::string::npos);

    SelectQuery query;
    query.select() << Users::ref<"id">() << Sessions::ref<"user_id">();
    query.from() << Users{} << Sessions{};
    query.where() << (Users::ref<"id">() == Sessions::ref<"user_id">());

    const auto sql = static_cast<std::string>(query);
    EXPECT_TRUE(sql.find("WHERE ((\"users\".\"id\") = (\"sessions\".\"user_id\"))") != std::string::npos);
}

TEST(SCHEMA_TEST, SELECT_QUERY_INTEGRATION_WITH_SCHEMA_TYPES) {
    SelectQuery query;

    auto userIdRef = UserTable::ref<IdName>();
    userIdRef.setName("uid");

    query.select() << userIdRef << SessionsBaseTable::ref<MetaName>();
    query.from() << UserTable{} << SessionsBaseTable{};
    query.where() << (UserTable::ref<IdName>() == SessionsBaseTable::ref<UserIdName>());
    query.orderBy().desc(SessionsBaseTable::ref<MetaName>());
    query.limit(5);

    const auto sql = static_cast<std::string>(query);

    EXPECT_TRUE(sql.find("SELECT (\"users\".\"id\") AS \"uid\", (\"sessions\".\"meta\")") != std::string::npos);
    EXPECT_TRUE(sql.find("FROM ((\"users\") CROSS JOIN (\"sessions\"))") != std::string::npos);
    EXPECT_TRUE(sql.find("WHERE ((\"users\".\"id\") = (\"sessions\".\"user_id\"))") != std::string::npos);
    EXPECT_TRUE(sql.find("ORDER BY \"sessions\".\"meta\" DESC") != std::string::npos);
    EXPECT_TRUE(sql.find("LIMIT 5") != std::string::npos);
}
