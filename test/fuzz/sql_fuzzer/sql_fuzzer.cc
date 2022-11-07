#include <string>
#include <iostream>

#include <libprotobuf-mutator/port/protobuf.h>
#include <libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h>
#include "sql_query.pb.h"
#include "sql_query_proto_to_string.h"

#include "fiber.h"
#include "memory.h"
#include "coll/coll.h"
#include "box/box.h"
#include "box/memtx_tx.h"
#include "box/module_cache.h"
#include "box/sql.h"

#include "box/execute.h"
#include "core/port.h"
#include "rmean.h"

#define SUCCESS 0

extern struct rmean *rmean_box;

DEFINE_PROTO_FUZZER(const sql_query::SQLQuery &query) {
	std::string query_str = sql_fuzzer::SQLQueryToString(query);

	if (query_str.length() == 0)
		return;

	if (::getenv("LPM_DUMP_NATIVE_INPUT") && query_str.size() != 0) {
		std::cout << "_________________________" << std::endl;
		std::cout << query_str << std::endl;
		std::cout << "-------------------------" << std::endl;
	}

	memory_init();
	fiber_init(fiber_cxx_invoke);
	coll_init();
	box_init();
	sql_init();

	struct port p;

	rmean_box = rmean_new(iproto_type_strs, IPROTO_TYPE_STAT_MAX);

	auto retval = sql_prepare(query_str.data(), query_str.size(), &p);

	if (retval == -1) {
		exit(-1);
	}

	// struct Expr *expr = sql_expr_compile(sql_get(), query_str.data(),
	// 				     query_str.size());

	// if (expr)
	// 	sql_expr_delete(sql_get(), expr);

	box_free();
	coll_free();
	fiber_free();
	memory_free();
}
