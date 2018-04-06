
include ksamd64.inc

OP_EQ   equ 0
OP_NEQ  equ 4

ZMM_ALIGN equ 64
YMM_ALIGN equ 32
XMM_ALIGN equ 16

_DATA$00 SEGMENT PAGE 'DATA'

        align   ZMM_ALIGN
QuickLazy       db      "The quick brown fox jumps over the lazy dog.  "

        align   ZMM_ALIGN
        public  Test1
Test1           db      "ABACAEEFGIHIJJJKLMNDOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!!"

        align   ZMM_ALIGN
        public  Shuffle1
Shuffle1        db  64  dup (02h, 00h, 00h, 00h, 60 dup (00h))

        align   ZMM_ALIGN
        public  AllOnes
AllOnes         dd  16  dup (1)

        align   ZMM_ALIGN
        public  AllNegativeOnes
AllNegativeOnes dd  16  dup (-1)

        align   ZMM_ALIGN
        public  AllBinsMinusOne
AllBinsMinusOne dd  16  dup (254)

        align   ZMM_ALIGN
        public  AllThirtyOne
AllThirtyOne    dd  16  dup (31)

        align   ZMM_ALIGN
Input1544    dd   5,  3, 3,  1,  8,  2, 50, 1,  0,  7,  6,  4,  9, 3, 10,  3
Permute1544  dd  -1, -1, 1, -1, -1, -1, -1, 3, -1, -1, -1, -1, -1, 2, -1, 13
Conflict1544 dd   0,  0, 2,  0,  0,  0,  0, 8,  0,  0,  0,  0,  0, 6,  0, 8198
Counts1544   dd   1,  1, 2,  1,  1,  1,  1, 2,  1,  1,  1,  1,  1, 3,  1,  4

        public  Input1544
        public  Permute1544
        public  Conflict1544
        public  Counts1544

Input1544v2  dd   5,  3, 3,  1,  8,  2, 50, 1,  0,  7,  6,  4,  9, 3, 10,  3
             dd   5,  3, 3,  1,  8,  2, 50, 1,  0,  7,  6,  4,  9, 3, 10,  3

        align   ZMM_ALIGN
Input1710    db   "ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCD"
             db   "ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCD"

_DATA$00 ends

CHARACTER_HISTOGRAM struct
    Count   dd 256 dup (?)
CHARACTER_HISTOGRAM ends
PCHARACTER_HISTOGRAM typedef ptr CHARACTER_HISTOGRAM

CHARACTER_HISTOGRAM_V4 struct
    Histogram1 CHARACTER_HISTOGRAM { }
    Histogram2 CHARACTER_HISTOGRAM { }
    Histogram3 CHARACTER_HISTOGRAM { }
    Histogram4 CHARACTER_HISTOGRAM { }
CHARACTER_HISTOGRAM_V4 ends
PCHARACTER_HISTOGRAM_V4 typedef ptr CHARACTER_HISTOGRAM_V4

Locals struct
    Temp dq ?

    ;
    ; Define non-volatile register storage.
    ;

    union
        FirstNvRegister     dq      ?
        SavedRbp            dq      ?
    ends

    SavedRbx                dq      ?
    SavedRdi                dq      ?
    SavedRsi                dq      ?
    SavedR12                dq      ?
    SavedR13                dq      ?
    SavedR14                dq      ?

    SavedXmm6               XMMWORD { }
    SavedXmm7               XMMWORD { }
    SavedXmm8               XMMWORD { }
    SavedXmm9               XMMWORD { }
    SavedXmm10              XMMWORD { }
    SavedXmm11              XMMWORD { }
    SavedXmm12              XMMWORD { }
    SavedXmm13              XMMWORD { }
    SavedXmm14              XMMWORD { }
    SavedXmm15              XMMWORD { }

    ;
    ; Stash R15 after the return address to ensure the XMM register space
    ; is aligned on a 16 byte boundary, as we use movdqa (i.e. aligned move)
    ; which will fault if we're only 8 byte aligned.
    ;

    SavedR15                dq      ?

    ReturnAddress   dq  ?
    HomeRcx         dq  ?
    HomeRdx         dq  ?
    HomeR8          dq  ?
    HomeR9          dq  ?
Locals ends

;
; Exclude the return address onward from the frame calculation size.
;

LOCALS_SIZE  equ ((sizeof Locals) + (Locals.ReturnAddress - (sizeof Locals)))
LOCALS_SIZE_TEXT textequ %(LOCALS_SIZE)
%echo Locals size: LOCALS_SIZE_TEXT


        NESTED_ENTRY ScratchAvx1, _TEXT$00

;
; Begin prologue.  Allocate stack space and save non-volatile registers.
;

        alloc_stack LOCALS_SIZE

        save_reg    rbp, Locals.SavedRbp        ; Save non-volatile rbp.
        save_reg    rbx, Locals.SavedRbx        ; Save non-volatile rbx.
        save_reg    rdi, Locals.SavedRdi        ; Save non-volatile rdi.
        save_reg    rsi, Locals.SavedRsi        ; Save non-volatile rsi.
        save_reg    r12, Locals.SavedR12        ; Save non-volatile r12.
        save_reg    r13, Locals.SavedR13        ; Save non-volatile r13.
        save_reg    r14, Locals.SavedR14        ; Save non-volatile r14.
        save_reg    r15, Locals.SavedR15        ; Save non-volatile r15.

        save_xmm128 xmm6, Locals.SavedXmm6      ; Save non-volatile xmm6.
        save_xmm128 xmm7, Locals.SavedXmm7      ; Save non-volatile xmm7.
        save_xmm128 xmm8, Locals.SavedXmm8      ; Save non-volatile xmm8.
        save_xmm128 xmm9, Locals.SavedXmm9      ; Save non-volatile xmm9.
        save_xmm128 xmm10, Locals.SavedXmm10    ; Save non-volatile xmm10.
        save_xmm128 xmm11, Locals.SavedXmm11    ; Save non-volatile xmm11.
        save_xmm128 xmm12, Locals.SavedXmm12    ; Save non-volatile xmm12.
        save_xmm128 xmm13, Locals.SavedXmm13    ; Save non-volatile xmm13.
        save_xmm128 xmm14, Locals.SavedXmm14    ; Save non-volatile xmm14.
        save_xmm128 xmm15, Locals.SavedXmm15    ; Save non-volatile xmm15.

        END_PROLOGUE

        vmovntdqa       zmm31, zmmword ptr [Test1]
        mov             rax, 1111111111111111h

        kmovq           k1, rax
        vpmovm2b        zmm1, k1

        kshiftlq        k2, k1, 1
        vpmovm2b        zmm2, k2

        kshiftlq        k3, k1, 2
        vpmovm2b        zmm3, k3

        kshiftlq        k4, k1, 3
        vpmovm2b        zmm4, k4

Scr35:  vpandd          zmm5, zmm1, zmm31
        vpandd          zmm6, zmm2, zmm31
        vpandd          zmm7, zmm3, zmm31
        vpandd          zmm8, zmm4, zmm31

        vpsrldq         zmm6, zmm6, 1
        vpsrldq         zmm7, zmm7, 2
        vpsrldq         zmm8, zmm8, 3

        vpconflictd     zmm10, zmm5
        vpconflictd     zmm11, zmm6
        vpconflictd     zmm12, zmm7
        vpconflictd     zmm13, zmm8

;
; Indicate success.
;

        mov rax, 1

;
; Restore non-volatile registers.
;

Scr99:
        mov             rbp,   Locals.SavedRbp[rsp]
        mov             rbx,   Locals.SavedRbx[rsp]
        mov             rdi,   Locals.SavedRdi[rsp]
        mov             rsi,   Locals.SavedRsi[rsp]
        mov             r12,   Locals.SavedR12[rsp]
        mov             r13,   Locals.SavedR13[rsp]
        mov             r14,   Locals.SavedR14[rsp]
        mov             r15,   Locals.SavedR15[rsp]

        movdqa          xmm6,  Locals.SavedXmm6[rsp]
        movdqa          xmm7,  Locals.SavedXmm7[rsp]
        movdqa          xmm8,  Locals.SavedXmm8[rsp]
        movdqa          xmm9,  Locals.SavedXmm9[rsp]
        movdqa          xmm10, Locals.SavedXmm10[rsp]
        movdqa          xmm11, Locals.SavedXmm11[rsp]
        movdqa          xmm12, Locals.SavedXmm12[rsp]
        movdqa          xmm13, Locals.SavedXmm13[rsp]
        movdqa          xmm14, Locals.SavedXmm14[rsp]
        movdqa          xmm15, Locals.SavedXmm15[rsp]

;
; Begin epilogue.  Deallocate stack space and return.
;

        add     rsp, LOCALS_SIZE
        ret


        NESTED_END ScratchAvx1, _TEXT$00

;
; Attempt to replicate the histogram algorithm on page 15-44 of the Intel
; Optimization Reference Manual dated December 2017.
;


        NESTED_ENTRY Histo1544, _TEXT$00

;
; Begin prologue.  Allocate stack space and save non-volatile registers.
;

        alloc_stack LOCALS_SIZE

        save_reg    rbp, Locals.SavedRbp        ; Save non-volatile rbp.
        save_reg    rbx, Locals.SavedRbx        ; Save non-volatile rbx.
        save_reg    rdi, Locals.SavedRdi        ; Save non-volatile rdi.
        save_reg    rsi, Locals.SavedRsi        ; Save non-volatile rsi.
        save_reg    r12, Locals.SavedR12        ; Save non-volatile r12.
        save_reg    r13, Locals.SavedR13        ; Save non-volatile r13.
        save_reg    r14, Locals.SavedR14        ; Save non-volatile r14.
        save_reg    r15, Locals.SavedR15        ; Save non-volatile r15.

        save_xmm128 xmm6, Locals.SavedXmm6      ; Save non-volatile xmm6.
        save_xmm128 xmm7, Locals.SavedXmm7      ; Save non-volatile xmm7.
        save_xmm128 xmm8, Locals.SavedXmm8      ; Save non-volatile xmm8.
        save_xmm128 xmm9, Locals.SavedXmm9      ; Save non-volatile xmm9.
        save_xmm128 xmm10, Locals.SavedXmm10    ; Save non-volatile xmm10.
        save_xmm128 xmm11, Locals.SavedXmm11    ; Save non-volatile xmm11.
        save_xmm128 xmm12, Locals.SavedXmm12    ; Save non-volatile xmm12.
        save_xmm128 xmm13, Locals.SavedXmm13    ; Save non-volatile xmm13.
        save_xmm128 xmm14, Locals.SavedXmm14    ; Save non-volatile xmm14.
        save_xmm128 xmm15, Locals.SavedXmm15    ; Save non-volatile xmm15.

        END_PROLOGUE

        mov     Locals.HomeRcx[rsp], rcx                ; Home rcx.
        mov     Locals.HomeRdx[rsp], rdx                ; Home rdx.
        mov     Locals.HomeR8[rsp], r8                  ; Home r8.
        mov     Locals.HomeR9[rsp], r9                  ; Home r9.

        vmovaps zmm4, zmmword ptr [AllOnes]         ;all_1 ;/ {1, 1, …, 1}
        vmovaps zmm5, zmmword ptr [AllNegativeOnes] ;all_negative_1
        vmovaps zmm6, zmmword ptr [AllThirtyOne]    ;all_31
        vmovaps zmm7, zmmword ptr [AllBinsMinusOne] ;all_bins_minus_1

        vmovaps zmm10, zmmword ptr [Permute1544]
        vmovaps zmm11, zmmword ptr [Conflict1544]
        vmovaps zmm12, zmmword ptr [Counts1544]

        mov ebx, 16             ;num_inputs
        xor rcx, rcx

        lea r10, Input1544      ; Load input buffer address.
        ;mov r10, pInput

        mov r15, rdx            ; Load first histo buffer.
        ;mov r15, pHistogram

        kxnorw          k7, k0, k0

histogram_loop:
        ;vpandd zmm10, zmm7, [r10+rcx*4] ;[rcx+4*r10], zmm7              ; r10*rcx+4

        vmovntdqa       zmm3, zmmword ptr [r10]     ; Load 64 bytes into zmm0.
        vpconflictd     zmm0, zmm3
        kxnorw          k1, k1, k1

        vmovaps         zmm2, zmm4
        vmovaps         zmm8, zmm4

        vpxord          zmm1, zmm1, zmm1
        ;vpxord          zmm8, zmm8, zmm8

        vpgatherdd      zmm1 {k1}, [r15+zmm3*4]
        vptestmd        k1, zmm0, zmm0
        kortestw        k1, k1
        je              short update

        vmovaps         zmm20, zmm0
        vplzcntd        zmm0, zmm0
        vmovaps         zmm21, zmm0
        vpsubd          zmm0, zmm6, zmm0
        ;jmp             conflict_loop

conflict_loop:
        ;original = ;vpermd          zmm8 {k1} {z}, zmm2, zmm0
        ;mine v1  = ;vpermd          zmm8 {k1} {z}, zmm2, zmm2
        ;vpermd          zmm8 {k1} {z}, zmm8, zmm4
        vpermd           zmm8 {k1} {z}, zmm0, zmm2
        ;vpermd          zmm8, zmm2, zmm0
        ;vpermd          zmm8 {k1} {z}, zmm2, zmm8
        ;vpermd          zmm25 {k1},    zmm0, zmm0
        ;vpermd          zmm26,         zmm0, zmm0
        ;vpermd          zmm27 {k7},    zmm0, zmm0
        ;vptestmd        k2, zmm5, zmm0
        vpermd          zmm0 {k1},     zmm0, zmm0
        ;vptestmd        k3 {k1}, zmm5, zmm0
        ;vpsubd          zmm0 {k1},     zmm0, zmm4
        vpaddd          zmm2 {k1},     zmm2, zmm8
        vpcmpd          k1, zmm5, zmm0, OP_NEQ ; vpcmpd k1, 4, zmm5, zmm0
        kortestw        k1, k1
        jne             short conflict_loop
        jmp             update

conflict_loop_2:
        ;original = ;vpermd          zmm8 {k1} {z}, zmm2, zmm0
        ;mine v1  = ;vpermd          zmm8 {k1} {z}, zmm2, zmm2
        vpermd          zmm8 {k1} {z}, zmm2, zmm4
        ;vpermd          zmm8 {k1} {z}, zmm2, zmm8
        vpsubd          zmm0 {k1}, zmm4, zmm4
        ;vpermd          zmm0 {k1}, zmm0, zmm0
        vpaddd          zmm2 {k1}, zmm2, zmm8
        vpcmpd          k1, zmm5, zmm0, OP_NEQ ; vpcmpd k1, 4, zmm5, zmm0
        kortestw        k1, k1
        jne             short conflict_loop_2


update:
        vpaddd          zmm0, zmm2, zmm1
        kxnorw          k1, k1, k1
        add             rcx, 16
        vpscatterdd     [r15+zmm3*4] {k1}, zmm0
        cmp             ecx, ebx
        jb              histogram_loop

;
; Indicate success.
;

        mov rax, 1

;
; Restore non-volatile registers.
;

Th199:
        mov             rbp,   Locals.SavedRbp[rsp]
        mov             rbx,   Locals.SavedRbx[rsp]
        mov             rdi,   Locals.SavedRdi[rsp]
        mov             rsi,   Locals.SavedRsi[rsp]
        mov             r12,   Locals.SavedR12[rsp]
        mov             r13,   Locals.SavedR13[rsp]
        mov             r14,   Locals.SavedR14[rsp]
        mov             r15,   Locals.SavedR15[rsp]

        movdqa          xmm6,  Locals.SavedXmm6[rsp]
        movdqa          xmm7,  Locals.SavedXmm7[rsp]
        movdqa          xmm8,  Locals.SavedXmm8[rsp]
        movdqa          xmm9,  Locals.SavedXmm9[rsp]
        movdqa          xmm10, Locals.SavedXmm10[rsp]
        movdqa          xmm11, Locals.SavedXmm11[rsp]
        movdqa          xmm12, Locals.SavedXmm12[rsp]
        movdqa          xmm13, Locals.SavedXmm13[rsp]
        movdqa          xmm14, Locals.SavedXmm14[rsp]
        movdqa          xmm15, Locals.SavedXmm15[rsp]

;
; Begin epilogue.  Deallocate stack space and return.
;

        add     rsp, LOCALS_SIZE
        ret


        NESTED_END Histo1544, _TEXT$00

;
; Attempt to replicate the histogram algorithm on page 17-9 and 17-10 of the
; Intel Optimization Reference Manual dated December 2017.
;


        NESTED_ENTRY Histo1710, _TEXT$00

;
; Begin prologue.  Allocate stack space and save non-volatile registers.
;

        alloc_stack LOCALS_SIZE

        save_reg    rbp, Locals.SavedRbp        ; Save non-volatile rbp.
        save_reg    rbx, Locals.SavedRbx        ; Save non-volatile rbx.
        save_reg    rdi, Locals.SavedRdi        ; Save non-volatile rdi.
        save_reg    rsi, Locals.SavedRsi        ; Save non-volatile rsi.
        save_reg    r12, Locals.SavedR12        ; Save non-volatile r12.
        save_reg    r13, Locals.SavedR13        ; Save non-volatile r13.
        save_reg    r14, Locals.SavedR14        ; Save non-volatile r14.
        save_reg    r15, Locals.SavedR15        ; Save non-volatile r15.

        save_xmm128 xmm6, Locals.SavedXmm6      ; Save non-volatile xmm6.
        save_xmm128 xmm7, Locals.SavedXmm7      ; Save non-volatile xmm7.
        save_xmm128 xmm8, Locals.SavedXmm8      ; Save non-volatile xmm8.
        save_xmm128 xmm9, Locals.SavedXmm9      ; Save non-volatile xmm9.
        save_xmm128 xmm10, Locals.SavedXmm10    ; Save non-volatile xmm10.
        save_xmm128 xmm11, Locals.SavedXmm11    ; Save non-volatile xmm11.
        save_xmm128 xmm12, Locals.SavedXmm12    ; Save non-volatile xmm12.
        save_xmm128 xmm13, Locals.SavedXmm13    ; Save non-volatile xmm13.
        save_xmm128 xmm14, Locals.SavedXmm14    ; Save non-volatile xmm14.
        save_xmm128 xmm15, Locals.SavedXmm15    ; Save non-volatile xmm15.

        END_PROLOGUE

        mov     Locals.HomeRcx[rsp], rcx                ; Home rcx.
        mov     Locals.HomeRdx[rsp], rdx                ; Home rdx.
        mov     Locals.HomeR8[rsp], r8                  ; Home r8.
        mov     Locals.HomeR9[rsp], r9                  ; Home r9.

        vmovntdqa       zmm28, zmmword ptr [AllOnes]
        vmovntdqa       zmm29, zmmword ptr [AllNegativeOnes]
        vmovntdqa       zmm30, zmmword ptr [AllBinsMinusOne]
        vmovntdqa       zmm31, zmmword ptr [AllThirtyOne]

        vmovaps         zmm17, zmmword ptr [Permute1544]
        vmovaps         zmm18, zmmword ptr [Conflict1544]
        vmovaps         zmm19, zmmword ptr [Counts1544]

        mov     rax, rdx
        xor     rdx, rdx

        lea     r10, Input1544v2

Top:
        ;vmovups zmm4, [rsp+rdx*4+0x40]
        vmovntdqa   zmm4, zmmword ptr [r10]
        add     r10, 40h

        ;vmovups zmm4, [rsp+rdx*4+0x40]
        vpxord zmm1, zmm1, zmm1

;
; kmovw k2, k1
;
;   What's k1?!  Assume it's all 1s for now given that it's fed into vpgatherdd.
;

        kxnorw k1, k1, k1

        kmovw k2, k1

        vpconflictd zmm2, zmm4

        vpgatherdd zmm1{k2}, [rax+zmm4*4]

;
; vptestmd k0, zmm2, [rip+0x185c]
;
;   What's [rip+0x185c]?  Guess -1 as it's being used to compare the vpconflictd
;   result, then determining if there are conflicts.
;
        ;vptestmd k0, zmm2, [rip+0x185c]
        vptestmd k0, zmm2, zmm29 ; Test against AllNegativeOnes

        kmovw ecx, k0

;
; vpaddd zmm3, zmm1, zmm0
;
;   What's zmm0?  Assume all 1s, so use zmm28.
;

        vmovaps zmm0, zmm28

        ;vpaddd zmm3, zmm1, zmm0
        vpaddd zmm3, zmm1, zmm0

        test ecx, ecx

        jz No_conflicts

;
; vmovups zmm1, [rip+0x1884]
;
;   What's [rip+0x1884]?
;
;   Try:
;       - AllThirtyOne (31).
;       ;- AllOnes (zmm28)
;

        ;vmovups zmm1, [rip+0x1884]
        vmovaps zmm1, zmm31

;
; vptestmd k0, zmm2, [rip+0x18ba]
;
;   What's [rip+0x18ba]?
;
;   Try:
;
;       - AllNegativeOnes
;

        ;vptestmd k0, zmm2, [rip+0x18ba]
        vptestmd k0, zmm2, zmm29

        vplzcntd zmm5, zmm2

        xor bl, bl

        kmovw ecx, k0

;
; XXX: why two vpsubds here?
;

        vpsubd zmm1, zmm1, zmm5
        ;vpsubd zmm1, zmm1, zmm5

Resolve_conflicts:
        vpbroadcastd zmm5, ecx
        kmovw k2, ecx
        ; The vpermd doesn't appear to have any effect.
        ;vpermd zmm3{k2}, zmm1, zmm3
        vpaddd zmm3{k2}, zmm3, zmm0
        vptestmd k0{k2}, zmm5, zmm2
        kmovw esi, k0
        and ecx, esi
        jz No_conflicts
        add bl, 1h
        cmp bl, 10h
        jb Resolve_conflicts

No_conflicts:
        kmovw k2, k1
        vpscatterdd [rax+zmm4*4]{k2}, zmm3
        add edx, 10h
        cmp edx, 20h
        jb Top


;
; Indicate success.
;

        mov rax, 1

;
; Restore non-volatile registers.
;

Th199:
        mov             rbp,   Locals.SavedRbp[rsp]
        mov             rbx,   Locals.SavedRbx[rsp]
        mov             rdi,   Locals.SavedRdi[rsp]
        mov             rsi,   Locals.SavedRsi[rsp]
        mov             r12,   Locals.SavedR12[rsp]
        mov             r13,   Locals.SavedR13[rsp]
        mov             r14,   Locals.SavedR14[rsp]
        mov             r15,   Locals.SavedR15[rsp]

        movdqa          xmm6,  Locals.SavedXmm6[rsp]
        movdqa          xmm7,  Locals.SavedXmm7[rsp]
        movdqa          xmm8,  Locals.SavedXmm8[rsp]
        movdqa          xmm9,  Locals.SavedXmm9[rsp]
        movdqa          xmm10, Locals.SavedXmm10[rsp]
        movdqa          xmm11, Locals.SavedXmm11[rsp]
        movdqa          xmm12, Locals.SavedXmm12[rsp]
        movdqa          xmm13, Locals.SavedXmm13[rsp]
        movdqa          xmm14, Locals.SavedXmm14[rsp]
        movdqa          xmm15, Locals.SavedXmm15[rsp]

;
; Begin epilogue.  Deallocate stack space and return.
;

        add     rsp, LOCALS_SIZE
        ret


        NESTED_END Histo1710, _TEXT$00




        NESTED_ENTRY Histo1544v2, _TEXT$00

;
; Begin prologue.  Allocate stack space and save non-volatile registers.
;

        alloc_stack LOCALS_SIZE

        save_reg    rbp, Locals.SavedRbp        ; Save non-volatile rbp.
        save_reg    rbx, Locals.SavedRbx        ; Save non-volatile rbx.
        save_reg    rdi, Locals.SavedRdi        ; Save non-volatile rdi.
        save_reg    rsi, Locals.SavedRsi        ; Save non-volatile rsi.
        save_reg    r12, Locals.SavedR12        ; Save non-volatile r12.
        save_reg    r13, Locals.SavedR13        ; Save non-volatile r13.
        save_reg    r14, Locals.SavedR14        ; Save non-volatile r14.
        save_reg    r15, Locals.SavedR15        ; Save non-volatile r15.

        save_xmm128 xmm6, Locals.SavedXmm6      ; Save non-volatile xmm6.
        save_xmm128 xmm7, Locals.SavedXmm7      ; Save non-volatile xmm7.
        save_xmm128 xmm8, Locals.SavedXmm8      ; Save non-volatile xmm8.
        save_xmm128 xmm9, Locals.SavedXmm9      ; Save non-volatile xmm9.
        save_xmm128 xmm10, Locals.SavedXmm10    ; Save non-volatile xmm10.
        save_xmm128 xmm11, Locals.SavedXmm11    ; Save non-volatile xmm11.
        save_xmm128 xmm12, Locals.SavedXmm12    ; Save non-volatile xmm12.
        save_xmm128 xmm13, Locals.SavedXmm13    ; Save non-volatile xmm13.
        save_xmm128 xmm14, Locals.SavedXmm14    ; Save non-volatile xmm14.
        save_xmm128 xmm15, Locals.SavedXmm15    ; Save non-volatile xmm15.

        END_PROLOGUE

        mov     Locals.HomeRcx[rsp], rcx                ; Home rcx.
        mov     Locals.HomeRdx[rsp], rdx                ; Home rdx.
        mov     Locals.HomeR8[rsp], r8                  ; Home r8.
        mov     Locals.HomeR9[rsp], r9                  ; Home r9.

        vmovaps zmm4, zmmword ptr [AllOnes]         ;all_1 ;/ {1, 1, …, 1}
        vmovaps zmm5, zmmword ptr [AllNegativeOnes] ;all_negative_1
        vmovaps zmm6, zmmword ptr [AllThirtyOne]    ;all_31
        vmovaps zmm7, zmmword ptr [AllBinsMinusOne] ;all_bins_minus_1

        vmovaps         zmm17, zmmword ptr [Permute1544]
        vmovaps         zmm18, zmmword ptr [Conflict1544]
        vmovaps         zmm19, zmmword ptr [Counts1544]

        mov ebx, 16             ;num_inputs
        xor rcx, rcx

        lea r10, Input1544      ; Load input buffer address. (mov r10, pInput)
        ;mov r10, pInput

        mov r15, rdx            ; Load first histo buffer. (mov r10, pInput)
        ;mov r15, pHistogram

histogram_loop:
        ;vpandd zmm10, zmm7, [r10+rcx*4] ;[rcx+4*r10], zmm7              ; r10*rcx+4
        vmovntdqa zmm3, zmmword ptr [r10]     ; Load 64 bytes into zmm0.
        vpconflictd zmm0, zmm3
        kxnorw k1, k1, k1
        vmovaps zmm2, zmm4
        vpxord zmm1, zmm1, zmm1
        vpgatherdd zmm1{k1}, [r15+zmm3*4]
        vptestmd k1, zmm0, zmm0

        kortestw k1, k1
        je update


        vplzcntd zmm5, zmm0
        vmovaps zmm10, zmm6
        vpsubd zmm10, zmm10, zmm5

conflict_loop:

        vpermd zmm8{k1}{z}, zmm2, zmm10
        vpermd zmm10{k1}, zmm10, zmm10
        vpaddd zmm2{k1}, zmm2, zmm8
        vpcmpd k1, zmm5, zmm10, OP_NEQ ; vpcmpd k1, 4, zmm5, zmm0
        kortestw k1, k1
        jne conflict_loop

update:
        vpaddd zmm0, zmm2, zmm1
        kxnorw k1, k1, k1
        add rcx, 16
        vpscatterdd [r15+zmm3*4]{k1}, zmm0
        cmp ebx, ecx
        jb histogram_loop

;
; Indicate success.
;

        mov rax, 1

;
; Restore non-volatile registers.
;

Th199:
        mov             rbp,   Locals.SavedRbp[rsp]
        mov             rbx,   Locals.SavedRbx[rsp]
        mov             rdi,   Locals.SavedRdi[rsp]
        mov             rsi,   Locals.SavedRsi[rsp]
        mov             r12,   Locals.SavedR12[rsp]
        mov             r13,   Locals.SavedR13[rsp]
        mov             r14,   Locals.SavedR14[rsp]
        mov             r15,   Locals.SavedR15[rsp]

        movdqa          xmm6,  Locals.SavedXmm6[rsp]
        movdqa          xmm7,  Locals.SavedXmm7[rsp]
        movdqa          xmm8,  Locals.SavedXmm8[rsp]
        movdqa          xmm9,  Locals.SavedXmm9[rsp]
        movdqa          xmm10, Locals.SavedXmm10[rsp]
        movdqa          xmm11, Locals.SavedXmm11[rsp]
        movdqa          xmm12, Locals.SavedXmm12[rsp]
        movdqa          xmm13, Locals.SavedXmm13[rsp]
        movdqa          xmm14, Locals.SavedXmm14[rsp]
        movdqa          xmm15, Locals.SavedXmm15[rsp]

;
; Begin epilogue.  Deallocate stack space and return.
;

        add     rsp, LOCALS_SIZE
        ret


        NESTED_END Histo1544v2, _TEXT$00


        NESTED_ENTRY Histo1710v2, _TEXT$00

;
; Begin prologue.  Allocate stack space and save non-volatile registers.
;

        alloc_stack LOCALS_SIZE

        save_reg    rbp, Locals.SavedRbp        ; Save non-volatile rbp.
        save_reg    rbx, Locals.SavedRbx        ; Save non-volatile rbx.
        save_reg    rdi, Locals.SavedRdi        ; Save non-volatile rdi.
        save_reg    rsi, Locals.SavedRsi        ; Save non-volatile rsi.
        save_reg    r12, Locals.SavedR12        ; Save non-volatile r12.
        save_reg    r13, Locals.SavedR13        ; Save non-volatile r13.
        save_reg    r14, Locals.SavedR14        ; Save non-volatile r14.
        save_reg    r15, Locals.SavedR15        ; Save non-volatile r15.

        save_xmm128 xmm6, Locals.SavedXmm6      ; Save non-volatile xmm6.
        save_xmm128 xmm7, Locals.SavedXmm7      ; Save non-volatile xmm7.
        save_xmm128 xmm8, Locals.SavedXmm8      ; Save non-volatile xmm8.
        save_xmm128 xmm9, Locals.SavedXmm9      ; Save non-volatile xmm9.
        save_xmm128 xmm10, Locals.SavedXmm10    ; Save non-volatile xmm10.
        save_xmm128 xmm11, Locals.SavedXmm11    ; Save non-volatile xmm11.
        save_xmm128 xmm12, Locals.SavedXmm12    ; Save non-volatile xmm12.
        save_xmm128 xmm13, Locals.SavedXmm13    ; Save non-volatile xmm13.
        save_xmm128 xmm14, Locals.SavedXmm14    ; Save non-volatile xmm14.
        save_xmm128 xmm15, Locals.SavedXmm15    ; Save non-volatile xmm15.

        END_PROLOGUE

        mov     Locals.HomeRcx[rsp], rcx                ; Home rcx.
        mov     Locals.HomeRdx[rsp], rdx                ; Home rdx.
        mov     Locals.HomeR8[rsp], r8                  ; Home r8.
        mov     Locals.HomeR9[rsp], r9                  ; Home r9.

        vmovntdqa       zmm28, zmmword ptr [AllOnes]
        vmovntdqa       zmm29, zmmword ptr [AllNegativeOnes]
        vmovntdqa       zmm30, zmmword ptr [AllBinsMinusOne]
        vmovntdqa       zmm31, zmmword ptr [AllThirtyOne]

        vmovaps         zmm17, zmmword ptr [Permute1544]
        vmovaps         zmm18, zmmword ptr [Conflict1544]
        vmovaps         zmm19, zmmword ptr [Counts1544]

        mov     rax, rdx
        xor     rdx, rdx

        lea     r10, Input1544v2
        ;lea     r10, Input1710

        vmovaps zmm0, zmm28


Top:
        vmovntdqa   zmm4, zmmword ptr [r10]
        add         r10, 40h

        vpxord      zmm1, zmm1, zmm1

        kxnorw      k2, k0, k0
        ;kmovw       k2, k1

        vpconflictd zmm2, zmm4

        vpgatherdd  zmm1 {k2}, [rax+zmm4*4]

        vptestmd    k0, zmm2, zmm29             ; Test against AllNegativeOnes

        kmovw       ecx, k0

        vpaddd      zmm3, zmm1, zmm0

        test        ecx, ecx

        jz          No_conflicts

        ;vptestmd    k0, zmm2, zmm29

        vplzcntd    zmm5, zmm2

        xor         bl, bl

        ;kmovw       ecx, k0

        vpsubd      zmm1, zmm31, zmm5

Resolve_conflicts:
        vpbroadcastd    zmm5, ecx
        kmovw           k2, ecx
        ;vpermd          zmm3 {k2}, zmm1, zmm3
        vpaddd          zmm3 {k2}, zmm3, zmm0
        vptestmd        k0 {k2},   zmm5, zmm2
        kmovw           esi, k0
        and             ecx, esi
        jz              No_conflicts

        add             bl, 1h
        cmp             bl, 10h
        jb              Resolve_conflicts

No_conflicts:
        ;kmovw           k2, k1
        ;vpscatterdd     [rax+zmm4*4]{k2}, zmm3
        kxnorw          k2, k0, k0
        vpscatterdd     [rax+zmm4*4]{k2}, zmm3
        add             edx, 10h
        cmp             edx, 20h
        jb              Top


;
; Indicate success.
;

        mov rax, 1

;
; Restore non-volatile registers.
;

Th199:
        mov             rbp,   Locals.SavedRbp[rsp]
        mov             rbx,   Locals.SavedRbx[rsp]
        mov             rdi,   Locals.SavedRdi[rsp]
        mov             rsi,   Locals.SavedRsi[rsp]
        mov             r12,   Locals.SavedR12[rsp]
        mov             r13,   Locals.SavedR13[rsp]
        mov             r14,   Locals.SavedR14[rsp]
        mov             r15,   Locals.SavedR15[rsp]

        movdqa          xmm6,  Locals.SavedXmm6[rsp]
        movdqa          xmm7,  Locals.SavedXmm7[rsp]
        movdqa          xmm8,  Locals.SavedXmm8[rsp]
        movdqa          xmm9,  Locals.SavedXmm9[rsp]
        movdqa          xmm10, Locals.SavedXmm10[rsp]
        movdqa          xmm11, Locals.SavedXmm11[rsp]
        movdqa          xmm12, Locals.SavedXmm12[rsp]
        movdqa          xmm13, Locals.SavedXmm13[rsp]
        movdqa          xmm14, Locals.SavedXmm14[rsp]
        movdqa          xmm15, Locals.SavedXmm15[rsp]

;
; Begin epilogue.  Deallocate stack space and return.
;

        add     rsp, LOCALS_SIZE
        ret


        NESTED_END Histo1710v2, _TEXT$00

        NESTED_ENTRY Histo1710v3, _TEXT$00

;
; Begin prologue.  Allocate stack space and save non-volatile registers.
;

        alloc_stack LOCALS_SIZE

        save_reg    rbp, Locals.SavedRbp        ; Save non-volatile rbp.
        save_reg    rbx, Locals.SavedRbx        ; Save non-volatile rbx.
        save_reg    rdi, Locals.SavedRdi        ; Save non-volatile rdi.
        save_reg    rsi, Locals.SavedRsi        ; Save non-volatile rsi.
        save_reg    r12, Locals.SavedR12        ; Save non-volatile r12.
        save_reg    r13, Locals.SavedR13        ; Save non-volatile r13.
        save_reg    r14, Locals.SavedR14        ; Save non-volatile r14.
        save_reg    r15, Locals.SavedR15        ; Save non-volatile r15.

        save_xmm128 xmm6, Locals.SavedXmm6      ; Save non-volatile xmm6.
        save_xmm128 xmm7, Locals.SavedXmm7      ; Save non-volatile xmm7.
        save_xmm128 xmm8, Locals.SavedXmm8      ; Save non-volatile xmm8.
        save_xmm128 xmm9, Locals.SavedXmm9      ; Save non-volatile xmm9.
        save_xmm128 xmm10, Locals.SavedXmm10    ; Save non-volatile xmm10.
        save_xmm128 xmm11, Locals.SavedXmm11    ; Save non-volatile xmm11.
        save_xmm128 xmm12, Locals.SavedXmm12    ; Save non-volatile xmm12.
        save_xmm128 xmm13, Locals.SavedXmm13    ; Save non-volatile xmm13.
        save_xmm128 xmm14, Locals.SavedXmm14    ; Save non-volatile xmm14.
        save_xmm128 xmm15, Locals.SavedXmm15    ; Save non-volatile xmm15.

        END_PROLOGUE

        mov     Locals.HomeRcx[rsp], rcx                ; Home rcx.
        mov     Locals.HomeRdx[rsp], rdx                ; Home rdx.
        mov     Locals.HomeR8[rsp], r8                  ; Home r8.
        mov     Locals.HomeR9[rsp], r9                  ; Home r9.

        vmovntdqa       zmm28, zmmword ptr [AllOnes]
        vmovntdqa       zmm29, zmmword ptr [AllNegativeOnes]
        vmovntdqa       zmm30, zmmword ptr [AllBinsMinusOne]
        vmovntdqa       zmm31, zmmword ptr [AllThirtyOne]

        vmovaps         zmm17, zmmword ptr [Permute1544]
        vmovaps         zmm18, zmmword ptr [Conflict1544]
        vmovaps         zmm19, zmmword ptr [Counts1544]

        mov             rax, 1111111111111111h
        kmovq           k1, rax
        vpmovm2b        zmm1, k1

        mov     rax, rdx
        xor     rdx, rdx

        ;lea     r10, Input1544v2
        lea     r10, Input1710

        vmovaps zmm0, zmm28

Top:
        vmovntdqa   zmm4, zmmword ptr [r10]
        add         r10, 40h

        vpandd      zmm4, zmm1, zmm4

        vpxord      zmm1, zmm1, zmm1

        kxnorw      k2, k0, k0
        ;kmovw       k2, k1

        vpconflictd zmm2, zmm4

        vpgatherdd  zmm1 {k2}, [rax+zmm4*4]

        vptestmd    k0, zmm2, zmm29             ; Test against AllNegativeOnes

        kmovw       ecx, k0

        vpaddd      zmm3, zmm1, zmm0

        test        ecx, ecx

        jz          No_conflicts

        ;vptestmd    k0, zmm2, zmm29

        vplzcntd    zmm5, zmm2

        xor         bl, bl

        ;kmovw       ecx, k0

        vpsubd      zmm1, zmm31, zmm5

        jmp         Resolve_conflicts2

Resolve_conflicts:
        vpbroadcastd    zmm5, ecx
        kmovw           k2, ecx
        ;vpermd          zmm3 {k2}, zmm1, zmm3
        vpaddd          zmm3 {k2}, zmm3, zmm0
        vptestmd        k0 {k2},   zmm5, zmm2
        kmovw           esi, k0
        and             ecx, esi
        jz              No_conflicts

        add             bl, 1h
        cmp             bl, 10h
        jb              Resolve_conflicts

Resolve_conflicts2:
        vpbroadcastd    zmm5, ecx
        kmovw           k2, ecx
;       vpermd          zmm3 {k2}, zmm1, zmm3
        vpaddd          zmm3 {k2}, zmm3, zmm0
        vptestmd        k0 {k2},   zmm5, zmm2
        kmovw           esi, k0
        and             ecx, esi
        jnz             Resolve_conflicts2

No_conflicts:
        ;kmovw           k2, k1
        ;vpscatterdd     [rax+zmm4*4]{k2}, zmm3
        kxnorw          k2, k0, k0
        vpscatterdd     [rax+zmm4*4]{k2}, zmm3
        add             edx, 10h
        cmp             edx, 20h
        jb              Top


;
; Indicate success.
;

        mov rax, 1

;
; Restore non-volatile registers.
;

Th199:
        mov             rbp,   Locals.SavedRbp[rsp]
        mov             rbx,   Locals.SavedRbx[rsp]
        mov             rdi,   Locals.SavedRdi[rsp]
        mov             rsi,   Locals.SavedRsi[rsp]
        mov             r12,   Locals.SavedR12[rsp]
        mov             r13,   Locals.SavedR13[rsp]
        mov             r14,   Locals.SavedR14[rsp]
        mov             r15,   Locals.SavedR15[rsp]

        movdqa          xmm6,  Locals.SavedXmm6[rsp]
        movdqa          xmm7,  Locals.SavedXmm7[rsp]
        movdqa          xmm8,  Locals.SavedXmm8[rsp]
        movdqa          xmm9,  Locals.SavedXmm9[rsp]
        movdqa          xmm10, Locals.SavedXmm10[rsp]
        movdqa          xmm11, Locals.SavedXmm11[rsp]
        movdqa          xmm12, Locals.SavedXmm12[rsp]
        movdqa          xmm13, Locals.SavedXmm13[rsp]
        movdqa          xmm14, Locals.SavedXmm14[rsp]
        movdqa          xmm15, Locals.SavedXmm15[rsp]

;
; Begin epilogue.  Deallocate stack space and return.
;

        add     rsp, LOCALS_SIZE
        ret


        NESTED_END Histo1710v3, _TEXT$00


; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=\:;           :

end
