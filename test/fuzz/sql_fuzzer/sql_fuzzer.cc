#include <cstdlib>
#include <iostream>

#include <libprotobuf-mutator/port/protobuf.h>
#include <libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h>
#include "sql_query.pb.h"
#include "sql_query_proto_to_string.h"

DEFINE_PROTO_FUZZER(const sql_query::SQLQuery& query) {
    // std::string query_str = sql_fuzzer::SQLQueryToString(query);

    // if (::getenv("LPM_DUMP_NATIVE_INPUT") && query_str.size() != 0) {
    //     std::cout << "_________________________" << std::endl;
    //     std::cout << query_str << std::endl;
    //     std::cout << "------------------------" << std::endl;
    // }

    // sql_init();
    // struct sql *db = sql_get();
    // Parse sParse;
    // sql_parser_create(&sParse, db, current_session()->sql_flags);

    // sqlRunParser(&sParse, query_str.data());
    // sql_parser_destroy(&sParse);
}