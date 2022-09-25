/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2022, Tarantool AUTHORS, please see AUTHORS file.
 */
#include "serializer.h"

PROTO_TOSTRING(LuaBlock, block)
{
	return ChunkToString(block.cur_chunk());
}

PROTO_TOSTRING(Chunk, chunk)
{
	std::string ret;
	for (int i = 0; i < chunk.stat_size(); ++i) {
		ret += StatementWithSemicolonToString(chunk.stat(i));
		ret += "\n";
	}

	if (chunk.has_last()) {
		ret += LastStatementWithSemicolonToString(chunk.last());
		ret += " ";
	}

	return ret;
}

PROTO_TOSTRING(StatementWithSemicolon, stat_semi)
{
	std::string ret = StatementToString(stat_semi.stat());
	if (stat_semi.has_semicolon())
		ret += "; ";
	return ret;
}

PROTO_TOSTRING(LastStatementWithSemicolon, last_stat_semi)
{
	std::string ret = LastStatementToString(last_stat_semi.stat());
	if (last_stat_semi.has_semicolon())
		ret += "; ";
	return ret;
}

PROTO_TOSTRING(Statement, stat)
{
	using StatType = Statement::StatOneofCase;
	switch (stat.stat_oneof_case()) {
	case StatType::kList:
		return AssignmentListToString(stat.list());
	case StatType::kCall:
		return FunctionCallToString(stat.call());
	case StatType::kBlock:
		return DoBlockToString(stat.block());
	/*
	 * Commented due to possible generation of infinite loops.
	 * In that case, fuzzer will drop only by timeout.
	 * Example: 'while true do end'.
	 */
	/*
	 * case StatType::kWhileCycle:
	 *      return WhileCycleToString(stat.whilecycle());
	 * case StatType::kRepeatCycle:
	 *	return RepeatCycleToString(stat.repeatcycle());
	 */
	case StatType::kIfStat:
		return IfStatementToString(stat.ifstat());
	case StatType::kForCycleName:
		return ForCycleNameToString(stat.forcyclename());
	case StatType::kForCycleList:
		return ForCycleListToString(stat.forcyclelist());
	case StatType::kFunc:
		return FunctionToString(stat.func());
	case StatType::kLocalFunc:
		return LocalFuncToString(stat.localfunc());
	case StatType::kNames:
		return LocalNamesToString(stat.names());
	default:
		/* Chosen arbitrarily more for simplicity. */
		return AssignmentListToString(stat.list());
	}
}

PROTO_TOSTRING(LastStatement, lastStat)
{
	using LastStatType = LastStatement::LastOneofCase;
	switch (lastStat.last_oneof_case()) {
	case LastStatType::kReturn:
		return ReturnLastToString(lastStat.return_());
	case LastStatType::kBreak:
		return "break";
	default:
		/* Chosen as default in order to decrease number of 'break's. */
		return ReturnLastToString(lastStat.return_());
	}
}

PROTO_TOSTRING(ReturnLast, returnLast)
{
	std::string ret = "return";
	if (returnLast.has_exprlist()) {
		ret += " " + ExpressionListToString(returnLast.exprlist());
		ret += " ";
	}
	return ret;
}

PROTO_TOSTRING(AssignmentList, list)
{
	std::string ret;
	ret += VariableListToString(list.varlist());
	ret += " = ";
	ret += ExpressionListToString(list.exprlist());
	return ret;
}

PROTO_TOSTRING(FunctionCall, call)
{
	using FuncCallType = FunctionCall::CallOneofCase;
	switch (call.call_oneof_case()) {
	case FuncCallType::kPrefArgs:
		return PrefixArgsToString(call.prefargs());
	case FuncCallType::kNamedArgs:
		return PrefixNamedArgsToString(call.namedargs());
	default:
		/* Chosen for more variability of generated programs. */
		return PrefixNamedArgsToString(call.namedargs());
	}
}

PROTO_TOSTRING(PrefixArgs, args)
{
	std::string ret = PrefixExpressionToString(args.prefix());
	ret += " ";
	ret += ArgsToString(args.arguments());
	return ret;
}

PROTO_TOSTRING(PrefixNamedArgs, args)
{
	std::string ret = PrefixExpressionToString(args.prefix());
	ret += ": ";
	ret += ConvertToString(args.name());
	ret += " ";
	ret += ArgsToString(args.arguments());
	return ret;
}

PROTO_TOSTRING(Args, args)
{
	using ArgsType = Args::ArgsOneofCase;
	switch (args.args_oneof_case()) {
	case ArgsType::kExprList:
	{
		std::string ret = OptionalExpressionListToString(
			args.exprlist());
		return "(" + ret + ")";
	}
	case ArgsType::kTable:
		return TableConstructorToString(args.table());
	case ArgsType::kStr:
		return "\'" + ConvertToString(args.str()) + "\'";
	default:
		/* For more variability. */
		return TableConstructorToString(args.table());
	}
}

PROTO_TOSTRING(DoBlock, block)
{
	std::string ret = "do ";
	ret += LuaBlockToString(block.block());
	ret += " end";
	return ret;
}

PROTO_TOSTRING(WhileCycle, cycle)
{
	std::string ret = "while ";
	ret += ExpressionToString(cycle.condition());
	ret += " do ";
	ret += LuaBlockToString(cycle.code());
	ret += " end";
	return ret;
}

PROTO_TOSTRING(RepeatCycle, cycle)
{
	std::string ret = "repeat ";
	ret += LuaBlockToString(cycle.code());
	ret += " until ";
	ret += ExpressionToString(cycle.condition());
	return ret;
}

PROTO_TOSTRING(IfStatement, ifStat)
{
	std::string ret = "if " + ExpressionToString(ifStat.firstcondition());
	ret += " then " + LuaBlockToString(ifStat.firstblock());
	for (int i = 0; i < ifStat.clauses_size(); ++i)
		ret += " " + ElseIfToString(ifStat.clauses(i)) + " ";

	if (ifStat.has_lastblock())
		ret += " else " + LuaBlockToString(ifStat.lastblock());

	ret += " end";
	return ret;
}

PROTO_TOSTRING(ElseIf, elifStat)
{
	std::string ret = "else if ";
	ret += ExpressionToString(elifStat.condition());
	ret += " then ";
	ret += LuaBlockToString(elifStat.code());
	return ret;
}

PROTO_TOSTRING(ForCycleName, cycle)
{
	std::string ret = "for " + ConvertToString(cycle.name());
	ret += " = " + ExpressionToString(cycle.init());
	ret += ", " + ExpressionToString(cycle.max_min());

	if (cycle.has_increment())
		ret += ", " + ExpressionToString(cycle.increment());

	ret += " do " + LuaBlockToString(cycle.code());
	ret += " end";
	return ret;
}

PROTO_TOSTRING(ForCycleList, cycle)
{
	std::string ret = "for " + NameListToString(cycle.names());
	ret += " in " + ExpressionListToString(cycle.expressions());
	ret += " do " + LuaBlockToString(cycle.code()) + " end";
	return ret;
}

PROTO_TOSTRING(Function, func)
{
	std::string ret = "function ";
	ret += FuncNameToString(func.name());
	ret += " ";
	ret += FuncBodyToString(func.body());
	return ret;
}

PROTO_TOSTRING(FuncName, name)
{
	std::string ret = ConvertToString(name.firstname());

	for (int i = 0; i < name.names_size(); ++i)
		ret += "." + ConvertToString(name.names(i));

	if (name.has_lastname())
		ret += ":" + ConvertToString(name.lastname());
	return ret;
}

PROTO_TOSTRING(FuncBody, body)
{
	std::string ret = "( ";
	if (body.has_list())
		ret += ParListToString(body.list());
	ret += " ) ";
	ret += LuaBlockToString(body.block());
	ret += " end ";
	return ret;
}

PROTO_TOSTRING(ParList, list)
{
	using ParListType = ParList::ParlistOneofCase;
	switch (list.parlist_oneof_case()) {
	case ParListType::kList:
		return NameListWithEllipsisToString(list.list());
	case ParListType::kEllipsis:
		return "...";
	default:
		/* Chosen as default in order to decrease number of ellipsises.
		 */
		return NameListWithEllipsisToString(list.list());
	}
}

PROTO_TOSTRING(NameListWithEllipsis, list)
{
	std::string ret = NameListToString(list.names());
	if (list.has_ellipsis()) {
		ret += ", ";
		ret += "...";
	}
	return ret;
}

PROTO_TOSTRING(NameList, list)
{
	std::string ret = ConvertToString(list.firstname());
	for (int i = 0; i < list.names_size(); ++i)
		ret += ", " + ConvertToString(list.names(i));
	return ret;
}

PROTO_TOSTRING(LocalFunc, func)
{
	std::string ret = "local function ";
	ret += ConvertToString(func.name());
	ret += " ";
	ret += FuncBodyToString(func.body());
	return ret;
}

PROTO_TOSTRING(LocalNames, localNames)
{
	std::string ret = "local ";
	ret += NameListToString(localNames.names());

	if (localNames.has_expressions()) {
		ret += " = ";
		ret += ExpressionListToString(localNames.expressions());
	}
	return ret;
}

PROTO_TOSTRING(VariableList, varList)
{
	std::string ret = VariableToString(varList.var());
	for (int i = 0; i < varList.varnext_size(); ++i) {
		ret += ", " + VariableToString(varList.varnext(i));
		ret += " ";
	}
	return ret;
}

PROTO_TOSTRING(ExpressionList, exprList)
{
	std::string ret;
	for (int i = 0; i < exprList.expr_size(); ++i)
		ret += ExpressionToString(exprList.expr(i)) + ", ";
	ret += ExpressionToString(exprList.lastexpr());
	ret += " ";
	return ret;
}

PROTO_TOSTRING(OptionalExpressionList, exprList)
{
	if (exprList.has_list())
		return ExpressionListToString(exprList.list());
	return " ";
}

PROTO_TOSTRING(Variable, var)
{
	using VarType = Variable::VarOneofCase;
	switch (var.var_oneof_case()) {
	case VarType::kName:
		return ConvertToString(var.name());
	case VarType::kExpr:
		return PrefixExpressionWithExpressionToString(var.expr());
	case VarType::kExprName:
		return PrefixExpressionWithNameToString(var.exprname());
	default:
		/* Can be generated too nested expressions with other options,
		 * though they can be enabled for more variable fuzzing.
		 */
		return ConvertToString(var.name());
	}
}

PROTO_TOSTRING(PrefixExpressionWithExpression, expr)
{
	std::string ret = PrefixExpressionToString(expr.prefix());
	ret += "[" + ExpressionToString(expr.expr()) + "]";
	return ret;
}

PROTO_TOSTRING(PrefixExpressionWithName, expr)
{
	std::string ret = PrefixExpressionToString(expr.prefix());
	ret += ".";
	ret += ConvertToString(expr.name());
	return ret;
}

PROTO_TOSTRING(PrefixExpression, prefExpr)
{
	using PrefExprType = PrefixExpression::PrefixOneofCase;
	switch (prefExpr.prefix_oneof_case()) {
	case PrefExprType::kVar:
		return VariableToString(prefExpr.var());
	case PrefExprType::kCall:
		return FunctionCallToString(prefExpr.call());
	case PrefExprType::kExpr:
		return "(" + ExpressionToString(prefExpr.expr()) + ")";
	default:
		/* Can be generated too nested expressions with other options,
		 * though they can be enabled for more variable fuzzing.
		 */
		return VariableToString(prefExpr.var());
	}
}

PROTO_TOSTRING(Expression, expr)
{
	using ExprType = Expression::ExprOneofCase;
	switch (expr.expr_oneof_case()) {
	case ExprType::kNil:
		return "nil";
	case ExprType::kFalse:
		return "false";
	case ExprType::kTrue:
		return "true";
	case ExprType::kNumber:
		return std::to_string(expr.number() % kMaxNumber);
	case ExprType::kStr:
		return "\'" + ConvertToString(expr.str()) + "\'";
	case ExprType::kEllipsis:
		return " ... ";
	case ExprType::kFunc:
		return AnonFuncToString(expr.func());
	case ExprType::kPrefExpr:
		return PrefixExpressionToString(expr.prefexpr());
	case ExprType::kTable:
		return TableConstructorToString(expr.table());
	case ExprType::kExpBinary:
		return ExpBinaryOpExpToString(expr.expbinary());
	case ExprType::kExpUnary:
		return UnaryOpExpToString(expr.expunary());
	default:
		/* Arbitrary choice */
		return "\'" + ConvertToString(expr.str()) + "\'";
	}
}

PROTO_TOSTRING(ExpBinaryOpExp, expr)
{
	std::string ret = ExpressionToString(expr.left());
	ret += " " + BinaryOperatorToString(expr.op()) + " ";
	ret += ExpressionToString(expr.right());
	return ret;
}

PROTO_TOSTRING(UnaryOpExp, expr)
{
	std::string ret = UnaryOperatorToString(expr.op());
	ret += ExpressionToString(expr.expr());
	return ret;
}

PROTO_TOSTRING(AnonFunc, func)
{
	std::string ret = "function ";
	ret += FuncBodyToString(func.body());
	return ret;
}

PROTO_TOSTRING(TableConstructor, table)
{
	std::string ret = "{ ";
	if (table.has_list())
		ret += FieldListToString(table.list());
	ret += " }";
	return ret;
}

PROTO_TOSTRING(FieldList, list)
{
	std::string ret = FieldToString(list.firstfield());
	for (int i = 0; i < list.fields_size(); ++i)
		ret += FieldWithFieldSepToString(list.fields(i));
	if (list.has_lastsep())
		ret += FieldSepToString(list.lastsep());
	return ret;
}

PROTO_TOSTRING(FieldWithFieldSep, field)
{
	std::string ret = FieldSepToString(field.sep());
	ret += " ";
	ret += FieldToString(field.cur_field());
	return ret;
}

PROTO_TOSTRING(Field, field)
{
	using FieldType = Field::FieldOneofCase;
	switch (field.field_oneof_case()) {
	case FieldType::kExprDouble:
		return ExpressionEqualExpressionToString(field.exprdouble());
	case FieldType::kName:
		return NameEqualExpressionToString(field.name());
	case FieldType::kExpr:
		return ExpressionToString(field.expr());
	default:
		/* More common case of using fields. */
		return NameEqualExpressionToString(field.name());
	}
}

PROTO_TOSTRING(ExpressionEqualExpression, equality)
{
	std::string ret = "[ ";
	ret += ExpressionToString(equality.leftexpr());
	ret += " ]";
	ret += " = ";
	ret += ExpressionToString(equality.rightexpr());
	return ret;
}

PROTO_TOSTRING(NameEqualExpression, equality)
{
	std::string ret = ConvertToString(equality.name());
	ret += " = ";
	ret += ExpressionToString(equality.expr());
	return ret;
}

PROTO_TOSTRING(FieldSep, sep)
{
	using FieldSepType = FieldSep::SepOneofCase;
	switch (sep.sep_oneof_case()) {
	case FieldSepType::kComma:
		return ",";
	case FieldSepType::kSemicolon:
		return ";";
	default:
		return ",";
	}
}

PROTO_TOSTRING(BinaryOperator, op)
{
	using BinopType = BinaryOperator::BinaryOneofCase;
	switch (op.binary_oneof_case()) {
	case BinopType::kAdd:
		return "+";
	case BinopType::kSub:
		return "-";
	case BinopType::kMult:
		return "*";
	case BinopType::kDiv:
		return "/";
	case BinopType::kExp:
		return "^";
	case BinopType::kMod:
		return "%";
	case BinopType::kConcat:
		return "..";
	case BinopType::kLess:
		return "<";
	case BinopType::kLessEqual:
		return "<=";
	case BinopType::kGreater:
		return ">";
	case BinopType::kGreaterEqual:
		return ">=";
	case BinopType::kEqual:
		return "==";
	case BinopType::kNotEqual:
		return "~=";
	case BinopType::kAnd:
		return "and";
	case BinopType::kOr:
		return "or";
	default:
		/* Works in most cases. */
		return "==";
	}
}

PROTO_TOSTRING(UnaryOperator, op)
{
	using UnaryopType = UnaryOperator::UnaryOneofCase;
	switch (op.unary_oneof_case()) {
	case UnaryopType::kNegate:
		return "-";
	case UnaryopType::kNot:
		return "not";
	case UnaryopType::kLength:
		return "#";
	default:
		/* Works in most cases. */
		return "not";
	}
}

std::string
ConvertToString(const std::string &s, const std::string &def)
{
	if (s.empty())
		return def;

	std::string ret;

	if (std::isalpha(s[0]) || s[0] == '_')
		ret += s[0];

	for (size_t i = 1; i < s.length() && i < kMaxStrLength; i++)
		if (std::iswalnum(s[i]) || s[i] == '_')
			ret += s[i];

	if (ret.empty())
		return def;

	return ret;
}
