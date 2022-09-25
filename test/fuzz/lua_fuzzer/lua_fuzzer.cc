extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "lua_grammar.pb.h"
#include "serializer.h"

#include <libprotobuf-mutator/port/protobuf.h>
#include <libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h>

DEFINE_PROTO_FUZZER(const lua_grammar::LuaBlock &message)
{
	lua_State *L = luaL_newstate();
	if (!L)
		return;

	std::string code = LuaBlockToString(message);

	if (::getenv("LPM_DUMP_NATIVE_INPUT") && code.size() != 0) {
		std::cout << "-------------------------" << std::endl;
		std::cout << code << std::endl;
	}

	/*
	 * See https://luajit.org/running.html
	 */
	luaL_dostring(L, "jit.opt.start('hotloop=1')");
	luaL_dostring(L, "jit.opt.start('hotexit=1')");
	luaL_dostring(L, "jit.opt.start('recunroll=1')");
	luaL_dostring(L, "jit.opt.start('callunroll=1')");

	luaL_openlibs(L);
	int status = luaL_loadbuffer(L, code.c_str(), code.size(), "fuzz_test");
	if (status == 0)
		/*
		 * Using lua_pcall (protected call) instead of lua_call to
		 * detect core dump here. Mostly, generated code is not
		 * semantically correct, so it is needed to describe Lua
		 * semantics for more interesting results and fuzzer tests.
		 */
		lua_pcall(L, 0, 0, LUA_MULTRET);

	lua_settop(L, 0);
}
