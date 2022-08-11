#include <cstdio>
#include <string>
#include <iostream>

#include "main.h"
#include "fiber.h"
#include "cbus.h"
#include "memory.h"
#include "coio_task.h"
#include "coll/coll.h"
#include "trivia/util.h"
#include "box/box.h"
#include "box/error.h"
#include "random.h"
#include "box/memtx_tx.h"
#include "box/module_cache.h"
#include "systemd.h"
#include "core/ssl.h"
#include "core/popen.h"
#include <crc32.h>
#include "core/crash.h"
#include "box/execute.h"
#include "box/sql.h"

int 
main(int argc, char **argv) 
{
	random_init(); //+
	crc32_init(); //+
	memory_init(); //+
	exception_init();
	fiber_init(fiber_cxx_invoke); //+
	popen_init(); //+
	cbus_init(); //+
	coll_init(); //+
	memtx_tx_manager_init(); //+
	module_init(); //+
	ssl_init(); //+
	systemd_init(); //+
    box_init(); //+
    sql_init();

    std::string query = "CREATE TABLE t (s1 SCALAR PRIMARY KEY)";

	struct port port;
    int result = sql_prepare_and_execute(query.data(), query.length(), 
		nullptr, 0, &port, &fiber()->gc);
    printf("%d\n", result);

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