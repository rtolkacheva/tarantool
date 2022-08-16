#include "utils.h"

#include "box/box.h"
#include "box/blackhole.h"
#include "box/service_engine.h"
#include "box/sysview.h"
#include "box/vinyl.h"
#include "cfg.h"
#include "memtx_engine.h"

void
engine_init(void)
{
	/*
	 * Sic: order is important here, since
	 * memtx must be the first to participate
	 * in checkpoints (in enigne_foreach order),
	 * so it must be registered first.
	 */
	struct memtx_engine *memtx;
	memtx = memtx_engine_new_xc(cfg_gets("memtx_dir"),
				    cfg_geti("force_recovery"),
				    cfg_getd("memtx_memory"),
				    cfg_geti("memtx_min_tuple_size"),
				    cfg_geti("strip_core"),
				    cfg_geti("slab_alloc_granularity"),
				    cfg_gets("memtx_allocator"),
				    cfg_getd("slab_alloc_factor"));
	engine_register((struct engine *)memtx);
	box_set_memtx_max_tuple_size();

	struct sysview_engine *sysview = sysview_engine_new_xc();
	engine_register((struct engine *)sysview);

	struct engine *service_engine = service_engine_new_xc();
	engine_register(service_engine);

	struct engine *blackhole = blackhole_engine_new_xc();
	engine_register(blackhole);

	struct engine *vinyl;
	vinyl = vinyl_engine_new_xc(cfg_gets("vinyl_dir"),
				    cfg_geti64("vinyl_memory"),
				    cfg_geti("vinyl_read_threads"),
				    cfg_geti("vinyl_write_threads"),
				    cfg_geti("force_recovery"));
	engine_register((struct engine *)vinyl);
	box_set_vinyl_max_tuple_size();
	box_set_vinyl_cache();
	box_set_vinyl_timeout();
}
