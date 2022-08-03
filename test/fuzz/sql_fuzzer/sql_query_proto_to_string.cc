#include "sql_query_proto_to_string.h"

#include <algorithm>

using namespace sql_query;

#define CONV_FN(TYPE, VAR_NAME) std::string TYPE##ToString(const TYPE& VAR_NAME)

namespace sql_fuzzer {

namespace {

constexpr uint32_t kMaxColumnNumber = 20;
constexpr uint32_t kMaxTableNumber = 8;
constexpr uint32_t kMaxColumnConstraintNumber = 10;
[[maybe_unused]]
constexpr uint32_t kMaxTableConstraintNumber = 10;
[[maybe_unused]]
constexpr uint32_t kMaxIndexNumber = 10;

} // namespace

CONV_FN(TableName, table_name) {
    return "table" + std::to_string(table_name.code() % kMaxTableNumber) + " ";
}

CONV_FN(ColumnName, column_name) {
    return "table" + std::to_string(column_name.code() % kMaxColumnNumber) + " ";
}

CONV_FN(VarChar, varchar) {
    return "VARCHAR (" + std::to_string(varchar.integer()) + ") ";
}

CONV_FN(CollationClause, collation_clause) {
    return "COLLATE " + CollationClause_CollationClauseEnum_Name(collation_clause.collation_clause_enum()) + " ";
}

CONV_FN(CollatableDataType, type) {
    std::string ret;
    switch (type.collatable_data_type_oneof_case()) {
        case CollatableDataType::kTypeEnum:
            ret += CollatableDataType_CollatableDataTypeEnum_Name(type.type_enum());
            break;
        case CollatableDataType::kVarchar:
            ret += VarCharToString(type.varchar());
            break;
        default:
            ret += CollatableDataType_CollatableDataTypeEnum_Name(type.type_enum_fallback());
            break;
    }
    if (type.has_collation_clause()) 
        ret += " " + CollationClauseToString(type.collation_clause());
    return ret;
}

CONV_FN(DataType, type) {
    switch (type.data_type_oneof_case()) {
        case DataType::kTypeEnum:
            return DataType_DataTypeEnum_Name(type.type_enum());
        case DataType::kCollatableType:
            return CollatableDataTypeToString(type.collatable_type());
        default:
            return "INVALID ";
    }
}

CONV_FN(ColumnConstraintNullable, nullable) {
    return ColumnConstraintNullable_ColumnConstraintNullableEnum_Name(nullable.nullable_enum()) + " ";
}

CONV_FN(ColumnConstraintName, constraint_name) {
    return "colcon" + std::to_string(constraint_name.code() % kMaxColumnConstraintNumber) + " ";
}

CONV_FN(Expression, expression) {
    return "TRUE"; // TODO: update after changing protobuf
}

CONV_FN(ReferenceForeignKeyClause, foreign_key_clause) {
    return ""; // TODO: update after changing protobuf
}

CONV_FN(NamedColumnConstraint, named_constraint) {
    std::string ret;
    if (named_constraint.has_constraint_name())
        ret += ColumnConstraintNameToString(named_constraint.constraint_name());
    switch (named_constraint.constraint_oneof_case()) {
        case NamedColumnConstraint::kUnique:
            if (named_constraint.unique())
                ret += "UNIQUE ";
            break;
        case NamedColumnConstraint::kCheckExpression:
            ret += ExpressionToString(named_constraint.check_expression());
            break;
        case NamedColumnConstraint::kForeignKeyClause:
            ret += ReferenceForeignKeyClauseToString(named_constraint.foreign_key_clause());
            break;
        default:
            break;
    }
    if (named_constraint.primary_key())
        ret += "PRIMARY KEY ";
    return ret;
}

CONV_FN(DefaultExpression, default_expression) {
    return ""; // TODO: update after changing protobuf
}

CONV_FN(ColumnConstraint, constraint) {
    switch (constraint.column_constraint_oneof_case()) {
        case ColumnConstraint::kNullable:
            return ColumnConstraintNullableToString(constraint.nullable());
        case ColumnConstraint::kNamedConstraint:
            return NamedColumnConstraintToString(constraint.named_constraint());
        default:
            return DefaultExpressionToString(constraint.default_expression());
    }
}

CONV_FN(ColumnDefinition, def) {
    std::string ret = ColumnNameToString(def.column_name()) + " "
                      +  DataTypeToString(def.type());
    for (int i = 0; i < def.constraints_size(); ++i) {
        if (i > 0)
            ret += ",";
        ret += " " + ColumnConstraintToString(def.constraints(i));
    }
    return ret + " ";
}

CONV_FN(TableConstraint, table_constraint) {
    return ""; // TODO: update after changing protobuf
}

CONV_FN(CreateTableOption, opt) {
    switch (opt.option_oneof_case()) {
        case CreateTableOption::kColumnDefinition: 
            return ColumnDefinitionToString(opt.column_definition());
        case CreateTableOption::kTableConstraint:
            return TableConstraintToString(opt.table_constraint());
        default:
            return ColumnDefinitionToString(opt.column_definition_fallback());
    }
}

CONV_FN(Engine, engine) {
    return "WITH ENGINE " + Engine_EngineEnum_Name(engine.engine_enum()) + " ";
}

CONV_FN(CreateTable, create_table) {
    std::string ret("CREATE TABLE ");
    if (create_table.if_not_exists())
        ret += "IF NOT EXISTS ";
    ret += TableNameToString(create_table.table_name());
    ret += "(";
    for (int i = 0; i < create_table.options_size(); ++i) {
        if (i > 0)
            ret += ",";
        ret += " " + CreateTableOptionToString(create_table.options(i));
    }
    ret += ") ";
    if (create_table.has_engine())
        ret += "WITH ENGINE " + EngineToString(create_table.engine());

    return ret;
}

CONV_FN(SQLQuery, query) {
    switch (query.query_oneof_case()) {
        case SQLQuery::kCreateTable: 
            return CreateTableToString(query.create_table());
        default:
            return "";
    }
}

} // namespace sql_fuzzer