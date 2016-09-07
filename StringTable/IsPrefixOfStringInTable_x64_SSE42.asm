        title "IsPrefixOfStringInTable_x64_SSE42"
        option casemap:none
;++
;
; Copyright (c) Trent Nelson, 2016.
;
; Module Name:
;
;   IsPrefixOfStringInTable_x64_SSE42.asm
;
; Abstract:
;
;   This module implements an IsPrefixOfStringInTable function using SSE 4.2
;   SIMD primitives.
;
;--


include StringTable.inc


Locals struct
    ;
    ; Callee register home space.
    ;

    CalleeHomeRcx   dq      ?       ; 8     24      32      (20h)
    CalleeHomeRdx   dq      ?       ; 8     16      24      (18h)
    CalleeHomeR8    dq      ?       ; 8     8       16      (10h)
    CalleeHomeR9    dq      ?       ; 8     0       8       (08h)

    ReturnValue dq  ?
    TscAux TSC_AUX { }

    Timestamp TSC { }

    ; 7     (+ 4)

    ProcessId dw ?
    ThreadId dw ?

    ; 8     (+ 1)

    TraceFrameRecord        dq ?
    union
        HookedFunction  FunctionPointer ?
        HookedFuncPtr   Function ptr ?
    ends

    Rflags  dq ?

Locals ends


;++
;
; BOOL
; IsPrefixOfStringInTable_x64_SSE42(
;     _In_ PSTRING_TABLE StringTable,
;     _In_ PSTRING String,
;     _Out_ PSTRING_MATCH StringMatch
;     )
;
; Routine Description:
;
;     This routine searches for a prefix match of String in the given
;     StringTable structure.
;
; Arguments:
;
;     StringTable - Supplies a pointer to a STRING_TABLE structure to search.
;
;     String - Supplies a pointer to a STRING structure that a prefix match
;         is searched for.
;
;     StringMatch - Supplies a pointer to a STRING_MATCH structure that will
;         receive the results of the string match.
;
; Return Value:
;
;    Returns TRUE on sucess, FALSE on failure.
;
;--
        NESTED_ENTRY IsPrefixOfStringInTable_x64_SSE42, _TEXT$00

        rex_push_reg rbp
        set_frame rbp, 8

        rex_push_reg rsi
        rex_push_reg rdi
        rex_push_reg rbx
        rex_push_reg r13

        alloc_stack sizeof(Locals)

        END_PROLOGUE

;
; Home our three parameter registers.
;
        save_reg rcx, Params.HomeRcx[rsp]   ; home rcx (param 1)
        save_reg rdx, Params.HomeRdx[rsp]   ; home rdx (param 2)
        save_reg r8,  Params.HomeR8x[rsp]   ; home r8  (param 3)

;
; If the search string is longer than the longest string in the table, there
; can't be any prefix matches.
;


        popcnt StringTable.ContinuationBitmap[rcx]
        lea r11, (String


FxEnd:  BEGIN_EPILOGUE

        add rsp, sizeof(Locals)

        pop r13
        pop rbx
        pop rdi
        pop rsi
        pop rbp

        ret

        NESTED_END IsPrefixOfStringInTable_x64_SSE42, _TEXT$00


; vim:set tw=80 ts=8 sw=4 sts=4 expandtab syntax=masm formatoptions=croql      :

end
