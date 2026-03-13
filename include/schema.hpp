#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

#include "query_base.hpp"

namespace QueryBuilder {

namespace Schema {

template <size_t N>
struct FixedString {
    char value[N]{};

    constexpr FixedString(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            value[i] = str[i];
        }
    }

    [[nodiscard]] constexpr std::string_view view() const { return std::string_view(value, N - 1); }
    constexpr operator std::string_view() const { return view(); }
};

template <FixedString Literal>
struct LiteralTag {
    static constexpr std::string_view value = Literal.view();
};

enum class RefMatch { Full, Partial, Simple };
enum class RefAction { NoAction, Restrict, Cascade, SetNull, SetDefault };
enum class OnCommitAction { PreserveRows, DeleteRows, Drop };

struct IfNotExists {};
struct Temporary {};
struct Unlogged {};
struct NotNull {};
struct Unique {};
struct PrimaryKey {};
struct GeneratedAlwaysIdentity {};
struct GeneratedByDefaultIdentity {};

template <typename ExprTag>
struct DefaultExpr {
    using expr_tag = ExprTag;
};

template <typename ExprTag>
struct CheckExpr {
    using expr_tag = ExprTag;
};

template <typename CollationTag>
struct Collate {
    using collation_tag = CollationTag;
};

template <typename ExprTag>
struct GeneratedStored {
    using expr_tag = ExprTag;
};

template <typename AccessMethodTag>
struct UsingMethod {
    using method_tag = AccessMethodTag;
};

template <typename TableSpaceTag>
struct TableSpace {
    using tablespace_tag = TableSpaceTag;
};

template <typename PartitionExprTag>
struct PartitionBy {
    using partition_expr_tag = PartitionExprTag;
};

template <OnCommitAction Action>
struct OnCommit {
    static constexpr OnCommitAction value = Action;
};

template <typename ParentTableTag>
struct Inherits {
    using table_tag = ParentTableTag;
};

template <typename KeyTag, typename ValueTag>
struct StorageParam {
    using key_tag = KeyTag;
    using value_tag = ValueTag;
};

template <typename RefTableTag, typename RefFieldTag, RefAction OnDelete = RefAction::NoAction,
          RefAction OnUpdate = RefAction::NoAction, RefMatch Match = RefMatch::Simple, bool Deferrable = false,
          bool InitiallyDeferred = false>
struct References;

template <typename Tag, typename = void>
struct is_string_tag : std::false_type {};

template <typename Tag>
struct is_string_tag<Tag, std::void_t<decltype(Tag::value)>>
    : std::bool_constant<std::is_convertible_v<decltype(Tag::value), std::string_view>> {};

template <typename Tag>
inline constexpr bool is_string_tag_v = is_string_tag<Tag>::value;

template <typename Left, typename Right>
struct same_name_tag : std::false_type {};

template <typename T, typename = void>
struct has_expr_tag : std::false_type {};
template <typename T>
struct has_expr_tag<T, std::void_t<typename T::expr_tag>> : std::true_type {};
template <typename T>
inline constexpr bool has_expr_tag_v = has_expr_tag<T>::value;

template <typename T, typename = void>
struct has_collation_tag : std::false_type {};
template <typename T>
struct has_collation_tag<T, std::void_t<typename T::collation_tag>> : std::true_type {};
template <typename T>
inline constexpr bool has_collation_tag_v = has_collation_tag<T>::value;

template <typename T, typename = void>
struct is_reference_option : std::false_type {};
template <typename T>
struct is_reference_option<T, std::void_t<typename T::table_tag, typename T::field_tag, decltype(T::on_delete),
                                          decltype(T::on_update), decltype(T::match)>> : std::true_type {};
template <typename T>
inline constexpr bool is_reference_option_v = is_reference_option<T>::value;

template <typename T, typename = void>
struct has_method_tag : std::false_type {};
template <typename T>
struct has_method_tag<T, std::void_t<typename T::method_tag>> : std::true_type {};
template <typename T>
inline constexpr bool has_method_tag_v = has_method_tag<T>::value;

template <typename T, typename = void>
struct has_tablespace_tag : std::false_type {};
template <typename T>
struct has_tablespace_tag<T, std::void_t<typename T::tablespace_tag>> : std::true_type {};
template <typename T>
inline constexpr bool has_tablespace_tag_v = has_tablespace_tag<T>::value;

template <typename T, typename = void>
struct has_partition_expr_tag : std::false_type {};
template <typename T>
struct has_partition_expr_tag<T, std::void_t<typename T::partition_expr_tag>> : std::true_type {};
template <typename T>
inline constexpr bool has_partition_expr_tag_v = has_partition_expr_tag<T>::value;

template <typename T, typename = void>
struct has_on_commit_value : std::false_type {};
template <typename T>
struct has_on_commit_value<T, std::void_t<decltype(T::value)>> : std::true_type {};
template <typename T>
inline constexpr bool has_on_commit_value_v = has_on_commit_value<T>::value;

template <typename T, typename = void>
struct has_inherits_tag : std::false_type {};
template <typename T>
struct has_inherits_tag<T, std::void_t<typename T::table_tag>> : std::true_type {};
template <typename T>
inline constexpr bool has_inherits_tag_v = has_inherits_tag<T>::value;

template <typename T, typename = void>
struct has_storage_param_tags : std::false_type {};
template <typename T>
struct has_storage_param_tags<T, std::void_t<typename T::key_tag, typename T::value_tag>> : std::true_type {};
template <typename T>
inline constexpr bool has_storage_param_tags_v = has_storage_param_tags<T>::value;

template <typename Left, typename Right>
    requires(is_string_tag_v<Left> && is_string_tag_v<Right>)
struct same_name_tag<Left, Right> : std::bool_constant<(std::string_view{Left::value} == std::string_view{Right::value})> {};

template <typename T>
struct is_schema_field : std::false_type {};

template <typename T>
struct is_schema_table : std::false_type {};

template <typename T, typename = void>
struct table_name_tag_of;

template <typename T>
struct table_name_tag_of<T, std::void_t<typename T::table_name_tag>> {
    using type = typename T::table_name_tag;
};

template <typename T>
using table_name_tag_of_t = typename table_name_tag_of<T>::type;

template <typename T, typename = void>
struct field_name_tag_of {
    using type = T;
};

template <typename T>
struct field_name_tag_of<T, std::void_t<typename T::name_tag>> {
    using type = typename T::name_tag;
};

template <typename T>
using field_name_tag_of_t = typename field_name_tag_of<T>::type;

template <typename TableTag, typename FieldTag>
class FieldReference;

template <typename NameTag, typename TypeTag, typename... Options>
class Field;

template <typename TableNameTag, typename... FieldDefs>
class Table;

template <typename NameTag, typename TypeTag, typename... Options>
struct is_schema_field<Field<NameTag, TypeTag, Options...>> : std::true_type {};

template <typename T>
inline constexpr bool is_schema_field_v = is_schema_field<T>::value;

template <typename...>
struct unique_names : std::true_type {};

template <typename First, typename... Rest>
struct unique_names<First, Rest...>
    : std::bool_constant<(!(same_name_tag<First, Rest>::value || ...)) && unique_names<Rest...>::value> {};

template <typename NameTag>
std::string quotedName() {
    static_assert(is_string_tag_v<NameTag>, "NameTag must provide static constexpr std::string_view value");
    return std::string{"\""} + Utility::fixString(std::string{NameTag::value}) + "\"";
}

inline constexpr std::string_view toString(RefMatch value) {
    switch (value) {
        case RefMatch::Full:
            return "FULL";
        case RefMatch::Partial:
            return "PARTIAL";
        case RefMatch::Simple:
            return "SIMPLE";
    }
    return "";
}

inline constexpr std::string_view toString(RefAction value) {
    switch (value) {
        case RefAction::NoAction:
            return "NO ACTION";
        case RefAction::Restrict:
            return "RESTRICT";
        case RefAction::Cascade:
            return "CASCADE";
        case RefAction::SetNull:
            return "SET NULL";
        case RefAction::SetDefault:
            return "SET DEFAULT";
    }
    return "";
}

inline constexpr std::string_view toString(OnCommitAction value) {
    switch (value) {
        case OnCommitAction::PreserveRows:
            return "PRESERVE ROWS";
        case OnCommitAction::DeleteRows:
            return "DELETE ROWS";
        case OnCommitAction::Drop:
            return "DROP";
    }
    return "";
}

template <typename TableTag, typename FieldTag>
class FieldReference : public QueryObject, public Base::Field {
public:
    static_assert(is_string_tag_v<TableTag>);
    static_assert(is_string_tag_v<FieldTag>);

    [[nodiscard]] constexpr std::string_view tableName() const { return TableTag::value; }
    [[nodiscard]] constexpr std::string_view fieldName() const { return FieldTag::value; }

    explicit operator std::string() const override { return quotedName<TableTag>() + "." + quotedName<FieldTag>(); }
};

template <typename NameTag, typename TypeTag, typename... Options>
class Field : public QueryObject, public Base::Field {
public:
    using name_tag = NameTag;

    static_assert(is_string_tag_v<NameTag>);
    static_assert(is_string_tag_v<TypeTag>);

    explicit operator std::string() const override {
        std::string sql = quotedName<NameTag>() + " " + std::string{TypeTag::value};
        (appendOption<Options>(sql), ...);
        return sql;
    }

private:
    template <typename Option>
    static void appendOption(std::string& sql) {
        if constexpr (std::is_same_v<Option, NotNull>) {
            sql += " NOT NULL";
        } else if constexpr (std::is_same_v<Option, Unique>) {
            sql += " UNIQUE";
        } else if constexpr (std::is_same_v<Option, PrimaryKey>) {
            sql += " PRIMARY KEY";
        } else if constexpr (std::is_same_v<Option, GeneratedAlwaysIdentity>) {
            sql += " GENERATED ALWAYS AS IDENTITY";
        } else if constexpr (std::is_same_v<Option, GeneratedByDefaultIdentity>) {
            sql += " GENERATED BY DEFAULT AS IDENTITY";
        } else if constexpr (has_expr_tag_v<Option>) {
            static_assert(is_string_tag_v<typename Option::expr_tag>);
            if constexpr (std::is_same_v<Option, DefaultExpr<typename Option::expr_tag>>) {
                sql += " DEFAULT ";
                sql += std::string{Option::expr_tag::value};
            } else if constexpr (std::is_same_v<Option, CheckExpr<typename Option::expr_tag>>) {
                sql += " CHECK (";
                sql += std::string{Option::expr_tag::value};
                sql += ")";
            } else if constexpr (std::is_same_v<Option, GeneratedStored<typename Option::expr_tag>>) {
                sql += " GENERATED ALWAYS AS (";
                sql += std::string{Option::expr_tag::value};
                sql += ") STORED";
            }
        } else if constexpr (has_collation_tag_v<Option>) {
            static_assert(is_string_tag_v<typename Option::collation_tag>);
            sql += " COLLATE ";
            sql += std::string{Option::collation_tag::value};
        } else if constexpr (is_reference_option_v<Option>) {
            sql += " REFERENCES ";
            sql += quotedName<typename Option::table_tag>();
            sql += "(";
            sql += quotedName<typename Option::field_tag>();
            sql += ")";
            sql += " MATCH ";
            sql += toString(Option::match);
            sql += " ON DELETE ";
            sql += toString(Option::on_delete);
            sql += " ON UPDATE ";
            sql += toString(Option::on_update);
            if constexpr (Option::deferrable) {
                sql += " DEFERRABLE";
            }
            if constexpr (Option::initially_deferred) {
                sql += " INITIALLY DEFERRED";
            }
        }
    }
};

template <typename TableNameTag, typename... FieldDefs>
class Table : public QueryObject, public Base::Table {
public:
    using table_name_tag = TableNameTag;

    static_assert(is_string_tag_v<TableNameTag>);
    static_assert((is_schema_field_v<FieldDefs> && ...), "Table fields must be Schema::Field");
    static_assert(unique_names<typename FieldDefs::name_tag...>::value, "Field names must be unique");

    template <typename FieldNameTag>
    static constexpr FieldReference<TableNameTag, FieldNameTag> ref() {
        static_assert((same_name_tag<FieldNameTag, typename FieldDefs::name_tag>::value || ...),
                      "This field does not belong to the current table type");
        return {};
    }

    template <FixedString FieldNameLiteral>
    static constexpr auto ref() -> FieldReference<TableNameTag, LiteralTag<FieldNameLiteral>> {
        return ref<LiteralTag<FieldNameLiteral>>();
    }

    static std::string renderColumns() {
        std::string sql;
        bool needComma = false;
        ((appendField<FieldDefs>(sql, needComma)), ...);
        return sql;
    }

    explicit operator std::string() const override { return quotedName<TableNameTag>(); }

private:
    template <typename FieldType>
    static void appendField(std::string& sql, bool& needComma) {
        if (needComma) {
            sql += ", ";
        }
        sql += static_cast<std::string>(FieldType{});
        needComma = true;
    }
};

template <typename TableNameTag, typename... FieldDefs>
struct is_schema_table<Table<TableNameTag, FieldDefs...>> : std::true_type {};

template <typename T>
inline constexpr bool is_schema_table_v = is_schema_table<T>::value;

template <typename RefTableTag, typename RefFieldTag, RefAction OnDelete, RefAction OnUpdate, RefMatch Match,
          bool Deferrable, bool InitiallyDeferred>
struct References {
    using table_tag = std::conditional_t<is_schema_table_v<RefTableTag>, table_name_tag_of_t<RefTableTag>, RefTableTag>;
    using field_tag = field_name_tag_of_t<RefFieldTag>;
    static constexpr RefAction on_delete = OnDelete;
    static constexpr RefAction on_update = OnUpdate;
    static constexpr RefMatch match = Match;
    static constexpr bool deferrable = Deferrable;
    static constexpr bool initially_deferred = InitiallyDeferred;

    static_assert(is_string_tag_v<table_tag>, "References table must be Schema::Table or table name tag");
    static_assert(is_string_tag_v<field_tag>, "References field must be Schema::Field or field name tag");
};

template <typename TableType, typename... TableOptions>
class CreateTableStatement : public QueryObject {
public:
    static_assert(is_schema_table_v<TableType>, "CreateTableStatement requires Schema::Table type");

    explicit operator std::string() const override {
        std::string sql = "CREATE";
        (appendPrefixOption<TableOptions>(sql), ...);
        sql += " TABLE";
        if constexpr ((std::is_same_v<TableOptions, IfNotExists> || ...)) {
            sql += " IF NOT EXISTS";
        }
        sql += " ";
        sql += static_cast<std::string>(TableType{});
        sql += " (";
        sql += TableType::renderColumns();
        sql += ")";
        (appendSuffixOption<TableOptions>(sql), ...);
        return sql;
    }

private:
    template <typename Option>
    static void appendPrefixOption(std::string& sql) {
        if constexpr (std::is_same_v<Option, Temporary>) {
            sql += " TEMP";
        } else if constexpr (std::is_same_v<Option, Unlogged>) {
            sql += " UNLOGGED";
        }
    }

    template <typename Option>
    static void appendSuffixOption(std::string& sql) {
        if constexpr (has_method_tag_v<Option>) {
            static_assert(is_string_tag_v<typename Option::method_tag>);
            sql += " USING ";
            sql += std::string{Option::method_tag::value};
        } else if constexpr (has_tablespace_tag_v<Option>) {
            static_assert(is_string_tag_v<typename Option::tablespace_tag>);
            sql += " TABLESPACE ";
            sql += std::string{Option::tablespace_tag::value};
        } else if constexpr (has_partition_expr_tag_v<Option>) {
            static_assert(is_string_tag_v<typename Option::partition_expr_tag>);
            sql += " PARTITION BY ";
            sql += std::string{Option::partition_expr_tag::value};
        } else if constexpr (has_on_commit_value_v<Option>) {
            sql += " ON COMMIT ";
            sql += toString(Option::value);
        } else if constexpr (has_inherits_tag_v<Option>) {
            static_assert(is_string_tag_v<typename Option::table_tag>);
            sql += " INHERITS (";
            sql += quotedName<typename Option::table_tag>();
            sql += ")";
        } else if constexpr (has_storage_param_tags_v<Option>) {
            static_assert(is_string_tag_v<typename Option::key_tag>);
            static_assert(is_string_tag_v<typename Option::value_tag>);
            sql += " WITH (";
            sql += std::string{Option::key_tag::value};
            sql += "=";
            sql += std::string{Option::value_tag::value};
            sql += ")";
        }
    }
};

template <FixedString NameLiteral, FixedString TypeLiteral, typename... Options>
using FieldL = Field<LiteralTag<NameLiteral>, LiteralTag<TypeLiteral>, Options...>;

template <FixedString TableLiteral, typename... FieldDefs>
using TableL = Table<LiteralTag<TableLiteral>, FieldDefs...>;

template <FixedString ExprLiteral>
using DefaultExprL = DefaultExpr<LiteralTag<ExprLiteral>>;

template <FixedString ExprLiteral>
using CheckExprL = CheckExpr<LiteralTag<ExprLiteral>>;

template <FixedString CollationLiteral>
using CollateL = Collate<LiteralTag<CollationLiteral>>;

template <FixedString ExprLiteral>
using GeneratedStoredL = GeneratedStored<LiteralTag<ExprLiteral>>;

template <FixedString MethodLiteral>
using UsingMethodL = UsingMethod<LiteralTag<MethodLiteral>>;

template <FixedString TableSpaceLiteral>
using TableSpaceL = TableSpace<LiteralTag<TableSpaceLiteral>>;

template <FixedString PartitionLiteral>
using PartitionByL = PartitionBy<LiteralTag<PartitionLiteral>>;

template <FixedString ParentTableLiteral>
using InheritsL = Inherits<LiteralTag<ParentTableLiteral>>;

template <FixedString KeyLiteral, FixedString ValueLiteral>
using StorageParamL = StorageParam<LiteralTag<KeyLiteral>, LiteralTag<ValueLiteral>>;

}  // namespace Schema

namespace Utility {

template <typename T>
struct like_schema_field : std::bool_constant<Schema::is_schema_field_v<T>> {};

template <typename T>
constexpr bool like_schema_field_v = like_schema_field<T>::value;

template <typename T>
struct like_schema_table : std::bool_constant<Schema::is_schema_table_v<T>> {};

template <typename T>
constexpr bool like_schema_table_v = like_schema_table<T>::value;

}  // namespace Utility

}  // namespace QueryBuilder
