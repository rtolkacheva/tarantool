#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

#include <google/protobuf/text_format.h>

#include "lua_grammar.pb.h"
#include "lua_grammar_old.pb.h"

#define PROTO_CONVERT(TYPE, VAR_NAME) \
	lua_grammar::TYPE TYPE##Convert(const lua_grammar_old::TYPE & VAR_NAME)
#define NESTED_PROTO_CONVERT(TYPE, VAR_NAME, PARENT_TYPE) \
	lua_grammar::PARENT_TYPE::TYPE TYPE##Convert(const lua_grammar_old::PARENT_TYPE::TYPE & VAR_NAME)

namespace {

namespace lg = ::lua_grammar;
namespace lg_old = ::lua_grammar_old;

// Order from .proto file, same everywhere

PROTO_CONVERT(Block, block);
PROTO_CONVERT(Chunk, chunk);
PROTO_CONVERT(Statement, stat);

PROTO_CONVERT(AssignmentList, assignmentlist);
NESTED_PROTO_CONVERT(VariableList, varlist, AssignmentList);

PROTO_CONVERT(FunctionCall, call);
NESTED_PROTO_CONVERT(Args, args, FunctionCall);
NESTED_PROTO_CONVERT(PrefixArgs, args, FunctionCall);
NESTED_PROTO_CONVERT(PrefixNamedArgs, args, FunctionCall);

PROTO_CONVERT(DoBlock, doblock);
PROTO_CONVERT(WhileCycle, cycle);
PROTO_CONVERT(RepeatCycle, cycle);

PROTO_CONVERT(IfStatement, ifstat);
NESTED_PROTO_CONVERT(ElseIfBlock, elseif_block, IfStatement);

PROTO_CONVERT(ForCycleName, for_name);
PROTO_CONVERT(ForCycleList, for_list);

PROTO_CONVERT(Function, func);
NESTED_PROTO_CONVERT(FuncName, func_name, Function);

PROTO_CONVERT(FuncBody, func_body);
NESTED_PROTO_CONVERT(NameListWithEllipsis, ellipsis, FuncBody);
NESTED_PROTO_CONVERT(ParList, parlist, FuncBody);

PROTO_CONVERT(NameList, namelist);
PROTO_CONVERT(LocalFunc, localfunc);
PROTO_CONVERT(LocalNames, to_conv);

PROTO_CONVERT(LastStatement, laststat);
NESTED_PROTO_CONVERT(ReturnOptionalExpressionList, explist, LastStatement);

PROTO_CONVERT(ExpressionList, to_conv);
PROTO_CONVERT(OptionalExpressionList, to_conv);

PROTO_CONVERT(Variable, to_conv);
NESTED_PROTO_CONVERT(IndexWithExpression, to_conv, Variable);
NESTED_PROTO_CONVERT(IndexWithName, to_conv, Variable);

PROTO_CONVERT(PrefixExpression, to_conv);

PROTO_CONVERT(Expression, to_conv);
NESTED_PROTO_CONVERT(AnonFunc, to_conv, Expression);
NESTED_PROTO_CONVERT(ExpBinaryOpExp, to_conv, Expression);
NESTED_PROTO_CONVERT(UnaryOpExp, to_conv, Expression);

PROTO_CONVERT(TableConstructor, to_conv);

PROTO_CONVERT(FieldList, to_conv);
NESTED_PROTO_CONVERT(FieldWithFieldSep, to_conv, FieldList);

PROTO_CONVERT(Field, to_conv);
NESTED_PROTO_CONVERT(ExpressionAssignment, to_conv, Field);
NESTED_PROTO_CONVERT(NameAssignment, to_conv, Field);

PROTO_CONVERT(FieldSep, to_conv);
PROTO_CONVERT(BinaryOperator, to_conv);
PROTO_CONVERT(UnaryOperator, to_conv);
PROTO_CONVERT(Name, to_conv);

PROTO_CONVERT(Block, block)
{
	lg::Block converted;
	*converted.mutable_chunk() = ChunkConvert(block.chunk());
	return converted;
}

PROTO_CONVERT(Chunk, chunk)
{
	lg::Chunk converted;

	for (int i = 0; i < chunk.stat_size(); ++i) {
		converted.mutable_stat()->Add(StatementConvert(chunk.stat(i)));
	}
	if (chunk.has_laststat()) {
		std::unique_ptr<lg::LastStatement> laststat_ptr(new lg::LastStatement());
		*laststat_ptr = LastStatementConvert(chunk.laststat());
		converted.set_allocated_laststat(laststat_ptr.release());
	}

	return converted;
}

PROTO_CONVERT(Statement, stat)
{
	lg::Statement converted;

	std::unique_ptr<lg::AssignmentList> list_ptr;
	std::unique_ptr<lg::FunctionCall> call_ptr;
	std::unique_ptr<lg::DoBlock> block_ptr;
	std::unique_ptr<lg::WhileCycle> whilecycle_ptr;
	std::unique_ptr<lg::RepeatCycle> repeatcycle_ptr;
	std::unique_ptr<lg::IfStatement> ifstat_ptr;
	std::unique_ptr<lg::ForCycleName> forcyclename_ptr;
	std::unique_ptr<lg::ForCycleList> forcyclelist_ptr;
	std::unique_ptr<lg::Function> func_ptr;
	std::unique_ptr<lg::LocalFunc> localfunc_ptr;
	std::unique_ptr<lg::LocalNames> localnames_ptr;

	switch (stat.stat_oneof_case()) {
	case lg_old::Statement::StatOneofCase::kList:
		list_ptr.reset(new lg::AssignmentList());
		*list_ptr = AssignmentListConvert(stat.list());
		converted.set_allocated_list(list_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kCall:
		call_ptr.reset(new lg::FunctionCall());
		*call_ptr = FunctionCallConvert(stat.call());
		converted.set_allocated_call(call_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kBlock:
		block_ptr.reset(new lg::DoBlock());
		*block_ptr = DoBlockConvert(stat.block());
		converted.set_allocated_block(block_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kWhilecycle:
		whilecycle_ptr.reset(new lg::WhileCycle());
		*whilecycle_ptr = WhileCycleConvert(stat.whilecycle());
		converted.set_allocated_whilecycle(whilecycle_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kRepeatcycle:
		repeatcycle_ptr.reset(new lg::RepeatCycle());
		*repeatcycle_ptr = RepeatCycleConvert(stat.repeatcycle());
		converted.set_allocated_repeatcycle(repeatcycle_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kIfstat:
		ifstat_ptr.reset(new lg::IfStatement());
		*ifstat_ptr = IfStatementConvert(stat.ifstat());
		converted.set_allocated_ifstat(ifstat_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kForcyclename:
		forcyclename_ptr.reset(new lg::ForCycleName());
		*forcyclename_ptr = ForCycleNameConvert(stat.forcyclename());
		converted.set_allocated_forcyclename(forcyclename_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kForcyclelist:
		forcyclelist_ptr.reset(new lg::ForCycleList());
		*forcyclelist_ptr = ForCycleListConvert(stat.forcyclelist());
		converted.set_allocated_forcyclelist(forcyclelist_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kFunc:
		func_ptr.reset(new lg::Function());
		*func_ptr = FunctionConvert(stat.func());
		converted.set_allocated_func(func_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kLocalfunc:
		localfunc_ptr.reset(new lg::LocalFunc());
		*localfunc_ptr = LocalFuncConvert(stat.localfunc());
		converted.set_allocated_localfunc(localfunc_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::kLocalnames:
		localnames_ptr.reset(new lg::LocalNames());
		*localnames_ptr = LocalNamesConvert(stat.localnames());
		converted.set_allocated_localnames(localnames_ptr.release());
		break;
	case lg_old::Statement::StatOneofCase::STAT_ONEOF_NOT_SET:
		break;
	}

	if (stat.has_semicolon()) {
		converted.set_semicolon(stat.semicolon());
	}

	return converted;
}

PROTO_CONVERT(AssignmentList, assignmentlist)
{
	lg::AssignmentList converted;
	*converted.mutable_varlist() = VariableListConvert(assignmentlist.varlist());
	*converted.mutable_explist() = ExpressionListConvert(assignmentlist.explist());
	return converted;
}

NESTED_PROTO_CONVERT(VariableList, varlist, AssignmentList)
{
	lg::AssignmentList::VariableList converted;

	*converted.mutable_var() = VariableConvert(varlist.var());
	for (int i = 0; i < varlist.vars_size(); ++i) {
		converted.mutable_vars()->Add(VariableConvert(varlist.vars(i)));
	}

	return converted;
}

PROTO_CONVERT(FunctionCall, call)
{
	lg::FunctionCall converted;

	std::unique_ptr<lg::FunctionCall::PrefixArgs> pref_args_ptr;
	std::unique_ptr<lg::FunctionCall::PrefixNamedArgs> named_args_ptr;

	switch (call.call_oneof_case()) {
	case lg_old::FunctionCall::CallOneofCase::kPrefArgs:
		pref_args_ptr.reset(new lg::FunctionCall::PrefixArgs());
		*pref_args_ptr = PrefixArgsConvert(call.prefargs());
		converted.set_allocated_prefargs(pref_args_ptr.release());
		break;
	case lg_old::FunctionCall::CallOneofCase::kNamedArgs:
		named_args_ptr.reset(new lg::FunctionCall::PrefixNamedArgs());
		*named_args_ptr = PrefixNamedArgsConvert(call.namedargs());
		converted.set_allocated_namedargs(named_args_ptr.release());
		break;
	case lg_old::FunctionCall::CallOneofCase::CALL_ONEOF_NOT_SET:
		break;
	}

	return converted;
}

NESTED_PROTO_CONVERT(Args, args, FunctionCall)
{
	lg::FunctionCall::Args converted;

	std::unique_ptr<lg::OptionalExpressionList> explist_ptr;
	std::unique_ptr<lg::TableConstructor> tableconstructor_ptr;
	std::unique_ptr<std::string> str_ptr;

	switch (args.args_oneof_case()) {
	case lg_old::FunctionCall::Args::ArgsOneofCase::kExplist:
		explist_ptr.reset(new lg::OptionalExpressionList());
		*explist_ptr = OptionalExpressionListConvert(args.explist());
		converted.set_allocated_explist(explist_ptr.release());
		break;
	case lg_old::FunctionCall::Args::ArgsOneofCase::kTableconstructor:
		tableconstructor_ptr.reset(new lg::TableConstructor());
		*tableconstructor_ptr = TableConstructorConvert(args.tableconstructor());
		converted.set_allocated_tableconstructor(tableconstructor_ptr.release());
		break;
	case lg_old::FunctionCall::Args::ArgsOneofCase::kStr:
		str_ptr.reset(new std::string());
		*str_ptr = args.str();
		converted.set_allocated_str(str_ptr.release());
		break;
	case lg_old::FunctionCall::Args::ArgsOneofCase::ARGS_ONEOF_NOT_SET:
		break;
	}

	return converted;
}

NESTED_PROTO_CONVERT(PrefixArgs, prefixargs, FunctionCall)
{
	lg::FunctionCall::PrefixArgs converted;
	*converted.mutable_prefixexp() = PrefixExpressionConvert(prefixargs.prefixexp());
	*converted.mutable_args() = ArgsConvert(prefixargs.args());
	return converted;
}

NESTED_PROTO_CONVERT(PrefixNamedArgs, prefixnamedargs, FunctionCall)
{
	lg::FunctionCall::PrefixNamedArgs converted;
	*converted.mutable_prefixexp() = PrefixExpressionConvert(prefixnamedargs.prefixexp());
	*converted.mutable_name() = NameConvert(prefixnamedargs.name());
	*converted.mutable_args() = ArgsConvert(prefixnamedargs.args());
	return converted;
}

PROTO_CONVERT(DoBlock, block)
{
	lg::DoBlock converted;
	*converted.mutable_block() = BlockConvert(block.block());
	return converted;
}

PROTO_CONVERT(WhileCycle, whilecycle)
{
	lg::WhileCycle converted;
	*converted.mutable_condition() = ExpressionConvert(whilecycle.condition());
	*converted.mutable_doblock() = DoBlockConvert(whilecycle.doblock());
	return converted;
}

PROTO_CONVERT(RepeatCycle, repeatcycle)
{
	lg::RepeatCycle converted;
	*converted.mutable_block() = BlockConvert(repeatcycle.block());
	*converted.mutable_condition() = ExpressionConvert(repeatcycle.condition());
	return converted;
}

PROTO_CONVERT(IfStatement, statement)
{
	lg::IfStatement converted;

	*converted.mutable_condition() = ExpressionConvert(statement.condition());
	*converted.mutable_first() = BlockConvert(statement.first());
	for (int i = 0; i < statement.clauses_size(); ++i) {
		converted.mutable_clauses()->Add(ElseIfBlockConvert(statement.clauses(i)));
	}
	if (statement.has_last()) {
		std::unique_ptr<lg::Block> last_ptr(new lg::Block());
		*last_ptr = BlockConvert(statement.last());
		converted.set_allocated_last(last_ptr.release());
	}

	return converted;
}

NESTED_PROTO_CONVERT(ElseIfBlock, elseif_block, IfStatement)
{
	lg::IfStatement::ElseIfBlock converted;

	*converted.mutable_condition() = ExpressionConvert(elseif_block.condition());
	*converted.mutable_block() = BlockConvert(elseif_block.block());

	return converted;
}

PROTO_CONVERT(ForCycleName, forcyclename)
{
	lg::ForCycleName converted;

	*converted.mutable_name() = NameConvert(forcyclename.name());
	*converted.mutable_startexp() = ExpressionConvert(forcyclename.startexp());
	*converted.mutable_stopexp() = ExpressionConvert(forcyclename.stopexp());
	*converted.mutable_doblock() = DoBlockConvert(forcyclename.doblock());

	if (forcyclename.has_stepexp()) {
		std::unique_ptr<lg::Expression> stepexp_ptr(new lg::Expression());
		*stepexp_ptr = ExpressionConvert(forcyclename.stepexp());
		converted.set_allocated_stepexp(stepexp_ptr.release());
	}

	return converted;
}

PROTO_CONVERT(ForCycleList, forcyclelist)
{
	lg::ForCycleList converted;
	*converted.mutable_names() = NameListConvert(forcyclelist.names());
	*converted.mutable_expressions() = ExpressionListConvert(forcyclelist.expressions());
	*converted.mutable_doblock() = DoBlockConvert(forcyclelist.doblock());
	return converted;
}

PROTO_CONVERT(Function, func)
{
	lg::Function converted;
	*converted.mutable_name() = FuncNameConvert(func.name());
	*converted.mutable_body() = FuncBodyConvert(func.body());
	return converted;
}

NESTED_PROTO_CONVERT(FuncName, funcname, Function)
{
	lg::Function::FuncName converted;

	*converted.mutable_firstname() = NameConvert(funcname.firstname());
	for (int i = 0; i < funcname.names_size(); ++i) {
		converted.mutable_names()->Add(NameConvert(funcname.names(i)));
	}
	if (funcname.has_lastname()) {
		std::unique_ptr<lg::Name> lastname_ptr(new lg::Name());
		*lastname_ptr = NameConvert(funcname.lastname());
		converted.set_allocated_lastname(lastname_ptr.release());
	}

	return converted;
}

PROTO_CONVERT(FuncBody, body)
{
	lg::FuncBody converted;

	if (body.has_parlist()) {
		std::unique_ptr<lg::FuncBody::ParList> parlist_ptr(new lg::FuncBody::ParList());
		*parlist_ptr = ParListConvert(body.parlist());
		converted.set_allocated_parlist(parlist_ptr.release());
	}
	*converted.mutable_block() = BlockConvert(body.block());

	return converted;
}

NESTED_PROTO_CONVERT(NameListWithEllipsis, namelist, FuncBody)
{
	lg::FuncBody::NameListWithEllipsis converted;

	*converted.mutable_namelist() = NameListConvert(namelist.namelist());
	if (namelist.has_ellipsis()) {
		std::unique_ptr<std::string> ellipsis_ptr(new std::string());
		*ellipsis_ptr = namelist.ellipsis();
		converted.set_allocated_ellipsis(ellipsis_ptr.release());
	}

	return converted;
}

NESTED_PROTO_CONVERT(ParList, parlist, FuncBody)
{
	lg::FuncBody::ParList converted;

	std::unique_ptr<lg::FuncBody::NameListWithEllipsis> namelist_ptr;
	std::unique_ptr<std::string> ellipsis_ptr;

	switch (parlist.parlist_oneof_case()) {
	case lg_old::FuncBody::ParList::ParlistOneofCase::kNamelist:
		namelist_ptr.reset(new lg::FuncBody::NameListWithEllipsis());
		*namelist_ptr = NameListWithEllipsisConvert(parlist.namelist());
		converted.set_allocated_namelist(namelist_ptr.release());
		break;
	case lg_old::FuncBody::ParList::ParlistOneofCase::kEllipsis:
		ellipsis_ptr.reset(new std::string());
		*ellipsis_ptr = parlist.ellipsis();
		converted.set_allocated_ellipsis(ellipsis_ptr.release());
		break;
	default:
		break;
	}

	return converted;
}

PROTO_CONVERT(NameList, namelist)
{
	lg::NameList converted;

	*converted.mutable_firstname() = NameConvert(namelist.firstname());
	for (int i = 0; i < namelist.names_size(); ++i) {
		converted.mutable_names()->Add(NameConvert(namelist.names(i)));
	}

	return converted;
}

PROTO_CONVERT(LocalFunc, localfunc)
{
	lg::LocalFunc converted;
	*converted.mutable_name() = NameConvert(localfunc.name());
	*converted.mutable_funcbody() = FuncBodyConvert(localfunc.funcbody());
	return converted;
}

PROTO_CONVERT(LocalNames, localnames)
{
	lg::LocalNames converted;

	*converted.mutable_namelist() = NameListConvert(localnames.namelist());
	if (localnames.has_explist()) {
		std::unique_ptr<lg::ExpressionList> ellipsis_ptr(new lg::ExpressionList());
		*ellipsis_ptr = ExpressionListConvert(localnames.explist());
		converted.set_allocated_explist(ellipsis_ptr.release());
	}

	return converted;
}

PROTO_CONVERT(LastStatement, laststat)
{
	lg::LastStatement converted;

	std::unique_ptr<lg::LastStatement::ReturnOptionalExpressionList> exptlist_ptr;

	switch (laststat.last_oneof_case()) {
	case lg_old::LastStatement::LastOneofCase::kExplist:
		exptlist_ptr.reset(new lg::LastStatement::ReturnOptionalExpressionList());
		*exptlist_ptr = ReturnOptionalExpressionListConvert(laststat.explist());
		converted.set_allocated_explist(exptlist_ptr.release());
		break;
	case lg_old::LastStatement::LastOneofCase::kBreak:
		converted.set_break_(laststat.break_());
		break;
	case lg_old::LastStatement::LastOneofCase::LAST_ONEOF_NOT_SET:
		break;
	}

	if (laststat.has_semicolon()) {
		converted.set_semicolon(laststat.semicolon());
	}

	return converted;
}

NESTED_PROTO_CONVERT(ReturnOptionalExpressionList, explist, LastStatement)
{
	lg::LastStatement::ReturnOptionalExpressionList converted;
	if (explist.has_explist()) {
		std::unique_ptr<lg::ExpressionList> explist_ptr(new lg::ExpressionList());
		*explist_ptr = ExpressionListConvert(explist.explist());
		converted.set_allocated_explist(explist_ptr.release());
	}
	return converted;
}

PROTO_CONVERT(ExpressionList, explist)
{
	lg::ExpressionList converted;

	for (int i = 0; i < explist.expressions_size(); ++i) {
		converted.mutable_expressions()->Add(ExpressionConvert(explist.expressions(i)));
	}
	*converted.mutable_explast() = ExpressionConvert(explist.explast());

	return converted;
}

PROTO_CONVERT(OptionalExpressionList, explist)
{
	lg::OptionalExpressionList converted;
	if (explist.has_explist()) {
		std::unique_ptr<lg::ExpressionList> explist_ptr(new lg::ExpressionList());
		*explist_ptr = ExpressionListConvert(explist.explist());
		converted.set_allocated_explist(explist_ptr.release());
	}
	return converted;
}

PROTO_CONVERT(Variable, var)
{
	lg::Variable converted;

	std::unique_ptr<lg::Name> name_ptr;
	std::unique_ptr<lg::Variable::IndexWithExpression> indexexpr_ptr;
	std::unique_ptr<lg::Variable::IndexWithName> indexname_ptr;

	switch (var.var_oneof_case()) {
	case lg_old::Variable::VarOneofCase::kName:
		name_ptr.reset(new lg::Name());
		*name_ptr = NameConvert(var.name());
		converted.set_allocated_name(name_ptr.release());
		break;
	case lg_old::Variable::VarOneofCase::kIndexexpr:
		indexexpr_ptr.reset(new lg::Variable::IndexWithExpression());
		*indexexpr_ptr = IndexWithExpressionConvert(var.indexexpr());
		converted.set_allocated_indexexpr(indexexpr_ptr.release());
		break;
	case lg_old::Variable::VarOneofCase::kIndexname:
		indexname_ptr.reset(new lg::Variable::IndexWithName());
		*indexname_ptr = IndexWithNameConvert(var.indexname());
		converted.set_allocated_indexname(indexname_ptr.release());
		break;
	default:
		break;
	}

	return converted;
}

NESTED_PROTO_CONVERT(IndexWithExpression, indexexpr, Variable)
{
	lg::Variable::IndexWithExpression converted;
	*converted.mutable_prefixexp() = PrefixExpressionConvert(indexexpr.prefixexp());
	*converted.mutable_exp() = ExpressionConvert(indexexpr.exp());
	return converted;
}

NESTED_PROTO_CONVERT(IndexWithName, indexname, Variable)
{
	lg::Variable::IndexWithName converted;
	*converted.mutable_prefixexp() = PrefixExpressionConvert(indexname.prefixexp());
	*converted.mutable_name() = indexname.name();
	return converted;
}

PROTO_CONVERT(PrefixExpression, prefixexp)
{
	lg::PrefixExpression converted;

	std::unique_ptr<lg::Variable> var_ptr;
	std::unique_ptr<lg::FunctionCall> functioncall_ptr;
	std::unique_ptr<lg::Expression> exp_ptr;

	switch (prefixexp.prefix_oneof_case()) {
	case lg_old::PrefixExpression::PrefixOneofCase::kVar:
		var_ptr.reset(new lg::Variable());
		*var_ptr = VariableConvert(prefixexp.var());
		converted.set_allocated_var(var_ptr.release());
		break;
	case lg_old::PrefixExpression::PrefixOneofCase::kFunctioncall:
		functioncall_ptr.reset(new lg::FunctionCall());
		*functioncall_ptr = FunctionCallConvert(prefixexp.functioncall());
		converted.set_allocated_functioncall(functioncall_ptr.release());
		break;
	case lg_old::PrefixExpression::PrefixOneofCase::kExp:
		exp_ptr.reset(new lg::Expression());
		*exp_ptr = ExpressionConvert(prefixexp.exp());
		converted.set_allocated_exp(exp_ptr.release());
		break;
	default:
		break;
	}

	return converted;
}

PROTO_CONVERT(Expression, expr)
{
	lg::Expression converted;

	std::unique_ptr<std::string> str_ptr;
	std::unique_ptr<std::string> ellipsis_ptr;
	std::unique_ptr<lg::Expression::AnonFunc> function_ptr;
	std::unique_ptr<lg::PrefixExpression> prefixexp_ptr;
	std::unique_ptr<lg::TableConstructor> tableconstructor_ptr;
	std::unique_ptr<lg::Expression::ExpBinaryOpExp> binary_ptr;
	std::unique_ptr<lg::Expression::UnaryOpExp> unary_ptr;

	switch (expr.expr_oneof_case()) {
	case lg_old::Expression::ExprOneofCase::kNil:
		converted.set_nil(expr.nil());
		break;
	case lg_old::Expression::ExprOneofCase::kFalse:
		converted.set_false_(expr.false_());
		break;
	case lg_old::Expression::ExprOneofCase::kTrue:
		converted.set_true_(expr.true_());
		break;
	case lg_old::Expression::ExprOneofCase::kNumber:
		converted.set_number(expr.number());
		break;
	case lg_old::Expression::ExprOneofCase::kStr:
		str_ptr.reset(new std::string());
		*str_ptr = expr.str();
		converted.set_allocated_str(str_ptr.release());
		break;
	case lg_old::Expression::ExprOneofCase::kEllipsis:
		ellipsis_ptr.reset(new std::string());
		*ellipsis_ptr = expr.ellipsis();
		converted.set_allocated_ellipsis(ellipsis_ptr.release());
		break;
	case lg_old::Expression::ExprOneofCase::kFunction:
		function_ptr.reset(new lg::Expression::AnonFunc());
		*function_ptr = AnonFuncConvert(expr.function());
		converted.set_allocated_function(function_ptr.release());
		break;
	case lg_old::Expression::ExprOneofCase::kPrefixexp:
		prefixexp_ptr.reset(new lg::PrefixExpression());
		*prefixexp_ptr = PrefixExpressionConvert(expr.prefixexp());
		converted.set_allocated_prefixexp(prefixexp_ptr.release());
		break;
	case lg_old::Expression::ExprOneofCase::kTableconstructor:
		tableconstructor_ptr.reset(new lg::TableConstructor());
		*tableconstructor_ptr = TableConstructorConvert(expr.tableconstructor());
		converted.set_allocated_tableconstructor(tableconstructor_ptr.release());
		break;
	case lg_old::Expression::ExprOneofCase::kBinary:
		binary_ptr.reset(new lg::Expression::ExpBinaryOpExp());
		*binary_ptr = ExpBinaryOpExpConvert(expr.binary());
		converted.set_allocated_binary(binary_ptr.release());
		break;
	case lg_old::Expression::ExprOneofCase::kUnary:
		unary_ptr.reset(new lg::Expression::UnaryOpExp());
		*unary_ptr = UnaryOpExpConvert(expr.unary());
		converted.set_allocated_unary(unary_ptr.release());
		break;
	default:
		break;
	}

	return converted;
}

NESTED_PROTO_CONVERT(AnonFunc, func, Expression)
{
	lg::Expression::AnonFunc converted;
	*converted.mutable_body() = FuncBodyConvert(func.body());
	return converted;
}

NESTED_PROTO_CONVERT(ExpBinaryOpExp, binary, Expression)
{
	lg::Expression::ExpBinaryOpExp converted;
	*converted.mutable_leftexp() = ExpressionConvert(binary.leftexp());
	*converted.mutable_binop() = BinaryOperatorConvert(binary.binop());
	*converted.mutable_rightexp() = ExpressionConvert(binary.rightexp());
	return converted;
}

NESTED_PROTO_CONVERT(UnaryOpExp, unary, Expression)
{
	lg::Expression::UnaryOpExp converted;
	*converted.mutable_unop() = UnaryOperatorConvert(unary.unop());
	*converted.mutable_exp() = ExpressionConvert(unary.exp());
	return converted;
}

PROTO_CONVERT(TableConstructor, table)
{
	lg::TableConstructor converted;
	if (table.has_fieldlist()) {
		std::unique_ptr<lg::FieldList> fieldlist_ptr(new lg::FieldList());
		*fieldlist_ptr = FieldListConvert(table.fieldlist());
		converted.set_allocated_fieldlist(fieldlist_ptr.release());
	}
	return converted;
}

PROTO_CONVERT(FieldList, fieldlist)
{
	lg::FieldList converted;

	*converted.mutable_firstfield() = FieldConvert(fieldlist.firstfield());
	for (int i = 0; i < fieldlist.fields_size(); ++i) {
		converted.mutable_fields()->Add(FieldWithFieldSepConvert(fieldlist.fields(i)));
	}
	if (fieldlist.has_lastsep()) {
		std::unique_ptr<lg::FieldSep> lastsep_ptr(new lg::FieldSep());
		*lastsep_ptr = FieldSepConvert(fieldlist.lastsep());
		converted.set_allocated_lastsep(lastsep_ptr.release());
	}

	return converted;
}

NESTED_PROTO_CONVERT(FieldWithFieldSep, field, FieldList)
{
	lg::FieldList::FieldWithFieldSep converted;
	*converted.mutable_sep() = FieldSepConvert(field.sep());
	*converted.mutable_field() = FieldConvert(field.field());
	return converted;
}

PROTO_CONVERT(Field, field)
{
	lg::Field converted;

	std::unique_ptr<lg::Field::ExpressionAssignment> exprassign_ptr;
	std::unique_ptr<lg::Field::NameAssignment> namedassign_ptr;
	std::unique_ptr<lg::Expression> expression_ptr;

	switch (field.field_oneof_case()) {
	case lg_old::Field::FieldOneofCase::kExprassign:
		exprassign_ptr.reset(new lg::Field::ExpressionAssignment());
		*exprassign_ptr = ExpressionAssignmentConvert(field.exprassign());
		converted.set_allocated_exprassign(exprassign_ptr.release());
		break;
	case lg_old::Field::FieldOneofCase::kNamedassign:
		namedassign_ptr.reset(new lg::Field::NameAssignment());
		*namedassign_ptr = NameAssignmentConvert(field.namedassign());
		converted.set_allocated_namedassign(namedassign_ptr.release());
		break;
	case lg_old::Field::FieldOneofCase::kExpression:
		expression_ptr.reset(new lg::Expression());
		*expression_ptr = ExpressionConvert(field.expression());
		converted.set_allocated_expression(expression_ptr.release());
		break;
	default:
		break;
	}

	return converted;
}

NESTED_PROTO_CONVERT(ExpressionAssignment, assignment, Field)
{
	lg::Field::ExpressionAssignment converted;
	*converted.mutable_key() = ExpressionConvert(assignment.key());
	*converted.mutable_value() = ExpressionConvert(assignment.value());
	return converted;
}

NESTED_PROTO_CONVERT(NameAssignment, assignment, Field)
{
	lg::Field::NameAssignment converted;
	*converted.mutable_name() = NameConvert(assignment.name());
	*converted.mutable_value() = ExpressionConvert(assignment.value());
	return converted;
}

PROTO_CONVERT(FieldSep, sep)
{
	lg::FieldSep converted;

	switch (sep.sep_oneof_case()) {
	case lg_old::FieldSep::SepOneofCase::kComma:
		converted.set_comma(sep.comma());
		break;
	case lg_old::FieldSep::SepOneofCase::kSemicolon:
		converted.set_semicolon(sep.semicolon());
		break;
	default:
		break;
	}

	return converted;
}

PROTO_CONVERT(BinaryOperator, op)
{
	lg::BinaryOperator converted;
	switch (op.binary_oneof_case()) {
	case lg_old::BinaryOperator::BinaryOneofCase::kAdd:
		converted.set_add(op.add());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kSub:
		converted.set_sub(op.sub());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kMult:
		converted.set_mult(op.mult());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kDiv:
		converted.set_div(op.div());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kExp:
		converted.set_exp(op.exp());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kMod:
		converted.set_mod(op.mod());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kConcat:
		converted.set_concat(op.concat());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kLess:
		converted.set_less(op.less());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kLessEqual:
		converted.set_lessequal(op.lessequal());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kGreater:
		converted.set_greater(op.greater());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kGreaterEqual:
		converted.set_greaterequal(op.greaterequal());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kEqual:
		converted.set_equal(op.equal());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kNotEqual:
		converted.set_notequal(op.notequal());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kAnd:
		converted.set_and_(op.and_());
		break;
	case lg_old::BinaryOperator::BinaryOneofCase::kOr:
		converted.set_or_(op.or_());
		break;
	default:
		break;
	}
	return converted;
}

PROTO_CONVERT(UnaryOperator, op)
{
	lg::UnaryOperator converted;

	switch (op.unary_oneof_case()) {
	case lg_old::UnaryOperator::UnaryOneofCase::kNegate:
		converted.set_negate(op.negate());
		break;
	case lg_old::UnaryOperator::UnaryOneofCase::kNot:
		converted.set_not_(op.not_());
		break;
	case lg_old::UnaryOperator::UnaryOneofCase::kLength:
		converted.set_length(op.length());
		break;
	default:
		break;
	}

	return converted;
}

PROTO_CONVERT(Name, name)
{
	lg::Name converted;
	converted.set_name(name.name());
	converted.set_num(name.num());
	return converted;
}

} // namespace

int main(int argc, char** argv)
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <input> <output>" << std::endl;
		return -1;
	}
	std::ifstream input(argv[1]);
	std::ofstream output(argv[2]);

	if (!input.good()) {
		std::cerr << "Failed to open " << argv[1] << std::endl;
		return -1;
	}
	if (!output.good()) {
		std::cerr << "Failed to open " << argv[2] << std::endl;
		return -1;
	}

	std::stringstream buffer;
	buffer << input.rdbuf();
	std::string proto_struct = buffer.str();
	input.close();

	// Read
	lua_grammar_old::Block block;
	google::protobuf::TextFormat::ParseFromString(proto_struct, &block);

	//convert
	lua_grammar::Block converted_block = BlockConvert(block);
	std::string converted_proto_struct;
	google::protobuf::TextFormat::PrintToString(converted_block, &converted_proto_struct);
	output << converted_proto_struct << std::endl;
}
