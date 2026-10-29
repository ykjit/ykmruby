#include <mruby.h>

#ifdef USE_YK
#include <stdlib.h>
#include <mruby/irep.h>
#include <mruby/internal.h>
#include <mruby/opcode.h>
#include <mruby/yk.h>

YkMT *yk_mt = NULL;
uint64_t mrb_yk_iseq_gen = 0;
YkLocation yk_null_loc;

void yk_init(void)
{
  if (!yk_mt) {
    yk_mt = yk_mt_new(NULL);
    yk_null_loc = yk_location_null();
    atexit(yk_shutdown);
  }
}

void yk_shutdown(void)
{
  if (yk_mt) {
    yk_mt_shutdown(yk_mt);
    yk_mt = NULL;
  }
}

YkLocation *
yk_init_loc(mrb_state *mrb, const mrb_irep *irep)
{
  YkLocation *locs;
  const mrb_code *pc, *end;

  if (irep->ilen == 0) return NULL;
  locs = (YkLocation*)mrb_malloc(mrb, sizeof(YkLocation) * irep->ilen);
  for (uint32_t i = 0; i < irep->ilen; i++) {
    locs[i] = yk_location_null();
  }

  pc = irep->iseq;
  end = irep->iseq + irep->ilen;
  while (pc < end) {
    struct mrb_insn_data d = mrb_decode_insn(pc);

    pc = d.next;
    int16_t off;
    switch (d.insn) {
    case OP_JMP:
    case OP_JMPUW: off = (int16_t)d.a; break;
    case OP_JMPIF:
    case OP_JMPNOT:
    case OP_JMPNIL: off = (int16_t)d.b; break;
    default: continue;
    }

    const mrb_code *target = pc + off;
    if (off < 0 && target >= irep->iseq && yk_location_is_null(locs[target - irep->iseq])) {
      locs[target - irep->iseq] = yk_location_new();
    }
  }
  return locs;
}

void yk_free_loc(mrb_state *mrb, mrb_irep *irep)
{
  YkLocation *locs = (YkLocation*)irep->yk_locs;
  if (locs){
    for (uint32_t i = 0; i < irep->ilen; i++) {
      if (!yk_location_is_null(locs[i])) {
        yk_location_drop(locs[i]);
      }
    }
    mrb_free(mrb, locs);
  }
}
#endif /* USE_YK */
