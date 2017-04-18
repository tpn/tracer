;++
;
; Copyright (c) 2017 Trent Nelson <trent@trent.me>
;
; Module Name:
;
;   Asm.inc
;
; Abstract:
;
;   This is the main include file for all assembly language modules.  It
;   contains includes, constant definitions and macros that are used in
;   subroutines.
;
;--

;
; Include the main Windows DDK MASM header.
;

include ksamd64.inc

;
; Define macros for inserting start and end markers that can be detected by
; Intel's code analysis tool IACA.
;

IACA_VC_START macro Name

        mov     byte ptr gs:[06fh], 06fh

        endm

IACA_VC_END macro Name

        mov     byte ptr gs:[0deh], 0deh

        endm

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=:;            :

