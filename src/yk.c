#include <mruby.h>

#ifdef USE_YK
#include <stdlib.h>
#include <mruby/irep.h>
#include <mruby/opcode.h>
#include <mruby/yk.h>

/* ==== instruction sizes, to walk an iseq without a full opcode decode ===
 * Built from the canonical opcode list (mruby/ops.h) instead of copied by
 * hand, so it can never drift from the FETCH_/READ_ decode tables in
 * mruby/opcode.h that vm.c itself uses to decode the same bytecode. */
#define Z 1
#define S 3
#define W 4
MRB_YK_STATIC const uint8_t mrb_jit_yk_insn_size[] = {
#define B 2
#define BB 3
#define BBB 4
#define BS 4
#define BSS 6
#define OPCODE(_,x) x,
#include <mruby/ops.h>
#undef OPCODE
#undef B
#undef BB
#undef BBB
#undef BS
#undef BSS
};
MRB_YK_STATIC const uint8_t mrb_jit_yk_insn_size1[] = {
#define B 3
#define BB 4
#define BBB 5
#define BS 5
#define BSS 7
#define OPCODE(_,x) x,
#include <mruby/ops.h>
#undef OPCODE
#undef B
#undef BB
#undef BBB
#undef BS
#undef BSS
};
MRB_YK_STATIC const uint8_t mrb_jit_yk_insn_size2[] = {
#define B 2
#define BB 4
#define BBB 5
#define BS 4
#define BSS 6
#define OPCODE(_,x) x,
#include <mruby/ops.h>
#undef OPCODE
#undef B
#undef BB
#undef BBB
#undef BS
#undef BSS
};
MRB_YK_STATIC const uint8_t mrb_jit_yk_insn_size3[] = {
#define B 3
#define BB 5
#define BBB 6
#define BS 5
#define BSS 7
#define OPCODE(_,x) x,
#include <mruby/ops.h>
#undef OPCODE
#undef B
#undef BB
#undef BBB
#undef BS
#undef BSS
};
#undef Z
#undef S
#undef W

YkMT *yk_mt = NULL;
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
    uint8_t op = pc[0];
    const mrb_code *next;
    const mrb_code *target;
    int16_t off;
    size_t idx;

    switch (op) {
    case OP_EXT1: pc += mrb_jit_yk_insn_size1[pc[1]] + 1; continue;
    case OP_EXT2: pc += mrb_jit_yk_insn_size2[pc[1]] + 1; continue;
    case OP_EXT3: pc += mrb_jit_yk_insn_size3[pc[1]] + 1; continue;

    // Ruby `redo` re-runs the current loop iteration from the top. 
    // It compiles to OP_JMPUW (same layout as OP_JMP)
    case OP_JMP:
    case OP_JMPUW:
      off = (int16_t)PEEK_S(pc + 1);
      next = pc + 3;
      break;

    case OP_JMPIF:
    case OP_JMPNOT:
    case OP_JMPNIL:
      off = (int16_t)PEEK_S(pc + 2);
      next = pc + 4;
      break;

    default:
      pc += mrb_jit_yk_insn_size[op];
      continue;
    }

    target = next + off;
    if (target < next && target >= irep->iseq) {
      idx = (size_t)(target - irep->iseq);
      if (yk_location_is_null(locs[idx])) {
        locs[idx] = yk_location_new();
      }
    }
    pc = next;
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
