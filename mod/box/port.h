#ifndef INCLUDES_TARANTOOL_BOX_PORT_H
#define INCLUDES_TARANTOOL_BOX_PORT_H
/*
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the
 *    following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <util.h>

struct tuple;
struct port;

struct port_vtab
{
	u32* (*add_u32)(struct port *port);
	void (*dup_u32)(struct port *port, u32 num);
	void (*add_tuple)(struct port *port, struct tuple *tuple);
};

struct port
{
	struct port_vtab *vtab;
};

/**
 * A hack to keep tuples alive until iov_flush(fiber->iovec).
 * Is internal to port_iproto implementation, but is also
 * used in memcached.m, which doesn't use fiber->iovec.
 */
void iov_ref_tuple(struct tuple *tuple);

static inline u32*
port_add_u32(struct port *port)
{
	return (port->vtab->add_u32)(port);
}

static inline void
port_dup_u32(struct port *port, u32 num)
{
	(port->vtab->dup_u32)(port, num);
}

static inline void
port_add_tuple(struct port *port, struct tuple *tuple)
{
	(port->vtab->add_tuple)(port, tuple);
}

/** Reused in port_lua */
u32*
port_null_add_u32(struct port *port __attribute__((unused)));

void
port_null_dup_u32(struct port *port __attribute__((unused)),
		  u32 num __attribute__((unused)));

/** These do not have state currently, thus a single
 * instance is sufficient.
 */
extern struct port port_null;

struct port *
port_iproto_create();

#endif /* INCLUDES_TARANTOOL_BOX_PORT_H */
