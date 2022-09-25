/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2022, Tarantool AUTHORS, please see AUTHORS file.
 */
#pragma once

#include "lua_grammar.pb.h"

using namespace lua_grammar;

#define PROTO_TOSTRING(PROTO_TYPE, VAR_NAME) \
	std::string PROTO_TYPE##ToString(const PROTO_TYPE & (VAR_NAME))

/**
 * Fuzzing parameters:
 * kMaxNumber - upper bound for all generated numbers.
 * kMaxStrLength - upper bound for generating string literals and identifiers.
 *
 * Default values were chosen arbitrary but not too big for better readability
 * of generated code samples.
 */
constexpr size_t kMaxNumber = 1000;
constexpr size_t kMaxStrLength = 20;

PROTO_TOSTRING(LuaBlock, block);
PROTO_TOSTRING(Chunk, chunk);
PROTO_TOSTRING(StatementWithSemicolon, stat_semi);
PROTO_TOSTRING(LastStatementWithSemicolon, last_stat_semi);

PROTO_TOSTRING(Statement, stat);
PROTO_TOSTRING(LastStatement, lastStat);
PROTO_TOSTRING(ReturnLast, returnLast);

/** Lua grammar: statement options. */
PROTO_TOSTRING(AssignmentList, list);

PROTO_TOSTRING(FunctionCall, call);
PROTO_TOSTRING(PrefixArgs, args);
PROTO_TOSTRING(PrefixNamedArgs, args);
PROTO_TOSTRING(Args, args);

PROTO_TOSTRING(DoBlock, block);
PROTO_TOSTRING(WhileCycle, cycle);
PROTO_TOSTRING(RepeatCycle, cycle);

PROTO_TOSTRING(IfStatement, ifStat);
PROTO_TOSTRING(ElseIf, elifStat);

PROTO_TOSTRING(ForCycleName, cycle);
PROTO_TOSTRING(ForCycleList, cycle);

PROTO_TOSTRING(Function, func);
PROTO_TOSTRING(FuncName, name);
PROTO_TOSTRING(FuncBody, body);
PROTO_TOSTRING(ParList, list);
PROTO_TOSTRING(NameListWithEllipsis, list);
PROTO_TOSTRING(NameList, list);

PROTO_TOSTRING(LocalFunc, func);

PROTO_TOSTRING(LocalNames, localNames);

/** Lua grammar: variables and expressions. */
PROTO_TOSTRING(VariableList, varList);
PROTO_TOSTRING(ExpressionList, exprList);
PROTO_TOSTRING(OptionalExpressionList, exprList);

PROTO_TOSTRING(Variable, var);
PROTO_TOSTRING(PrefixExpressionWithExpression, expr);
PROTO_TOSTRING(PrefixExpressionWithName, expr);
PROTO_TOSTRING(PrefixExpression, prefExpr);

PROTO_TOSTRING(Expression, expr);
PROTO_TOSTRING(ExpBinaryOpExp, expr);
PROTO_TOSTRING(UnaryOpExp, expr);
PROTO_TOSTRING(AnonFunc, func);

/** Lua grammar: tables and fields. */
PROTO_TOSTRING(TableConstructor, table);
PROTO_TOSTRING(FieldList, list);
PROTO_TOSTRING(FieldWithFieldSep, field);
PROTO_TOSTRING(Field, field);
PROTO_TOSTRING(ExpressionEqualExpression, equality);
PROTO_TOSTRING(NameEqualExpression, equality);
PROTO_TOSTRING(FieldSep, sep);

/** Lua grammar: operators. */
PROTO_TOSTRING(BinaryOperator, op);
PROTO_TOSTRING(UnaryOperator, op);

std::string
ConvertToString(const std::string &s, const std::string &def = "a");
