#include <cstdio>
#include <string>
#include <iostream>

#include "box/box.h"
#include "box/error.h"
#include "box/execute.h"
#include "box/iproto.h"
#include "box/lua/init.h"
#include "box/memtx_tx.h"
#include "box/module_cache.h"
#include "box/sql.h"
#include "cbus.h"
#include "cfg.h"
#include "coio_task.h"
#include "coll/coll.h"
#include "core/crash.h"
#include "core/ssl.h"
#include "core/popen.h"
#include <crc32.h>
#include "engine.h"
#include "fiber.h"
#include "lua/init.h"
#include "main.h"
#include "memory.h"
#include "random.h"
#include "replication.h"
#include "schema.h"
#include "systemd.h"
#include "trivia/util.h"
#include "utils.h"

#include "lua/init.h"
#include "lua/utils.h"
#include <lua.h>
#include <lualib.h>
#include <luajit.h>
#include "lua/backtrace.h"
#include "lua/fiber.h"
#include "lua/fiber_cond.h"
#include "lua/fiber_channel.h"
#include "lua/errno.h"
#include "lua/socket.h"
#include "lua/utils.h"
#include "lua/serializer.h"
#include <lua-cjson/lua_cjson.h>
#include <lua-yaml/lyaml.h>
#include "lua/msgpack.h"
#include "lua/pickle.h"
#include "lua/fio.h"
#include "lua/popen.h"
#include "lua/httpc.h"
#include "lua/utf8.h"
#include "lua/swim.h"
#include "lua/decimal.h"
#include "lua/uri.h"

void my_box_cfg(void)
{
    try
    {
        engine_init();
        schema_init();
        replication_init(cfg_geti_default("replication_threads", 1));
        port_init();
        iproto_init(cfg_geti("iproto_threads"));
        sql_init();
        // box_set_net_msg_max();
        // box_set_readahead();
        // box_set_too_long_threshold();
        // box_set_replication_timeout();
        // box_set_replication_connect_timeout();
        // box_set_replication_connect_quorum();
        // box_set_replication_sync_lag();
        // if (box_set_replication_synchro_quorum() != 0)
        //     diag_raise();
        //     diag_raise();
        // box_set_replication_sync_timeout();
        // box_set_replication_skip_conflict();
        // box_set_replication_anon();
    }
    catch (Exception *e)
    {
        e->log();
        panic("can't initialize storage: %s", e->get_errmsg());
    }
}

static void fuzzer_lua_init() 
{
    lua_State *L = luaL_newstate();
	if (L == NULL) {
		panic("failed to initialize Lua");
	}
	luaL_openlibs(L);

	/* Initialize ffi to enable luaL_pushcdata/luaL_checkcdata functions */
	luaL_loadstring(L, "return require('ffi')");
	lua_call(L, 0, 0);
    luaopen_http_client_driver(L);
	lua_pop(L, 1);
	luaopen_msgpack(L);
	lua_pop(L, 1);
	luaopen_yaml(L);
	lua_pop(L, 1);
	luaopen_json(L);
	lua_pop(L, 1);
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_settop(L, 0);
}

int main(int argc, char **argv)
{
    start_time = ev_monotonic_time();
    random_init(); //+
    crc32_init();  //+
    memory_init(); //+
    exception_init();
    fiber_init(fiber_cxx_invoke); //+
    popen_init();                 //+
    cbus_init();                  //+
    coll_init();                  //+
    memtx_tx_manager_init();      //+
    module_init();                //+
    ssl_init();                   //+
    systemd_init();               //+
    box_init();                   //+
    fuzzer_lua_init();
    box_lua_init(tarantool_L);

    my_box_cfg();

    std::string query = "DROP TABLE t31;";

    struct port port;
    int result = sql_prepare_and_execute(query.data(), query.length(),
                                         nullptr, 0, &port, &fiber()->gc);
    printf("%d\n", result);

    // auto expr = sql_expr_compile(sql_get(), query.data(), query.length());
    // std::cout << expr << std::endl;

    box_free();
    systemd_free();
    ssl_free();
    module_free();
    memtx_tx_manager_free();
    coll_free();
    cbus_free();
    popen_free();
    fiber_free();
    memory_free();
    random_free();
}