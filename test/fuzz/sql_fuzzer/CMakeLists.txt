# External dependencies
include(ProtobufMutator)
include(protobuf) # FIXME double building protobuf

set(CMAKE_CXX_STANDARD 11)

add_executable(sql_fuzzer)

add_library(sql-query-proto)

foreach(lib ${LPM_LIBRARIES})
    find_library(${lib} REQUIRED_FILES)
endforeach(lib)

protobuf_generate(LANGUAGE cpp
                  TARGET sql-query-proto
                  PROTOS sql_query.proto)
target_link_libraries(sql-query-proto
                      ${PROTOBUF_LIBRARIES}
                      )

target_link_libraries(sql_fuzzer PUBLIC sql-query-proto ${LPM_LIBRARIES} fuzzer_config)

# set_property(TARGET sql_fuzzer
#              PROPERTY COMPILE_FLAGS "-fsanitize=fuzzer-no-link,address -fsanitize-coverage=trace-cmp -fprofile-instr-generate -fcoverage-mapping -g")
set_property(TARGET sql-query-proto
             PROPERTY COMPILE_FLAGS "-fsanitize=fuzzer-no-link,address -fsanitize-coverage=trace-cmp -fprofile-instr-generate")
# target_link_libraries(sql_fuzzer
#                       sql-query-proto
#                       ${LPM_LIBRARIES}
#                       "-fsanitize=fuzzer,address -fsanitize-coverage=trace-cmp -fprofile-instr-generate"
#                       )
