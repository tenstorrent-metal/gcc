#define INCLUDE_STRING
#include <unordered_map>
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "tm.h"
#include "rtl.h"
#include "tree.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "regs.h"
#include "insn-config.h"
#include "insn-attr.h"
#include "recog.h"
#include "output.h"
#include "alias.h"
#include "stringpool.h"
#include "attribs.h"
#include "varasm.h"
#include "stor-layout.h"
#include "calls.h"
#include "function.h"
#include "explow.h"
#include "memmodel.h"
#include "emit-rtl.h"
#include "reload.h"
#include "tm_p.h"
#include "target.h"
#include "target-def.h"
#include "basic-block.h"
#include "expr.h"
#include "optabs.h"
#include "bitmap.h"
#include "df.h"
#include "diagnostic.h"
#include "builtins.h"
#include "predict.h"
#include "tree-pass.h"
#include "ssa.h"
#include "tree-ssa.h"
#include "sfpu-protos.h"
#include "sfpu.h"
#include <vector>

#define DUMP(...) //fprintf(stderr, __VA_ARGS__)

const int riscv_sfpu_name_stub_no_arch_len = 15;
const int riscv_sfpu_name_stub_len = 18;

struct str_cmp
{
  bool operator()(const char *a, const char *b) const
  {
     return std::strcmp(a, b) == 0;
  }
};

struct str_hash
{
  std::size_t operator()(const char *cstr) const
  {
    std::size_t hash = 5381;
    for (; *cstr != '\0' ; ++cstr)
      hash = (hash * 33) + *cstr;
    return hash;
  }
};

unsigned int riscv_sfpu_cmp_ex_to_setcc_mod1_map[] = {
  0,
  SFPSETCC_MOD1_LREG_LT0,
  SFPSETCC_MOD1_LREG_EQ0,
  SFPSETCC_MOD1_LREG_GTE0,
  SFPSETCC_MOD1_LREG_NE0,
};

static std::unordered_map<const char*, riscv_sfpu_insn_data&, str_hash, str_cmp> insn_map;
static const int NUMBER_OF_ARCHES = 2;
static const int NUMBER_OF_INTRINSICS = 85;
static riscv_sfpu_insn_data sfpu_insn_data_target[NUMBER_OF_ARCHES][NUMBER_OF_INTRINSICS] = {
  {
#define SFPU_INTERNAL(id, nim) { riscv_sfpu_insn_data::id, #id, nullptr, false, false, false, -1, -1, -1, nim, true, 0, 0 },
#define SFPU_BUILTIN(id, fmt, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis) { riscv_sfpu_insn_data::id, #id, nullptr, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis },
#define SFPU_NO_TGT_BUILTIN(id, fmt, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis) { riscv_sfpu_insn_data::id, #id, nullptr, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis },
#define SFPU_GS_BUILTIN(id, fmt, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis) { riscv_sfpu_insn_data::id, #id, nullptr, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis },
#define SFPU_GS_NO_TGT_BUILTIN(id, fmt, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis) { riscv_sfpu_insn_data::id, #id, nullptr, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis },
#define SFPU_GS_PAD_BUILTIN(id) { riscv_sfpu_insn_data::id, #id, nullptr, false, false, false, 0, 0, 0, 0, false, 0, 0 },
#define SFPU_GS_PAD_NO_TGT_BUILTIN(id) { riscv_sfpu_insn_data::id, #id, nullptr, 0, 0, 0, 0, 0, 0, 0, false, 0, 0 },
#include "sfpu-insn.h"
    { riscv_sfpu_insn_data::nonsfpu, "nonsfpu", nullptr, false, false, false, 0, 0, 0, 0, false, 0, 0 }
  },
  {
#define SFPU_INTERNAL(id, nim) { riscv_sfpu_insn_data::id, #id, nullptr, 0, 0, 0, -1, -1, -1, nim, true, 0, 0 },
#define SFPU_BUILTIN(id, fmt, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis) { riscv_sfpu_insn_data::id, #id, nullptr, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis },
#define SFPU_NO_TGT_BUILTIN(id, fmt, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis) { riscv_sfpu_insn_data::id, #id, nullptr, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis },
#define SFPU_WH_BUILTIN(id, fmt, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis) { riscv_sfpu_insn_data::id, #id, nullptr, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis },
#define SFPU_WH_NO_TGT_BUILTIN(id, fmt, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis) { riscv_sfpu_insn_data::id, #id, nullptr, cc, lv, hho, dap, mp, sched, nip, ntr, nim, nis },
#include "sfpu-insn.h"
    { riscv_sfpu_insn_data::nonsfpu, "nonsfpu", nullptr, false, false, false, 0, 0, 0, 0, false, 0, 0 }
  }
};

static std::vector<const riscv_sfpu_insn_data *> sfpu_rtl_insn_ptrs;

static riscv_sfpu_insn_data *sfpu_insn_data = sfpu_insn_data_target[0];
static const char* riscv_sfpu_builtin_name_stub;
void
riscv_sfpu_insert_insn(int idx, const char* name, tree decl)
{
  static int start = 0;

  int arch;
  if (flag_grayskull) {
    arch = 0;
  } else if (flag_wormhole) {
    arch = 1;
  } else {
    return;
  }

  int offset = start;
  while (offset < NUMBER_OF_INTRINSICS)
    {
      // string is __rvtt_builtin_XX_<name>
      if ((strcmp(sfpu_insn_data_target[arch][offset].name, name) == 0) ||
	  (strcmp(sfpu_insn_data_target[arch][offset].name, &name[riscv_sfpu_name_stub_len]) == 0) ||
	  (strcmp(sfpu_insn_data_target[arch][offset].name, &name[riscv_sfpu_name_stub_no_arch_len]) == 0))
	{
	  sfpu_insn_data_target[arch][offset].decl = decl;
	  insn_map.insert(std::pair<const char*, riscv_sfpu_insn_data&>(name, sfpu_insn_data_target[arch][offset]));
	  start = offset + 1;
	  return;
	}
      offset++;
    }

  fprintf(stderr, "Failed to match insn %d named %s to builtin for arch index %d starting at builtin %d\n",
	  idx, name, arch, offset);
  gcc_assert(0);
}

static const riscv_sfpu_insn_data *
init_rtx_insnd(int code)
{
  DUMP("trying to lookup rtx name %d %s\n", code, insn_data[code].name);

  if (strncmp(insn_data[code].name, "riscv_", 6) == 0) {
    // There are multiple possible matches to the table name from the rtl [name]
    // due to expand calls going to _int insns and other internal names
    // Input is riscv_[name], matches can be:
    //  - name (literal match)
    //  - __builtin_rvtt_[arch]_name (matches a builtin)
    //  - __builtin_rvtt_[arch]_name (minus _int) (matches an internal name)
    //  - [arch]_namex_int (change sfpname to sfpxname (matches an sfpx builtin)

    // Try name
    const riscv_sfpu_insn_data *tmp = riscv_sfpu_get_insn_data(&insn_data[code].name[6]);
    if (tmp->id != riscv_sfpu_insn_data::nonsfpu) return tmp;

    // Try __builtin_rvtt_[arch]_name
    char name[100];
    int len = strlen(insn_data[code].name);
    sprintf(name, "__builtin_rvtt_");
    strncpy(&name[riscv_sfpu_name_stub_no_arch_len], &insn_data[code].name[6], len - 6);
    name[len + riscv_sfpu_name_stub_no_arch_len - 6] = 0;
    tmp = riscv_sfpu_get_insn_data(name);
    if (tmp->id != riscv_sfpu_insn_data::nonsfpu) return tmp;

    // Try __builtin_rvtt_[arch]_name (minus _int)
    if (strcmp(&insn_data[code].name[len - 4], "_int") == 0) {
      name[len + riscv_sfpu_name_stub_no_arch_len - 10] = 0;
      tmp = riscv_sfpu_get_insn_data(name);
      if (tmp->id != riscv_sfpu_insn_data::nonsfpu) return tmp;

      // Try [arch]_namex_int (change sfpname to sfpxname (matches an sfpx builtin)
      strncpy(&name[riscv_sfpu_name_stub_no_arch_len + 7],
	      &name[riscv_sfpu_name_stub_no_arch_len + 6],
	      strlen(name) - (riscv_sfpu_name_stub_no_arch_len + 6));
      name[riscv_sfpu_name_stub_no_arch_len + 6] = 'x';
      name[len + riscv_sfpu_name_stub_no_arch_len - 9] = 0;
      tmp = riscv_sfpu_get_insn_data(name);
      if (tmp->id != riscv_sfpu_insn_data::nonsfpu) return tmp;
    }
  }

  return &sfpu_insn_data[riscv_sfpu_insn_data::nonsfpu];
}

void
riscv_sfpu_init_builtins()
{
  int arch;
  if (flag_grayskull) {
    arch = 0;
    riscv_sfpu_builtin_name_stub = "__builtin_rvtt_gs";
  } else if (flag_wormhole) {
    arch = 1;
    riscv_sfpu_builtin_name_stub = "__builtin_rvtt_wh";
  } else {
    return;
  }
  sfpu_insn_data = sfpu_insn_data_target[arch];

  // Fill in the non-builtin internal insns
  for (int i = 0; i < NUMBER_OF_INTRINSICS; i++)
    {
      if (sfpu_insn_data[i].internal) {
	insn_map.insert(std::pair<const char*, riscv_sfpu_insn_data&>(sfpu_insn_data[i].name,
								      sfpu_insn_data[i]));
      }
    }

  // If these asserts fire, the sfpu-insn.h instruction tables are out of sync
  // across architectures
  for (int i = 1; i < NUMBER_OF_ARCHES; i++)
    {
      for (int j = 0; j < NUMBER_OF_INTRINSICS; j++)
	{
	  if (sfpu_insn_data_target[0][j].id != sfpu_insn_data_target[i][j].id)
	    {
	      fprintf(stderr, "SFPU intrinsic table element (%d, %d) does not match (%d != %d)!\n",
		      i, j, sfpu_insn_data_target[0][j].id, sfpu_insn_data_target[i][j].id);
	      gcc_assert(0);
	    }
	}

      gcc_assert(sfpu_insn_data_target[i][NUMBER_OF_INTRINSICS - 1].id == riscv_sfpu_insn_data::nonsfpu);
    }

  sfpu_rtl_insn_ptrs.resize(NUM_INSN_CODES);
  for (unsigned int i = 0; i < NUM_INSN_CODES; i++)
    {
      sfpu_rtl_insn_ptrs[i] = init_rtx_insnd(i);
    }
}

const char *
riscv_sfpu_get_builtin_name_stub()
{
  return riscv_sfpu_builtin_name_stub;
}

const riscv_sfpu_insn_data*
riscv_sfpu_get_insn_data(const riscv_sfpu_insn_data::insn_id id)
{
  return &sfpu_insn_data[id];
}

const riscv_sfpu_insn_data*
riscv_sfpu_get_insn_data(const char *name)
{
  auto match = insn_map.find(name);
  if (match == insn_map.end())
    {
      return &sfpu_insn_data[riscv_sfpu_insn_data::nonsfpu];
    }
  else
    {
      return &match->second;
    }
}

const riscv_sfpu_insn_data *
riscv_sfpu_get_insn_data(const gcall *stmt)
{
  tree fn_ptr = gimple_call_fn (stmt);

  if (fn_ptr)
    {
      return riscv_sfpu_get_insn_data(IDENTIFIER_POINTER (DECL_NAME (TREE_OPERAND (fn_ptr, 0))));
    }
  else
    {
      return nullptr;
    }
}

bool
riscv_sfpu_p(const riscv_sfpu_insn_data **insnd, gcall **stmt, gimple *g)
{
  bool found = false;

  if (g == nullptr || g->code != GIMPLE_CALL) return false;

  *stmt = dyn_cast<gcall *> (g);
  tree fn_ptr = gimple_call_fn (*stmt);

  if (fn_ptr && TREE_CODE (fn_ptr) == ADDR_EXPR)
    {
      tree fn_decl = TREE_OPERAND (fn_ptr, 0);
      *insnd = riscv_sfpu_get_insn_data(IDENTIFIER_POINTER (DECL_NAME (fn_decl)));
      found = (*insnd != nullptr && (*insnd)->id != riscv_sfpu_insn_data::nonsfpu);
    }

  return found;
}

bool
riscv_sfpu_p(const riscv_sfpu_insn_data **insnd, gcall **stmt, gimple_stmt_iterator gsi)
{
  bool found = false;
  gimple *g = gsi_stmt (gsi);

  if (g != nullptr && g->code == GIMPLE_CALL)
    {
      found = riscv_sfpu_p(insnd, stmt, g);
    }

  return found;
}

bool
riscv_sfpu_p(const riscv_sfpu_insn_data **insnd, rtx_insn *insn)
{
  int code = INSN_CODE(insn);
  *insnd = (code == -1) ? &sfpu_insn_data[riscv_sfpu_insn_data::nonsfpu] : sfpu_rtl_insn_ptrs[code];
  return (*insnd)->id != riscv_sfpu_insn_data::nonsfpu;
}

// Relies on live instructions being next in sequence in the insn table
const riscv_sfpu_insn_data *
riscv_sfpu_get_live_version(const riscv_sfpu_insn_data *insnd)
{
  const riscv_sfpu_insn_data *out = nullptr;

  if (insnd->id < riscv_sfpu_insn_data::nonsfpu)
    {
      if (sfpu_insn_data[insnd->id + 1].live)
	{
	  out = &sfpu_insn_data[insnd->id + 1];
	}
    }

  return out;
}

const riscv_sfpu_insn_data *
riscv_sfpu_get_notlive_version(const riscv_sfpu_insn_data *insnd)
{
  const riscv_sfpu_insn_data *out = insnd;

  if (insnd->live)
    {
      out = &sfpu_insn_data[insnd->id - 1];
    }

  return out;
}

static long int
get_int_arg(gcall *stmt, unsigned int arg)
{
  tree decl = gimple_call_arg(stmt, arg);
  if (decl)
  {
    gcc_assert(TREE_CODE(decl) == INTEGER_CST);
    return *(decl->int_cst.val);
  }
  return -1;
}

bool
riscv_sfpu_sets_cc(const riscv_sfpu_insn_data *insnd, gcall *stmt)
{
  bool sets_cc = false;

  if (insnd->can_set_cc)
    {
      long int arg = (insnd->mod_pos != -1) ? get_int_arg (stmt, insnd->mod_pos) : 0;
      if (insnd->id == riscv_sfpu_insn_data::sfpxiadd_i)
	{
	  if (arg & SFPXCMP_MOD1_CC_MASK)
	    sets_cc = true;
	}
      else if (insnd->id == riscv_sfpu_insn_data::sfpxiadd_v)
	{
	  if (arg & SFPXCMP_MOD1_CC_MASK)
	    sets_cc = true;
	}
      else if (insnd->id == riscv_sfpu_insn_data::sfpexexp)
	{
	  if (arg == 2 || arg == 3 || arg == 8 || arg == 9 || arg == 10 || arg == 11)
	    sets_cc = true;
	}
      else if (insnd->id == riscv_sfpu_insn_data::sfplz)
	{
	  if (arg == 2 || arg == 8 || arg == 10)
	    sets_cc = true;
	}
      else
	{
	  sets_cc = true;
	}
    }

  return sets_cc;
}

bool riscv_sfpu_permutable_operands(const riscv_sfpu_insn_data *insnd, gcall *stmt)
{
  return
      insnd->id == riscv_sfpu_insn_data::sfpand ||

      insnd->id == riscv_sfpu_insn_data::sfpor ||

      insnd->id == riscv_sfpu_insn_data::sfpxor ||

      (insnd->id == riscv_sfpu_insn_data::sfpxiadd_v &&
       (get_int_arg (stmt, 2) & SFPXIADD_MOD1_IS_SUB) == 0);
}


rtx riscv_sfpu_clamp_signed(rtx v, unsigned int mask)
{
  int i = INTVAL(v);
  int out = i & mask;

  if (i & (mask + 1)) {
    out |= ~mask;
  }

  return GEN_INT(out);
}

rtx riscv_sfpu_clamp_unsigned(rtx v, unsigned int mask)
{
  int i = INTVAL(v);
  int out = i & mask;

  return GEN_INT(out);
}

rtx riscv_sfpu_gen_const0_vector()
{
    int i;
    rtx vec[64];

    for (i = 0; i < 64; i++) {
      vec[i] = const_double_from_real_value(dconst0, SFmode);
    }

    return gen_rtx_CONST_VECTOR(V64SFmode, gen_rtvec_v(64, vec));
}

const char* riscv_sfpu_lv_regno_str(char *str, rtx operand)
{
  if (GET_CODE(operand) == CONST_VECTOR) {
    sprintf(str, "-");
  } else {
    sprintf(str, "lv(lr%d) ", REGNO(operand) - SFPU_REG_FIRST);
  }

  return str;
}

// If a stmt's single use args aren't tracked back to their
// defs and deleted prior to deleting the stmt, errors occur w/
// flag_checking=1
// There has to be an internal version of this...
void riscv_sfpu_prep_stmt_for_deletion(gimple *stmt)
{
  for (unsigned int i = 0; i < gimple_call_num_args (stmt); i++)
    {
      tree arg = gimple_call_arg(stmt, i);

      if (TREE_CODE(arg) == SSA_NAME && num_imm_uses (arg) == 1)
	{
	  gimple *def_g = SSA_NAME_DEF_STMT (arg);

	  if (def_g->code == GIMPLE_PHI)
	    {
	      // XXXX handle phi
	      // this seems to work fine and SSA checks are ok w/ doing nothing
	    }
	  else if (def_g->code == GIMPLE_CALL)
	    {
	      tree lhs_name = gimple_call_lhs (def_g);
	      gimple_call_set_lhs(def_g, NULL_TREE);
	      release_ssa_name(lhs_name);
	      update_stmt (def_g);
	    }
	  else if (def_g->code == GIMPLE_ASSIGN)
	    {
	      unlink_stmt_vdef(def_g);
	      gimple_stmt_iterator gsi = gsi_for_stmt(def_g);
	      gsi_remove(&gsi, true);
	      release_defs(def_g);
	    }
	}
    }
}

char const * riscv_sfpu_output_nonimm_and_nops(const char *sw, int nnops, rtx operands[])
{
  // Replay pass on wormhole assumes insns only emit 1 insn
  // Should split this out to clean it up, assert it for now
  gcc_assert(flag_wormhole == 0 || nnops == 0);
  char const *out = sw;
  while (nnops-- > 0) {
     output_asm_insn(out, operands);
     out = "SFPNOP";
  }
  return out;
}

uint32_t riscv_sfpu_fp32_to_fp16a(const uint32_t val)
{
    // https://stackoverflow.com/questions/1659440/32-bit-to-16-bit-floating-point-conversion
    // Handles denorms.  May be costly w/ non-immediate values
    const unsigned int b = val + 0x00001000;
    const unsigned int e = (b & 0x7F800000) >> 23;
    const unsigned int m = b & 0x007FFFFF;
    const unsigned int result =
       (b & 0x80000000) >> 16 |
       (e > 112) * ((((e - 112) << 10) &0x7C00) | m >> 13) |
       ((e < 113) & (e > 101)) * ((((0x007FF000 + m) >> (125 -e )) + 1) >> 1) |
       (e > 143) * 0x7FFF;
#if 0
    // Simple/faster but less complete
    const unsigned int result =
       ((val >> 16) & 0x8000) |
       ((((val & 0x7F800000) - 0x38000000) >> 13) & 0x7c00) |
       ((val >> 13) & 0x03FF);
#endif

    return result;
}

uint32_t riscv_sfpu_fp32_to_fp16b(const uint32_t val)
{
    return val >> 16;
}

uint32_t riscv_sfpu_scmp2loadi_mod(int mod)
{
  int fmt = mod & SFPXSCMP_MOD1_FMT_MASK;

  if (fmt == SFPXSCMP_MOD1_FMT_A) {
    return SFPLOADI_MOD0_FLOATA;
  }
  if (fmt == SFPXSCMP_MOD1_FMT_B) {
    return SFPLOADI_MOD0_FLOATB;
  }

  return SFPXLOADI_MOD0_FLOAT;
}

bool riscv_sfpu_get_fp16b(tree *value, gcall *stmt, const riscv_sfpu_insn_data *insnd)
{
  int mod0 = get_int_arg(stmt, insnd->mod_pos);
  bool representable = false;
  tree arg = gimple_call_arg(stmt, SFPXLOADI_IMM_POS);

  switch (mod0) {
  case SFPLOADI_MOD0_FLOATB:
    *value = arg;
    representable = true;
    break;

  case SFPLOADI_MOD0_FLOATA:
    // Corner case.  Someone requested fp16a, but the value fits in fp16b
    // XXXXX ignore for now
    break;

  case SFPXLOADI_MOD0_FLOAT:
    if (TREE_CODE(arg) == INTEGER_CST) {
      unsigned int inval = *(arg->int_cst.val);
      unsigned int man = inval & 0x007FFFFF;

      if ((man & 0xFFFF) == 0) {
       // Fits in fp16b
       representable = true;
       *value = build_int_cst(integer_type_node, riscv_sfpu_fp32_to_fp16b(inval));
      }
    }
    break;

  default:
    // Other fmts are int fmts
    break;
  }

  return representable;
}

bool riscv_sfpu_get_next_sfpu_insn(const riscv_sfpu_insn_data **insnd,
				   gcall **stmt,
				   gimple_stmt_iterator gsi,
				   bool allow_empty)
{
  gimple_stmt_iterator next_gsi = gsi;
  gsi_next_nondebug(&next_gsi);
  bool done = false;
  while (!done && !gsi_end_p(next_gsi))
    {
      // XXXXX load_immediate isn't really an sfpu insn
      // create a more generic mechanism for this
      if (riscv_sfpu_p(insnd, stmt, next_gsi) &&
	  (*insnd)->id != riscv_sfpu_insn_data::load_immediate &&
          ((*insnd)->schedule != -1 || allow_empty))
        {
          done = true;
        }
      else
        {
          gsi_next_nondebug(&next_gsi);
        }
    }

  return done;
}

void riscv_sfpu_emit_sfpassignlr(rtx dst, rtx lr)
{
  int lregnum = INTVAL(lr);
  SET_REGNO(dst, SFPU_REG_FIRST + lregnum);
  emit_insn(gen_riscv_sfpassignlr_int(dst));
}

static void
finish_new_insn(gimple_stmt_iterator *gsip, bool insert_before, gimple *new_stmt, gimple *stmt)
{
  gcc_assert(new_stmt != nullptr);
  gimple_set_location (new_stmt, gimple_location (stmt));
  gimple_set_block (new_stmt, gimple_block (stmt));
  update_stmt (new_stmt);
  if (insert_before)
    {
      gsi_insert_before(gsip, new_stmt, GSI_SAME_STMT);
    }
  else
    {
      gsi_insert_after(gsip, new_stmt, GSI_SAME_STMT);
    }
}

static tree
emit_mask(tree val, unsigned int mask, gimple_stmt_iterator *gsip, gimple *stmt)
{
  tree tmp = make_temp_ssa_name (unsigned_type_node, NULL, "mask");
  gimple *new_stmt = gimple_build_assign(tmp, BIT_AND_EXPR, val,
					 build_int_cst(unsigned_type_node, mask));
  finish_new_insn(gsip, true, new_stmt, stmt);
  return tmp;
}

static tree
emit_shift(tree val, int shft, gimple_stmt_iterator *gsip, gimple *stmt)
{
  if (shft != 0)
    {
      tree_code dir;
      if (shft < 0)
	{
	  shft = -shft;
	  dir = RSHIFT_EXPR;
	}
      else
	{
	  dir = LSHIFT_EXPR;
	}

      tree tmp = make_temp_ssa_name (unsigned_type_node, NULL, "shift");
      gimple *new_stmt = gimple_build_assign(tmp, dir, val,
					     build_int_cst(unsigned_type_node, shft));
      finish_new_insn(gsip, true, new_stmt, stmt);
      return tmp;
    }
  return val;
}

static tree
emit_load_imm(unsigned int id, gimple_stmt_iterator *gsip, gimple *stmt)
{
  const riscv_sfpu_insn_data *new_insnd =
    riscv_sfpu_get_insn_data(riscv_sfpu_insn_data::load_immediate);

  tree tmp = make_temp_ssa_name (unsigned_type_node, NULL, "li");
  gimple *new_stmt = gimple_build_call(new_insnd->decl, 1);
  gimple_call_set_arg(new_stmt, 0, build_int_cst(unsigned_type_node, id));
  gimple_call_set_lhs (new_stmt, tmp);
  finish_new_insn(gsip, true, new_stmt, stmt);

  return tmp;
}

static tree
emit_add(tree lop, tree rop, gimple_stmt_iterator *gsip, gimple *stmt)
{
  tree tmp = make_temp_ssa_name (unsigned_type_node, NULL, "sum");
  gimple *new_stmt = gimple_build_assign(tmp, PLUS_EXPR, lop, rop);
  finish_new_insn(gsip, true, new_stmt, stmt);
  return tmp;
}

tree
riscv_sfpu_emit_nonimm_prologue(unsigned int unique_id,
				const riscv_sfpu_insn_data *insnd,
				gcall *stmt,
				gimple_stmt_iterator gsi)
{
  // nonimm_pos contains the raw value
  // nonimm_pos+1 contains the shifted/masked value + load_immediate
  // nonimm_pos+2 (will) contain the unique_id
  tree immarg = gimple_call_arg(stmt, insnd->nonimm_pos);

  // Insert insns to generate:
  //   sum = unique_id + ((raw & nonimm_mask) << nonimm_shft)
  tree mask = emit_mask(immarg, insnd->nonimm_mask, &gsi, stmt);
  tree shft = emit_shift(mask, insnd->nonimm_shft, &gsi, stmt);
  tree li = emit_load_imm(unique_id, &gsi, stmt);
  tree sum = emit_add(shft, li, &gsi, stmt);

  return sum;
}

// Determine if a prologue has been emitted for the current instruction based
// on the unique id.  If so, re-use it, if not emit and track it
void
riscv_sfpu_link_nonimm_prologue(std::vector<tree> &load_imm_map,
				unsigned int unique_id,
				tree old_add,
				const riscv_sfpu_insn_data *insnd,
				gcall *stmt)
{
  gimple *add_stmt = SSA_NAME_DEF_STMT(old_add);

  if (load_imm_map.size() <= unique_id)
   {
      load_imm_map.resize(unique_id + 1);
    }

  tree sum;
  if (load_imm_map[unique_id] == nullptr)
    {
      sum = riscv_sfpu_emit_nonimm_prologue(unique_id, insnd, stmt, gsi_for_stmt(add_stmt));
      load_imm_map[unique_id] = sum;
    }
  else
    {
      sum = load_imm_map[unique_id];
    }

  // Update insn to make insnd->nonimm_pos+1 contain the sum
  gimple_call_set_arg(stmt, insnd->nonimm_pos + 1, sum);
  // Save unique_id in insn's id field
  gimple_call_set_arg(stmt, insnd->nonimm_pos + 2,
		      build_int_cst(integer_type_node, unique_id));
  update_stmt (stmt);
}

void
riscv_sfpu_cleanup_nonimm_lis(function *fun)
{
  basic_block bb;
  gimple_stmt_iterator gsi;

  FOR_EACH_BB_FN (bb, fun)
    {
      gsi = gsi_start_bb (bb);
      while (!gsi_end_p (gsi))
	{
	  gcall *stmt;
	  const riscv_sfpu_insn_data *insnd;
	  bool remove = false;

	  if (riscv_sfpu_p(&insnd, &stmt, gsi) &&
	      insnd->id == riscv_sfpu_insn_data::load_immediate)
	    {
	      tree lhs = gimple_call_lhs(stmt);
	      gimple *use_stmt;
	      imm_use_iterator iter;
	      remove = true;
	      FOR_EACH_IMM_USE_STMT (use_stmt, iter, lhs)
		{
		  remove = false;
		  if (use_stmt->code != GIMPLE_DEBUG)
		    {
		      tree sum_lhs = gimple_assign_lhs(use_stmt);
		      if (sum_lhs != NULL_TREE && has_zero_uses(sum_lhs))
			{
			  DUMP("    ...removing sum\n");
			  gimple_stmt_iterator gsi_sum = gsi_for_stmt(use_stmt);
			  gsi_remove(&gsi_sum, true);
			  remove = true;
			  BREAK_FROM_IMM_USE_STMT(iter);
			}
		    }
		}
	      if (remove)
		{
		  DUMP("    ...removing %s\n", insnd->name);
		  unlink_stmt_vdef(stmt);
		  gsi_remove(&gsi, true);
		  release_defs(stmt);
		}
	    }
	  if (!remove)
	    {
	      gsi_next(&gsi);
	    }
	}
    }
}
