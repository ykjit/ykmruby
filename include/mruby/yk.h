#ifndef MRUBY_YK_H
#define MRUBY_YK_H

#ifdef USE_YK
#include <yk.h>
#include <mruby.h>
#include <mruby/irep.h>

extern YkMT *yk_mt;

void yk_init(void);

void yk_shutdown(void);

YkLocation *yk_init_loc(mrb_state *mrb, const mrb_irep *irep);

void yk_free_loc(mrb_state *mrb, mrb_irep *irep);

extern YkLocation yk_null_loc;

/* mrb_yk_iseq_gen increased whenever an iseq is freed.
  Its used as idempotent key for bytecode fetching in the dispatch loop.*/
extern uint64_t mrb_yk_iseq_gen;

#endif /* USE_YK */
#endif /* MRUBY_YK_H */
