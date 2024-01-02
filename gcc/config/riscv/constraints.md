;; Constraint definitions for RISC-V target.
;; Copyright (C) 2011-2020 Free Software Foundation, Inc.
;; Contributed by Andrew Waterman (andrew@sifive.com).
;; Based on MIPS target for GNU compiler.
;;
;; This file is part of GCC.
;;
;; GCC is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.
;;
;; GCC is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.

;; Register constraints

(define_register_constraint "f" "TARGET_HARD_FLOAT ? FP_REGS : NO_REGS"
  "A floating-point register (if available).")

(define_register_constraint "j" "SIBCALL_REGS"
  "@internal")

(define_register_constraint "x" "SFPU_REGS"
  "Any of the SFPU Registers L0 - L3 (L0 - L7 for Wormhole)")

(define_register_constraint "Q0" "SFPU_REGS_L0"
  "SFPU Register L0")

(define_register_constraint "Q1" "SFPU_REGS_L1"
  "SFPU Register L1")

(define_register_constraint "Q2" "SFPU_REGS_L2"
  "SFPU Register L2")

(define_register_constraint "Q3" "SFPU_REGS_L3"
  "SFPU Register L3")

(define_register_constraint "Q4" "SFPU_REGS_L4"
  "SFPU Register L4")

(define_register_constraint "Q5" "SFPU_REGS_L5"
  "SFPU Register L5")

(define_register_constraint "Q6" "SFPU_REGS_L6"
  "SFPU Register L6")

(define_register_constraint "Q7" "SFPU_REGS_L7"
  "SFPU Register L7")

;; Avoid using register t0 for JALR's argument, because for some
;; microarchitectures that is a return-address stack hint.
(define_register_constraint "l" "JALR_REGS"
  "@internal")

;; General constraints

(define_constraint "I"
  "An I-type 12-bit signed immediate."
  (and (match_code "const_int")
       (match_test "SMALL_OPERAND (ival)")))

(define_constraint "J"
  "Integer zero."
  (and (match_code "const_int")
       (match_test "ival == 0")))

(define_constraint "K"
  "A 5-bit unsigned immediate for CSR access instructions."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 31)")))

(define_constraint "L"
  "A U-type 20-bit signed immediate."
  (and (match_code "const_int")
       (match_test "LUI_OPERAND (ival)")))

(define_constraint "M01U"
  "A 1-bit unsigned immediate for SFPU instruction modifiers."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 1)")))

(define_constraint "M02U"
  "A 2-bit unsigned immediate for SFPU instruction modifiers."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 3)")))

(define_constraint "M03U"
  "A 3-bit unsigned immediate for SFPU instruction modifiers."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 7)")))

(define_constraint "M04U"
  "A 4-bit unsigned immediate for SFPU instruction modifiers."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 15)")))

(define_constraint "M05U"
  "A 5-bit unsigned immediate for SFPU instruction modifiers."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 31)")))

(define_constraint "MP5U"
  "A 5-bit unsigned immediate for SFPU instruction modifiers."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 1, 32)")))

(define_constraint "M12S"
  "A 12-bit signed immediate for SFPU."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, -2048, 2047)")))

(define_constraint "M12U"
  "A 12-bit unsigned immediate for SFPU."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 4095)")))

(define_constraint "M13U"
  "A 13-bit unsigned immediate for SFPU."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 8191)")))

(define_constraint "M14U"
  "A 14-bit unsigned immediate for SFPU."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 16383)")))

(define_constraint "M16U"
  "A 16-bit unsigned immediate for SFPU load/store instruction offsets."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, 0, 65535)")))

(define_constraint "M16S"
  "A 16-bit signed immediate for SFPU load/store instruction offsets."
  (and (match_code "const_int")
       (match_test "IN_RANGE (ival, -32768, 32767)")))

;; Floating-point constant +0.0, used for FCVT-based moves when FMV is
;; not available in RV32.
(define_constraint "G"
  "@internal"
  (and (match_code "const_double")
       (match_test "op == CONST0_RTX (mode)")))

(define_memory_constraint "A"
  "An address that is held in a general-purpose register."
  (and (match_code "mem")
       (match_test "GET_CODE(XEXP(op,0)) == REG")))

(define_constraint "S"
  "@internal
   A constant call address."
  (match_operand 0 "absolute_symbolic_operand"))

(define_constraint "U"
  "@internal
   A PLT-indirect call address."
  (match_operand 0 "plt_symbolic_operand"))

(define_constraint "T"
  "@internal
   A constant @code{move_operand}."
  (and (match_operand 0 "move_operand")
       (match_test "CONSTANT_P (op)")))
