/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2022, Tarantool AUTHORS, please see AUTHORS file.
 */

#include "sql_query_proto_to_string.h"

#include <algorithm>
#include <cstring>

using namespace sql_query;

/* Every conversation function from SQL Query type to std::string must end with
 * a space symbol. Thus, no reserved or user words won't accidently concatenate.
 */
#define CONV_FN(TYPE, VAR_NAME) \
	std::string TYPE##ToString(const TYPE & (VAR_NAME))

namespace sql_fuzzer {

namespace {
constexpr uint32_t kMaxColumnNumber = 20;
constexpr uint32_t kMaxTableNumber = 8;
constexpr uint32_t kMaxColumnConstraintNumber = 10;
[[maybe_unused]]
constexpr uint32_t kMaxTableConstraintNumber = 10;
[[maybe_unused]]
constexpr uint32_t kMaxIndexNumber = 10;
} /* namespace */

CONV_FN(Select, select);
CONV_FN(FunctionExpression, expression);
CONV_FN(Term, term);
CONV_FN(PredicateExpression, expression);
CONV_FN(JoinSource, source);

CONV_FN(TableName, table_name) {
	return "table"
		+ std::to_string(table_name.code() % kMaxTableNumber) + " ";
}

CONV_FN(ColumnName, column_name) {
	return "column"
		+ std::to_string(column_name.code() % kMaxColumnNumber) + " ";
}

CONV_FN(VarChar, varchar) {
	return "VARCHAR (" + std::to_string(varchar.integer()) + ") ";
}

CONV_FN(CollationClause, collation_clause) {
	return "COLLATE \"" + CollationClause_CollationClauseEnum_Name(
		collation_clause.collation_clause_enum()
		) + "\" ";
}

CONV_FN(CollatableDataType, type) {
	std::string ret;
	switch (type.collatable_data_type_oneof_case()) {
	case CollatableDataType::kTypeEnum:
		ret += CollatableDataType_CollatableDataTypeEnum_Name(
			type.type_enum()
			) + " ";
		break;
	case CollatableDataType::kVarchar:
		ret += VarCharToString(type.varchar());
		break;
	default:
		ret += CollatableDataType_CollatableDataTypeEnum_Name(
			type.type_enum_fallback()
			) + " ";
		break;
	}
	if (type.has_collation_clause())
		ret += " " + CollationClauseToString(type.collation_clause());
	return ret;
}

CONV_FN(DataType, type) {
	switch (type.data_type_oneof_case()) {
	case DataType::kSpecialTypeEnum:
		return DataType_SpecialDataTypeEnum_Name(
			type.special_type_enum()
			) + " ";
	case DataType::kTypeEnum:
		return DataType_DataTypeEnum_Name(type.type_enum()) + " ";
	case DataType::kCollatableType:
		return CollatableDataTypeToString(type.collatable_type());
	default:
		return DataType_DataTypeEnum_Name(type.type_enum_fallback())
			+ " ";
	}
}

CONV_FN(ColumnConstraintNullable, nullable) {
	switch (nullable.nullable_enum()) {
	case ColumnConstraintNullable::NOT_NULL:
		return "NOT NULL ";
	case ColumnConstraintNullable::NULLABLE:
		return "NULL ";
	default:
		return "";
	}
}

CONV_FN(ColumnConstraintName, constraint_name) {
	/* colcon is a constraction for column constraint */
	return "colcon" + std::to_string(constraint_name.code()
		% kMaxColumnConstraintNumber) + " ";
}

static std::string
BooleanConstantToString(uint64_t value)
{
	return value ? "TRUE " : "FALSE ";
}

static std::string
DecimalConstantToString(uint64_t value)
{
	return std::to_string(value) + " ";
}

static std::string
UUIDConstantToString(uint64_t value)
{
	return std::to_string(value) + " ";
}

static std::string
VarbinaryToString(uint64_t value)
{
	return "X\'" + std::to_string(value) + "\' ";
}

static std::string
StringConstantToString(uint64_t value)
{
	return "\'" + std::to_string(value) + "\' ";
}

static std::string
DoubleConstantToString(uint64_t value)
{
	double d_value = 0.0;
	size_t number_of_bytes_to_copy =
		std::min(sizeof(double), sizeof(uint64_t));
	std::memcpy(&d_value, &value, number_of_bytes_to_copy);
	return std::to_string(d_value) + " ";
}

static std::string
IntegerConstantToString(uint64_t value)
{
	int64_t signed_value = 0;
	std::memcpy(&signed_value, &value, sizeof(uint64_t));
	return std::to_string(signed_value) + " ";
}

static std::string
NumberConstantToString(uint64_t value)
{
	return DoubleConstantToString(value);
}

static std::string
UnsignedConstantToString(uint64_t value)
{
	return std::to_string(value) + " ";
}

static std::string
ScalarConstantToString(uint64_t value)
{
	if (value % 2 == 1)
		return IntegerConstantToString(value);
	else
		return StringConstantToString(value);
}

static std::string
DataTypeEnumConstantToString(DataType_DataTypeEnum type, uint64_t value)
{
	switch (type) {
	case DataType::BOOLEAN:
		return BooleanConstantToString(value);
	case DataType::DECIMAL:
		return DecimalConstantToString(value);
	case DataType::DOUBLE:
		return DoubleConstantToString(value);
	case DataType::INTEGER:
		return IntegerConstantToString(value);
	case DataType::NUMBER:
		return NumberConstantToString(value);
	case DataType::UNSIGNED:
		return UnsignedConstantToString(value);
	case DataType::UUID:
		return UUIDConstantToString(value);
	case DataType::VARBINARY:
		return VarbinaryToString(value);
	}
}

static std::string
CollatableDataTypeEnumConstantToString(
	CollatableDataType_CollatableDataTypeEnum type,
	uint64_t value)
{
	switch (type) {
	case CollatableDataType::SCALAR:
		return ScalarConstantToString(value);
	case CollatableDataType::STRING:
		return StringConstantToString(value);
	case CollatableDataType::TEXT:
		return StringConstantToString(value);
	}
}

static std::string
CollatableDataTypeConstantToString(CollatableDataType type, uint64_t value)
{
	switch (type.collatable_data_type_oneof_case()) {
	case CollatableDataType::kTypeEnum:
		return CollatableDataTypeEnumConstantToString(
			type.type_enum(), value
			);
	case CollatableDataType::kVarchar:
		return StringConstantToString(value);
	default:
		return CollatableDataTypeEnumConstantToString(
			type.type_enum_fallback(), value
			);
	}
}

CONV_FN(ConstantValue, constant) {
	switch (constant.type().data_type_oneof_case()) {
	case DataType::kTypeEnum:
		return DataTypeEnumConstantToString(
			constant.type().type_enum(),
			constant.value()
			);
	case DataType::kCollatableType:
		return CollatableDataTypeConstantToString(
			constant.type().collatable_type(),
			constant.value()
			);
	default:
		return DataTypeEnumConstantToString(
			constant.type().type_enum_fallback(),
			constant.value()
			);
	}
}

CONV_FN(BinaryOperatorExpression, expression) {
	std::string ret = "(" + TermToString(expression.left_operand());

	switch (expression.binary_operator()) {
	case BinaryOperatorExpression::PLUS:
		ret += "+ ";
		break;
	case BinaryOperatorExpression::MINUS:
		ret += "- ";
		break;
	case BinaryOperatorExpression::AND:
		ret += "AND ";
		break;
	case BinaryOperatorExpression::OR:
		ret += "OR ";
		break;
	case BinaryOperatorExpression::BINARY_AND:
		ret += "& ";
		break;
	case BinaryOperatorExpression::BINARY_OR:
		ret += "| ";
		break;
	case BinaryOperatorExpression::MULTIPLY:
		ret += "* ";
		break;
	case BinaryOperatorExpression::DIVISION:
		ret += "/ ";
		break;
	case BinaryOperatorExpression::REMAINDER:
		ret += "% ";
		break;
	}

	ret += TermToString(expression.right_operand()) + ") ";

	return ret;
}

CONV_FN(UnaryOperatorExpression, expression) {
	std::string ret = "(";
	switch (expression.unary_operator()) {
	case UnaryOperatorExpression::NOT:
		ret += "!" + TermToString(expression.term());
		break;
	case UnaryOperatorExpression::PLUS:
		ret += "+" + TermToString(expression.term());
		break;
	case UnaryOperatorExpression::MINUS:
		ret += "-" + TermToString(expression.term());
		break;
	case UnaryOperatorExpression::IS_NULL:
		ret += TermToString(expression.term()) + "IS NULL ";
		break;
	case UnaryOperatorExpression::IS_NOT_NULL:
		ret += TermToString(expression.term()) + "IS NOT NULL ";
		break;
	case UnaryOperatorExpression::NO_OPERATOR:
		ret += TermToString(expression.term());
		break;
	}
	ret += ") ";

	return ret;
}

CONV_FN(FunctionExpression, expression) {
	switch (expression.function_expression_oneof_case()) {
	case FunctionExpression::kBinaryOperatorExpression:
		return BinaryOperatorExpressionToString(
			expression.binary_operator_expression()
			);
	default:
		return UnaryOperatorExpressionToString(
			expression.unary_operator_expression_fallback()
			);
	}
}

CONV_FN(Term, term) {
	switch (term.term_oneof_case()) {
	case Term::kFunc:
		return FunctionExpressionToString(term.func());
	case Term::kPredicate:
		return PredicateExpressionToString(term.predicate());
	case Term::kColumnName:
		return ColumnNameToString(term.column_name());
	default:
		return ConstantValueToString(term.constant_fallback());
	}
}

CONV_FN(CompareExpression, compare) {
	std::string ret = "(" + TermToString(compare.left_operand());
	switch (compare.comporator()) {
	case CompareExpression::LESS:
		ret += "< ";
		break;
	case CompareExpression::LESS_EQUAL:
		ret += "<= ";
		break;
	case CompareExpression::EQUAL:
		ret += "= ";
		break;
	case CompareExpression::NOT_EQUAL:
		ret += "!= ";
		break;
	case CompareExpression::GREATER_EQUAL:
		ret += ">= ";
		break;
	case CompareExpression::GREATER:
		ret += "> ";
		break;
	case CompareExpression::EQUAL_EQUAL:
		ret += "== ";
		break;
	case CompareExpression::DIFFERENT:
		ret += "<> ";
		break;
	}

	ret += TermToString(compare.right_operand()) + ") ";

	return ret;
}

CONV_FN(PredicateExpression, expression) {
	switch (expression.predicate_oneof_case()) {
	case PredicateExpression::kCompare:
		return CompareExpressionToString(expression.compare());
	default:
		return expression.bool_constant_fallback()
			? "TRUE " : "FALSE ";
	}
}

CONV_FN(ReferenceForeignKeyClauseRule, rule) {
	switch (rule.rule()) {
	case ReferenceForeignKeyClauseRule::CASCADE:
		return "CASCADE ";
	case ReferenceForeignKeyClauseRule::SET_DEFAULT:
		return "SET DEFAULT ";
	case ReferenceForeignKeyClauseRule::SET_NULL:
		return "SET NULL ";
	case ReferenceForeignKeyClauseRule::RESTRICT:
		return "RESTRICT ";
	case ReferenceForeignKeyClauseRule::NO_ACTION:
		return "NO ACTION ";
	}
}

CONV_FN(ReferenceForeignKeyClause, foreign_key_clause) {
	std::string ret = "REFERENCES ";

	ret += TableNameToString(foreign_key_clause.table_name()) + "(";
	ret += ColumnNameToString(foreign_key_clause.column_name());
	for (int i = 0; i < foreign_key_clause.extra_column_names_size(); ++i) {
		ret += ", ";
		ret += ColumnNameToString(
			foreign_key_clause.extra_column_names(i)
			);
	}
	ret += ") ";

	if (foreign_key_clause.match_full()) {
		ret += "MATCH FULL ";
	}

	if (foreign_key_clause.has_delete_rule()) {
		ret += "ON DELETE "
			+ ReferenceForeignKeyClauseRuleToString(
				foreign_key_clause.delete_rule()
				);
	}

	if (foreign_key_clause.has_update_rule()) {
		ret += "ON UPDATE "
			+ ReferenceForeignKeyClauseRuleToString(
				foreign_key_clause.update_rule()
				);
	}

	return ret;
}

CONV_FN(NamedColumnConstraintPrimaryKey, primary_key) {
	return "PRIMARY KEY ";
}

CONV_FN(NamedColumnConstraintCheck, check) {
	return "CHECK ("
		+ PredicateExpressionToString(check.check_expression()) + ") ";
}

CONV_FN(NamedColumnConstraintUnique, unique) {
	return "UNIQUE ";
}

CONV_FN(NamedColumnConstraint, named_constraint) {
	std::string ret;
	if (named_constraint.has_constraint_name())
		ret += ColumnConstraintNameToString(
			named_constraint.constraint_name()
			);

	switch (named_constraint.constraint_oneof_case()) {
	case NamedColumnConstraint::kPrimaryKey:
		ret += NamedColumnConstraintPrimaryKeyToString(
			named_constraint.primary_key()
			);
		break;
	case NamedColumnConstraint::kForeignKeyClause:
		ret += ReferenceForeignKeyClauseToString(
			named_constraint.foreign_key_clause()
			);
		break;
	case NamedColumnConstraint::kCheckExpression:
		ret += NamedColumnConstraintCheckToString(
			named_constraint.check_expression()
			);
		break;
	default:
		ret += NamedColumnConstraintUniqueToString(
			named_constraint.unique_fallback()
			);
		break;
	}
	return ret;
}

CONV_FN(DefaultExpression, default_expression) {
	return "DEFAULT "
		+ FunctionExpressionToString(default_expression.expression());
}

CONV_FN(ColumnConstraint, constraint) {
	switch (constraint.column_constraint_oneof_case()) {
	case ColumnConstraint::kNullable:
		return ColumnConstraintNullableToString(
			constraint.nullable()
			);
	case ColumnConstraint::kNamedConstraint:
		return NamedColumnConstraintToString(
			constraint.named_constraint()
			);
	default:
		return DefaultExpressionToString(
			constraint.default_expression()
			);
	}
}

CONV_FN(ColumnDefinition, def) {
	std::string ret = ColumnNameToString(def.column_name()) + " "
		+ DataTypeToString(def.type());
	for (int i = 0; i < def.constraints_size(); ++i) {
		if (i > 0)
			ret += ",";
		ret += " " + ColumnConstraintToString(def.constraints(i));
	}
	return ret;
}

CONV_FN(TableConstraintPrimaryKey, primary_key) {
	std::string ret = "PRIMARY KEY ("
		+ ColumnNameToString(primary_key.column_name());
	for (int i = 0; i <= primary_key.extra_column_names_size(); ++i) {
		ret += ", "
			+ ColumnNameToString(primary_key.extra_column_names(i));
	}
	ret += ") ";
	return ret;
}

CONV_FN(TableConstraintUnique, unique) {
	std::string ret = "UNIQUE (" + ColumnNameToString(unique.column_name());
	for (int i = 0; i <= unique.extra_column_names_size(); ++i) {
		ret += ", " + ColumnNameToString(unique.extra_column_names(i));
	}
	ret += ") ";
	return ret;
}

CONV_FN(TableConstraintCheck, check_expression) {
	return "CHECK ("
		+ PredicateExpressionToString(check_expression.predicate())
		+ ") ";
}

CONV_FN(TableConstraintForeignKeyClause, foreign_key) {
	std::string ret = "FOREIGN KEY ("
	+ ColumnNameToString(foreign_key.column_name());
	for (int i = 0; i <= foreign_key.extra_column_names_size(); ++i) {
		ret += ", "
			+ ColumnNameToString(foreign_key.extra_column_names(i));
	}
	ret += ") ";
	ret += ReferenceForeignKeyClauseToString(foreign_key.reference());
	return ret;
}

CONV_FN(TableConstraint, table_constraint) {
	switch (table_constraint.table_constraint_oneof_case()) {
	case TableConstraint::kPrimaryKey:
		return TableConstraintPrimaryKeyToString(
			table_constraint.primary_key()
			);
	case TableConstraint::kCheckExpression:
		return TableConstraintCheckToString(
			table_constraint.check_expression()
			);
	case TableConstraint::kForeignKey:
		return TableConstraintForeignKeyClauseToString(
			table_constraint.foreign_key()
			);
	default:
		return TableConstraintUniqueToString(
			table_constraint.unique_fallback()
			);
	}
}

CONV_FN(DummyColumnDefinition, definition) {
	std::string ret = "column0 ";
	if (definition.type().data_type_oneof_case()
	    == DataType::kSpecialTypeEnum) {
		ret += DataType_DataTypeEnum_Name(
			definition.type().type_enum_fallback()
			) + " ";
	} else {
		ret += DataTypeToString(definition.type());
	}

	ret += "PRIMARY KEY ";

	return ret;
}

CONV_FN(Engine, engine) {
	return "WITH ENGINE = \'"
		+ Engine_EngineEnum_Name(engine.engine_enum()) + "\' ";
}

static ColumnName
CreateColumnName(uint32_t code)
{
	ColumnName name;
	name.set_code(code);
	return name;
}

static std::string
ColumnDefinitionToStringWithName(const ColumnDefinition &column_def,
				 const ColumnName &name)
{
	ColumnDefinition mutable_column_def(column_def);
	*mutable_column_def.mutable_column_name() = name;
	return ColumnDefinitionToString(mutable_column_def);
}

CONV_FN(CreateTable, create_table) {
	std::string ret("CREATE TABLE ");
	if (create_table.if_not_exists())
		ret += "IF NOT EXISTS ";
	ret += TableNameToString(create_table.table_name());
	ret += "(";
	ret += DummyColumnDefinitionToString(create_table.dummy_definition());

	uint32_t column_count = 1;
	for (int i = 0; i < create_table.options_size(); ++i) {
		/* Inlined CreateTableOptionToString() function so columns
		 * would be named better than just randomly.
		 */
		switch (create_table.options(i).option_oneof_case()) {
		case CreateTableOption::kColumnDefinition:
			if (column_count >= kMaxColumnNumber)
				break;
			ret += ", "
				+ ColumnDefinitionToStringWithName(
					create_table.options(i)
						.column_definition(),
					CreateColumnName(column_count++));
			break;
		case CreateTableOption::kTableConstraint:
			ret += TableConstraintToString(
				create_table.options(i).table_constraint()
				);
		default:
			if (column_count >= kMaxColumnNumber)
				break;
			ret += ", "
				+ ColumnDefinitionToStringWithName(
					create_table.options(i)
						.column_definition_fallback(),
					CreateColumnName(column_count++)
				);
			break;
		}
	}
	ret += ") ";
	if (create_table.has_engine())
		ret += EngineToString(create_table.engine());

	return ret;
}

CONV_FN(IndexName, name) {
	return "index" + std::to_string(name.code() % kMaxIndexNumber) + " ";
}

CONV_FN(SelectFromClauseOption1, from_clause) {
	std::string ret = TableNameToString(from_clause.table_name());

	if (from_clause.has_as_table_name()) {
		ret += "AS " + TableNameToString(from_clause.as_table_name());
	}
	switch (from_clause.indexed_oneof_case()) {
	case SelectFromClauseOption1::kIndexedEnum:
		switch (from_clause.indexed_enum()) {
		case SelectFromClauseOption1::NOT_INDEXED:
			ret += "NOT INDEXED ";
			break;
		}
		break;
	case SelectFromClauseOption1::kIndexName:
		ret += "INDEXED BY "
			+ IndexNameToString(from_clause.index_name());
		break;
	default:
		break;
	}
	return ret;
}

CONV_FN(SelectFromClauseOption2, option) {
	std::string ret = "(" + SelectToString(option.select_statement())
			+ ") ";

	if (option.has_table_name()) {
		if (option.as_construction_present_flag())
			ret += "AS ";
		ret += TableNameToString(option.table_name());
	}

	return ret;
}

CONV_FN(LeftJoin, left_join) {
	std::string ret;
	if (left_join.natural()) {
		ret += "NATURAL ";
	}
	ret += "LEFT ";
	if (left_join.outer()) {
		ret += "OUTER ";
	}
	ret += "JOIN ";
	return ret;
}

CONV_FN(InnerJoin, inner_join) {
	std::string ret;
	if (inner_join.natural()) {
		ret += "NATURAL ";
	}
	ret += "INNER JOIN ";
	return ret;
}

CONV_FN(JoinOperator, join_operator) {
	switch (join_operator.join_operator_oneof_case()) {
	case JoinOperator::kLeftJoin:
		return LeftJoinToString(join_operator.left_join());
	case JoinOperator::kInnerJoin:
		return InnerJoinToString(join_operator.inner_join());
	default:
		return "CROSS JOIN ";
	}
}

CONV_FN(JoinSpecificationUsing, using_specification) {
	std::string ret = "USING ( "
		+ ColumnNameToString(using_specification.column_name());
	for (int i = 0;
		 i <= using_specification.extra_column_names_size();
		 ++i) {
		ret += ", "
		+ ColumnNameToString(
			using_specification.extra_column_names(i)
			);
	}
	ret += ") ";
	return ret;
}

CONV_FN(JoinSpecificationOnExpression, on_expr) {
	return "ON " + PredicateExpressionToString(on_expr.expr());
}

CONV_FN(JoinSpecification, specification) {
	switch (specification.join_specification_oneof_case()) {
	case JoinSpecification::kUsingSpecification:
		return JoinSpecificationUsingToString(
			specification.using_specification()
			);
	default:
		return JoinSpecificationOnExpressionToString(
			specification.on_expr()
			);
	}
}

CONV_FN(JoinedTable, joined_table) {
	std::string ret = "( "
		+ JoinSourceToString(joined_table.left_join_source())
		+ JoinOperatorToString(joined_table.join_operator())
		+ JoinSourceToString(joined_table.right_join_source()) + ") ";
	if (joined_table.has_specification()) {
		ret += JoinSpecificationToString(joined_table.specification());
	}
	return ret;
}

CONV_FN(JoinSource, source) {
	switch (source.join_source_oneof_case()) {
	case JoinSource::kJoinedTable:
		return JoinedTableToString(source.joined_table());
	default:
		return TableNameToString(source.table_name_fallback());
	}
}

CONV_FN(SelectFromClause, option) {
	std::string ret("FROM ");
	switch (option.select_from_clause_oneof_case()) {
	case SelectFromClause::kOption1:
		ret += SelectFromClauseOption1ToString(
			option.option1()
			);
		break;
	case SelectFromClause::kOption2:
		ret += SelectFromClauseOption2ToString(
			option.option2()
			);
		break;
	default:
		ret += JoinSourceToString(
			option.join_source_fallback()
			);
		break;
	}
	return ret;
}

CONV_FN(ColumnAny, any) {
	return "* ";
}

CONV_FN(ColumnAsExpression, as_expr) {
	std::string ret = FunctionExpressionToString(as_expr.expression());
	if (as_expr.has_column_name()) {
		ret += "AS " + ColumnNameToString(as_expr.column_name());
	}
	return ret;
}

CONV_FN(SelectColumn, column) {
	std::string ret;

	switch (column.column_oneof_case()) {
	case SelectColumn::kAnyColumnFromTable:
		return TableNameToString(
			column.any_column_from_table()
			) + ".* ";
	case SelectColumn::kSelectColumnExpression:
		return ColumnAsExpressionToString(
			column.select_column_expression()
			);
	default:
		return ColumnAnyToString(column.any_fallback());
	}
}

CONV_FN(OrderByExpression, order_by_expr) {
	std::string ret = ColumnNameToString(order_by_expr.column_name());
	switch (order_by_expr.order()) {
	case OrderByExpression::ASCENDING:
		ret += "ASC ";
		break;
	case OrderByExpression::DESCENDING:
		ret += "DESC ";
		break;
	default:
		break;
	}
	return ret;
}

CONV_FN(OrderBy, order_by) {
	std::string ret = "ORDER BY "
		+ OrderByExpressionToString(order_by.expr());
	for (int i = 0; i < order_by.extra_exprs_size(); ++i) {
		ret += ", "
			+ OrderByExpressionToString(order_by.extra_exprs(i));
	}
	return ret;
}

CONV_FN(Limit, limit) {
	std::string ret = "LIMIT ";
	if (limit.has_offset()) {
		switch (limit.offset_symbol_enum()) {
		case Limit::OFFSET:
			ret += std::to_string(limit.limit()) + " OFFSET "
					+ std::to_string(limit.offset()) + " ";
			break;
		case Limit::COMMA:
			ret += std::to_string(limit.offset()) + " , "
					+ std::to_string(limit.limit()) + " ";
			break;
		}
	} else {
		ret += ret += std::to_string(limit.limit()) + " ";
	}
	return ret;
}

static bool
SelectColumnIsColumnAny(const SelectColumn &column)
{
	switch (column.column_oneof_case()) {
	case SelectColumn::kSelectColumnExpression:
		return false;
	default:
		return true;
	}
}

static bool
SelectHasColumnAny(const Select &select)
{
	if (SelectColumnIsColumnAny(select.column()))
		return true;
	for (int i = 0; i < select.extra_columns_size(); ++i) {
		if (SelectColumnIsColumnAny(select.extra_columns(i)))
			return true;
	}
	return false;
}

CONV_FN(SelectWhereExpression, where_expr) {
	return "WHERE " + PredicateExpressionToString(where_expr.expr());
}

CONV_FN(SelectGroupByExpression, goup_by_expr) {
	return "GROUP BY " + FunctionExpressionToString(goup_by_expr.expr());
}

CONV_FN(SelectHavingExpression, having_expr) {
	return "HAVING " + PredicateExpressionToString(having_expr.expr());
}

CONV_FN(Select, select) {
	std::string ret("SELECT ");

	switch (select.option()) {
	case Select::DISTINCT:
	case Select::ALL:
		ret += Select_SelectOptionEnum_Name(select.option()) + " ";
		break;
	default:
		break;
	}

	ret += SelectColumnToString(select.column());
	for (int i = 0; i < select.extra_columns_size(); ++i) {
		ret += ", " + SelectColumnToString(select.extra_columns(i));
	}

	if (select.from_clause_present_flag() || SelectHasColumnAny(select)) {
		ret += SelectFromClauseToString(select.from_clause());
	}

	if (select.has_where_expr()) {
		ret += SelectWhereExpressionToString(select.where_expr());
	}

	if (select.has_group_by_exr()) {
		ret += SelectGroupByExpressionToString(select.group_by_exr());
	}

	if (select.has_having_expr()) {
		ret += SelectHavingExpressionToString(select.having_expr());
	}

	if (select.has_order_by()) {
		ret += OrderByToString(select.order_by());
	}

	if (select.has_limit()) {
		ret += LimitToString(select.limit());
	}

	return ret;
}

CONV_FN(SQLQuery, query) {
	switch (query.query_oneof_case()) {
	case SQLQuery::kCreateTable:
		return CreateTableToString(query.create_table());
	case SQLQuery::kSelect:
		return SelectToString(query.select());
	default:
		return "";
	}
}

} /* namespace sql_fuzzer */
