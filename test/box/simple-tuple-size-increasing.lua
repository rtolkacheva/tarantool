#!/usr/bin/env tarantool

require('console').listen(os.getenv('ADMIN'))

box.cfg{
    memtx_memory = 512 * 1024 * 1024,
    memtx_max_tuple_size = 1024 * 1024,
    wal_mode = 'none'
}

