#ifndef INCLUDES_TARANTOOL_TEST_SQL_QUERY_PROTO_TO_STRING_H
#define INCLUDES_TARANTOOL_TEST_SQL_QUERY_PROTO_TO_STRING_H

#include <string>
#include "sql_query.pb.h"

namespace sql_fuzzer {

std::string SQLQueryToString(const sql_query::SQLQuery& query);

}

#endif // INCLUDES_TARANTOOL_TEST_SQL_QUERY_PROTO_TO_STRING_H
