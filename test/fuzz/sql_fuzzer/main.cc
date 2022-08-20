#include <cstdio>
#include <string>
#include <iostream>

#include "box/box.h"
#include "box/error.h"
#include "box/execute.h"
#include "box/iproto.h"
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