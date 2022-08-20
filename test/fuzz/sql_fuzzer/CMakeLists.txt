# External dependencies
include(ProtobufMutator)
include(protobuf) # FIXME: double building protobuf
include(tarantool-c)

set(CMAKE_CXX_STANDARD 11)

add_executable(sql_fuzzer
               sql_fuzzer.cc
               sql_query_proto_to_string.cc
               sql_query_proto_to_string.h)

add_library(sql-query-proto)

# ????
foreach(lib ${LPM_LIBRARIES})
    find_library(${lib} REQUIRED_FILES)
endforeach(lib)

protobuf_generate(LANGUAGE cpp
                  TARGET sql-query-proto
                  PROTOS sql_query.proto)
target_link_libraries(sql-query-proto
                      ${PROTOBUF_LIBRARIES}
                      )

target_link_libraries(sql_fuzzer PUBLIC box core sql-query-proto ${LPM_LIBRARIES} fuzzer_config)

# set_property(TARGET sql_fuzzer
#              PROPERTY COMPILE_FLAGS "-fsanitize=fuzzer-no-link,address -fsanitize-coverage=trace-cmp -fprofile-instr-generate -fcoverage-mapping -g")
set_property(TARGET sql-query-proto
             PROPERTY COMPILE_FLAGS "-fsanitize=fuzzer-no-link,address -fsanitize-coverage=trace-cmp -fprofile-instr-generate")
# target_link_libraries(sql_fuzzer
#                       sql-query-proto
#                       ${LPM_LIBRARIES}
#                       "-fsanitize=fuzzer,address -fsanitize-coverage=trace-cmp -fprofile-instr-generate"
#                       )

set(LIBUTIL_FREEBSD_SRC ${PROJECT_SOURCE_DIR}/third_party/libutil_freebsd)
include_directories(${LIBUTIL_FREEBSD_SRC})

add_executable(test_sql main.cc 
${PROJECT_SOURCE_DIR}/test/unit/box_test_utils.c 
    utils.c
    utils.cc
    ${LIBUTIL_FREEBSD_SRC}/flopen.c
    ${LIBUTIL_FREEBSD_SRC}/pidfile.c)
target_link_libraries(test_sql PUBLIC box core -fsanitize=address)
