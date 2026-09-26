#ifndef MRUBY_YK_H
#define MRUBY_YK_H

#include <yk.h>
#include <mruby.h>
#include <mruby/irep.h>

extern YkMT *yk_mt;

void yk_init(void);

void yk_shutdown(void);

YkLocation *yk_init_loc(mrb_state *mrb, const mrb_irep *irep);

void yk_free_loc(mrb_state *mrb, mrb_irep *irep);

extern YkLocation yk_null_loc;

static inline void
mrb_jit_yk_hook(mrb_state *mrb, const mrb_irep *irep, const mrb_code *pc)
{
  YkLocation *locs = (YkLocation*)irep->yk_locs;
  if (yk_is_interpreting()) {
    if (!locs) {
      locs = yk_init_loc(mrb, irep);
      ((mrb_irep*)irep)->yk_locs = locs;
    }
    if (locs) {
      YkLocation *loc = &locs[(size_t)(pc - irep->iseq)];
      if (pc == irep->iseq) {
        if (!irep->called) {
          ((mrb_irep*)irep)->called = TRUE;
        }
      }
      else if (yk_location_is_null(*loc)) {
        *loc = yk_location_new();
      }
    }
  }
  YkLocation *loc = &yk_null_loc;
  if (locs) {
    loc = &locs[(size_t)(pc - irep->iseq)];
  }
  yk_mt_control_point(yk_mt, loc);
}

#endif /* MRUBY_YK_H */
