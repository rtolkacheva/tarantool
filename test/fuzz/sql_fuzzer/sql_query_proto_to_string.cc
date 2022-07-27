#include "sql_query_proto_to_string.h"

using namespace sql_query;

#define CONV_FN(TYPE, VAR_NAME) std::string TYPE##ToString(const TYPE& VAR_NAME)

namespace sql_fuzzer {

namespace {

constexpr uint32_t kMaxColumnNumber = 20;
constexpr uint32_t kMaxTableNumber = 8;
constexpr uint32_t kMaxColumnConstraintNumber = 10;
constexpr uint32_t kMaxTableConstraintNumber = 10;
constexpr uint32_t kMaxIndexNumber = 10;

} // namespace

CONV_FN(SQLQuery, query) {
    switch (query.query_oneof_case()) {
        case SQLQuery::kCreateTable: 
            return CreateTableToString(query.create_table());
        default:
            return "";
    }
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

CONV_FN(TableName, table_name) {
    return "table" + std::to_string(table_name.code() % kMaxTableNumber);
}

CONV_FN(CreateTableOption, opt) {
    switch (opt.option_oneof_case()) {
        case CreateTableOption::kColumnDefinition: 
            return ColumnDefinitionToString(opt.column_definition());
        case CreateTableOption::kTableConstraint {
            return TableConstraintToString(opt.table_constraint());
        default:
            return ColumnDefinitionToString(opt.column_definition_fallback());
        }
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
    return ret;
}

CONV_FN(ColumnName, column_name) {
    return "table" + std::to_string(column_name.code() % kMaxColumnNumber);
}

CONV_FN(DataType, type) {
    switch (type.data_type_oneof_case()) {
        case DataType::kTypeEnum:
            return DataType_DataTypeEnum_Name(type.type_enum());
        case DataType::kCollatableType:
            return CollatableDataTypeToString(type.collatable_type());
    }
}

CONV_FN(CollatableDataType, type) {
    std::string ret;
    switch (type.collatable_data_type_oneof_case()) {
        case CollatableDataType::kTypeEnum:
            // TODO
    }
}

CONV_FN(ColumnConstraint, constraint) {

}

CONV_FN(Engine) {

}


}


} // namespace sql_fuzzer