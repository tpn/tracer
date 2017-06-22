
include ksamd64.inc

SkFrame struct
    ReturnAddress   dq      ?
    HomeRcx         dq      ?
    HomeRdx         dq      ?
    HomeR8          dq      ?
    HomeR9          dq      ?
    Param5          dq      ?
    Param6          dq      ?
    Param7          dq      ?
    Param8          dq      ?
SkFrame ends

;
; TestParams1
;

        LEAF_ENTRY TestParams1, _TEXT$00

        mov     SkFrame.HomeRcx[rsp], rcx
        mov     SkFrame.HomeRdx[rsp], rdx
        mov     SkFrame.HomeR8[rsp], r8
        mov     SkFrame.HomeR9[rsp], r9

        mov     r10, SkFrame.Param5[rsp]
        mov     r11, SkFrame.Param6[rsp]

        mov     r8, SkFrame.Param7[rsp]
        mov     r9, SkFrame.Param8[rsp]

        mov rbp, 99h

        ret

        LEAF_END TestParams1, _TEXT$00

;
; TestParams2
;

Locals struct
    CalleeHomeRcx   dq      ?
    CalleeHomeRdx   dq      ?
    CalleeHomeR8    dq      ?
    CalleeHomeR9    dq      ?
    CalleeParam5    dq      ?
    CalleeParam6    dq      ?
    CalleeParam7    dq      ?
    CalleeParam8    dq      ?

    Temp1           dq      ?
    Temp2           dq      ?
    Temp3           dq      ?
    Temp4           dq      ?

    union
        FirstNvRegister dq      ?
        SavedRbp        dq      ?
    ends

    SavedRbx                dq      ?
    SavedRdi                dq      ?
    SavedRsi                dq      ?
    SavedR12                dq      ?
    SavedR13                dq      ?
    SavedR14                dq      ?
    SavedR15                dq      ?

    ;
    ; The return address of our caller is next, followed by home parameter
    ; space provided for rcx-r9.
    ;

    ReturnAddress           dq      ?
    HomeRcx                 dq      ?
    HomeRdx                 dq      ?
    HomeR8                  dq      ?
    HomeR9                  dq      ?

Locals ends

;
; Exclude the space within the locals frame that was used for storing nv regs.
;

LOCALS_SIZE  equ ((sizeof Locals) + (Locals.FirstNvRegister - (sizeof Locals)))

        NESTED_ENTRY TestParams2, _TEXT$00

        rex_push_reg rbp
        alloc_stack LOCALS_SIZE
        set_frame rbp, Locals.SavedRbp

        END_PROLOGUE

        mov rax, Locals.SavedRbp[rsp]

        mov Locals.Temp1[rsp], 10h
        mov Locals.Temp2[rsp], 20h
        mov Locals.Temp3[rsp], 30h

        lea r10, [rbp + 8]

        mov rax, SkFrame.Param5[r10]
        mov Locals.CalleeParam5[rsp], rax

        mov rax, SkFrame.Param6[r10]
        mov Locals.CalleeParam6[rsp], rax

        mov rax, SkFrame.Param7[r10]
        mov Locals.CalleeParam7[rsp], rax

        mov rax, SkFrame.Param8[r10]
        mov Locals.CalleeParam8[rsp], rax

        call TestParams1

TeP90:  add rsp, LOCALS_SIZE
        pop rbp

        ret

        NESTED_END TestParams2, _TEXT$00

        NESTED_ENTRY InjectionThunk, _TEXT$00

;
; Thunk prologue.  Push all non-volatile registers to the stack, allocate
; space for our locals structure and assign a frame pointer.
;

        rex_push_reg    rbp
        push_reg        rbx
        push_reg        rdi
        push_reg        rsi
        push_reg        r12
        push_reg        r13
        push_reg        r14
        push_reg        r15
        alloc_stack LOCALS_SIZE

        END_PROLOGUE

;
; Home our Thunk (rcx) parameter register, then save in r12.  The homing of rcx
; isn't technically necessary (as we never re-load it from rcx), but it doesn't
; hurt, and it is useful during development and debugging to help detect certain
; anomalies (like clobbering r12 accidentally, for example).
;

        mov     Locals.HomeRcx[rsp], rcx            ; Home Thunk (rcx) param.
        mov     r12, rcx                            ; Move Thunk into r12.

        mov     rax, Locals.ReturnAddress[rsp]

Inj90:

;
; Begin epilogue.  Deallocate stack space and restore non-volatile registers,
; then return.
;

        add     rsp, LOCALS_SIZE
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     rsi
        pop     rdi
        pop     rbx
        pop     rbp

        ret

        NESTED_END InjectionThunk, _TEXT$00


; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql com=:;                 :

end
