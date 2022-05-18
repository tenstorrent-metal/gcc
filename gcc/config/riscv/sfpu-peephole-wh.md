;; Machine description for Tenstorrent SFPU Intrinsics.
;; Copyright (C) 2011-2020 Free Software Foundation, Inc.
;; Contributed by Ayonam Ray (ayonam@helprack.com)
;; Based on RISC-V target for GNU compiler.

;; This file is part of GCC.

;; GCC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; GCC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.


;; LZ
(define_peephole2
  [(set (match_operand:V64SF 0 "register_operand")
        (unspec_volatile [(match_operand:V64SF 1 "nonmemory_operand")
                          (match_operand:V64SF 2 "register_operand")
                          (match_operand:SI    3 "const_0_operand")] UNSPECV_WH_SFPLZ_INT))
   (unspec_volatile [(match_dup:V64SF     2)
                     (match_operand:SI    4 "const_setcc_z_or_nez")] UNSPECV_WH_SFPSETCC_V)]

  "TARGET_SFPU_WH"
  [(const_int 0)]
{
  int mod1b = INTVAL(operands[4]);
  // Only legal values of SETCC are 2 or 6 which map to 2 and 10
  rtx mod = GEN_INT((mod1b == 2) ? 2 : 10);

  emit_insn(gen_riscv_wh_sfplz_int(operands[0], operands[1], operands[2], mod));
})

(define_peephole2
  [(set (match_operand:V64SF 0 "register_operand")
        (unspec_volatile [(match_operand:V64SF 1 "nonmemory_operand")
                          (match_operand:V64SF 2 "register_operand")
                          (match_operand:SI    3 "const_0_operand")] UNSPECV_WH_SFPLZ_INT))
   (unspec_volatile [(match_operand:SI    4 "immediate_operand")] UNSPECV_WH_SFPPUSHC)
   (unspec_volatile [(match_dup:V64SF     2)
                     (match_operand:SI    5 "const_setcc_z_or_nez")] UNSPECV_WH_SFPSETCC_V)]

  "TARGET_SFPU_WH"
  [(const_int 0)]
{
  int mod1b = INTVAL(operands[5]);
  // Only legal values of SETCC are 2 or 6 which map to 2 and 10
  rtx mod = GEN_INT((mod1b == 2) ? 2 : 10);

  emit_insn(gen_riscv_wh_sfppushc(operands[4]));
  emit_insn(gen_riscv_wh_sfplz_int(operands[0], operands[1], operands[2], mod));
})
