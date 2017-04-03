        title "Miscellaneous Assembly Routine"

;++
;
; Copyright (c) 2017 Trent Nelson <trent@trent.me>
;
; Module Name:
;
;   Misc.asm
;
; Abstract:
;
;   This module implements miscellaneous AMD64 assembly routines.
;
;--

include ksamd64.inc

;++
;
; ULONG_PTR
; GetInstructionPointerAddress(
;     VOID
;     );
;
; Routine Description:
;
;   This routine returns the address of the instruction pointer prior to
;   entering this call.
;
; Arguments:
;
;   None.
;
; Return Value:
;
;   The address of the caller's instruction pointer.
;
;--

        LEAF_ENTRY GetInstructionPointerAddress, _TEXT$00
        ALTERNATE_ENTRY GetInstructionPointerAddressRaw

        mov     rax, qword ptr [rsp + 40h]      ; Copy the IP address.
        ret                                     ; Return.

        LEAF_END GetInstructionPointerAddress, _TEXT$00

;++
;
; VOID
; GetFunctionBoundaries(
;     _In_ ULONG_PTR Address,
;     _Out_ PULONG_PTR StartAddress,
;     _Out_ PULONG_PTR EndAddress
;     );
;
; Routine Description:
;
;   This routine returns the start and end addresses of a function.
;
; Arguments:
;
;   Address (rcx) - Starting address to scan forward and backward from.
;
;   StartAddress (rdx) - Supplies a pointer to a variable that will receive the
;       starting address of the function.
;
;   EndAddress (r8) - Supplies a pointer to a variable that will receive the
;       ending address of the function.
;
; Return Value:
;
;   None.
;
;--

        LEAF_ENTRY GetFunctionBoundaries, _TEXT$00
        ALTERNATE_ENTRY GetFunctionBoundariesRaw

;
; Fill an XMM register with 0xCC (int 3), then broadcast the byte across all
; 16 lanes of a second XMM register.
;

        mov             rax,  0cch              ; Load 0xCC into RAX.
        movd            xmm1, rax               ; Load 0xCC into XMM1.
        ;pbroadcastb     xmm0, xmm0, xmm1        ; Broadcast across XMM0.

;
; Clear r10 and r11.
;

        xor     r10, r10
        xor     r11, r11

;
; Stash the original address into r9, then align rcx down to a 32 byte boundary.
; This allows us to use aligned loads.
;

        mov     r9, rcx
        and     rcx, 31

Gfb05:  xor     rax, rax
        sub     rax, 32

        align 16

;
; Search backwards 32 bytes at a time, via two 16 byte registers.
;

Gfb10:  movntdqa    xmm1, xmmword ptr [rcx + rax + 16]  ; Load -32-16 bytes.
        ;pcmpeqb     xmm2, xmm1, xmm0                    ; Check for 0xCC.
        pmovmskb    r10, xmm2                           ; Create mask.

        movntdqa    xmm3, xmmword ptr [rcx + rax]       ; Load -16-0 bytes.
        ;pcmpeqb     xmm4, xmm2, xmm0                    ; Check for 0xCC.
        pmovmskb    r11, xmm4                           ; Create mask.

;
; Fast-path comparison of the two masks.
;
        sub     rax, 32                         ; Subtract 32 from rax.
        test    r10, r11                        ; Any 0xCCs?
        jz      short Gfb10                     ; No, keep searching.

;
; There's at least one match.
;

;
; XXX TODO: work in progress.
;

        ;lea r8, [rcx - r10]

        ret                                     ; Return.

        LEAF_END GetFunctionBoundaries, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql com=:;                 :

end
