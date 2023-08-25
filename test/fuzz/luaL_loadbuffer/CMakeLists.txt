add_executable(luaL_loadbuffer_fuzzer
               luaL_loadbuffer_fuzzer.cc
               serializer.cc)

add_library(lua_grammar-proto)

foreach(lib ${LPM_LIBRARIES})
    find_library(${lib} REQUIRED_FILES)
endforeach(lib)

protobuf_generate(LANGUAGE cpp
                  TARGET lua_grammar-proto
                  PROTOS lua_grammar.proto)

target_link_libraries(lua_grammar-proto
                      ${PROTOBUF_LIBRARIES})

target_include_directories(luaL_loadbuffer_fuzzer PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(luaL_loadbuffer_fuzzer
                      PUBLIC
                      lua_grammar-proto
                      ${LPM_LIBRARIES}
                      libluajit_static
                      fuzzer_config)

add_dependencies(luaL_loadbuffer_fuzzer
                 libluajit_static
                 ${LPM_LIBRARIES}
                 lua_grammar-proto)

set(FUZZ_TEST_TARGETS "${FUZZ_TEST_TARGETS};luaL_loadbuffer_fuzzer" PARENT_SCOPE)

add_library(lua_grammar_old-proto)
protobuf_generate(LANGUAGE cpp
                  TARGET lua_grammar_old-proto
                  PROTOS lua_grammar_old.proto)

target_link_libraries(lua_grammar_old-proto
                      ${PROTOBUF_LIBRARIES})

add_executable(luaL_proto_converter
               converter.cc)

add_dependencies(luaL_proto_converter
                 ${LPM_LIBRARIES}
                 lua_grammar-proto
                 lua_grammar_old-proto)

target_link_libraries(luaL_proto_converter
                      PUBLIC
                      lua_grammar-proto
                      lua_grammar_old-proto
                      ${LPM_LIBRARIES})

target_include_directories(luaL_proto_converter PUBLIC ${CMAKE_CURRENT_BINARY_DIR})