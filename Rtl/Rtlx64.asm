title "Runtime Library AMD64 Assembly Support Routines"

include ksamd64.inc

LEAF_ENTRY ZeroPages, _TEXT$00

        xor     eax, eax                ; clear register
        shr     rdx, 7                  ; number of 128 byte chunks (loop count)

        align   16


KiZS10: movnti  [rcx], rax              ; zero 128-byte block
        movnti  [rcx +  8], rax         ;
        movnti  [rcx + 16], rax         ;
        movnti  [rcx + 24], rax         ;
        movnti  [rcx + 32], rax         ;
        movnti  [rcx + 40], rax         ;
        movnti  [rcx + 48], rax         ;
        movnti  [rcx + 56], rax         ;
        add     rcx, 128                ; advance to next block
        movnti  [rcx - 64], rax         ;
        movnti  [rcx - 56], rax         ;
        movnti  [rcx - 48], rax         ;
        movnti  [rcx - 40], rax         ;
        movnti  [rcx - 32], rax         ;
        movnti  [rcx - 24], rax         ;
        movnti  [rcx - 16], rax         ;
        movnti  [rcx -  8], rax         ;
        dec     rdx                     ; decrement loop count
        jnz     short KiZS10            ; if nz, more bytes to zero
   lock or      byte ptr [rsp], 0       ; flush data to memory
        ret                             ; return

LEAF_END ZeroPages, _TEXT$00

.code

	;extern	?ProfilerEnter@@YAX_JPEAX@Z:near
	;extern	?ProfilerExit@@YAX_JPEAX@Z:near

; See https://software.intel.com/en-us/articles/introduction-to-x64-assembly for a good introduction to x64 architecture and calling conventions.

	Profiler_enter PROC

		PUSH	RAX		; save RAX before clobbering it with the flags
		LAHF			; get the flags
		PUSH	RAX		; save the flags

		; we save 14 64-bit registers (14 * 8 bytes) + 6 128-bit registers (6 * 16 bytes) and we offset 24 bytes for the overhead of the ProfileEnter function (return address + RCX + RDX)
		; (14 * 8) + (6 * 16) + 24 = 232 bytes
		SUB		RSP, 232

		; "push" the volatile registers onto the stack (see http://msdn.microsoft.com/en-us/library/ms235286.aspx)
		; RBX, RCX, RDX, RBP, RDI, RSI, R8, R9, R10, R11, R12, R13, R14, R15, XXM0 (16), XXM1 (16), XXM2 (16), XXM3 (16), XXM4 (16), XXM5 (16)

		MOV		[RSP+24], RBX
		MOV		[RSP+32], RCX
		MOV		[RSP+40], RDX
		MOV		[RSP+48], RBP
		MOV		[RSP+56], RDI
		MOV		[RSP+64], RSI
		MOV		[RSP+72], R8
		MOV		[RSP+80], R9
		MOV		[RSP+88], R10
		MOV		[RSP+96], R11
		MOV		[RSP+104], R12
		MOV		[RSP+112], R13
		MOV		[RSP+120], R14
		MOV		[RSP+128], R15

		MOVDQU	OWORD PTR [RSP+136], XMM0  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+152], XMM1  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+168], XMM2  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+184], XMM3  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+200], XMM4  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+216], XMM5  ; use unaligned move (slower but easier)

		XOR		EAX, EAX
		XOR		ECX, ECX
		CPUID  ; slower but more accurate across multiple threads running on different cores
		RDTSC
		SHL		RDX, 20h
		OR		RDX, RAX
		MOV		RCX, RDX  ; store the counter (in first argument)

		MOV		RDX, QWORD PTR [RSP + 248]  ; get the return address off the stack (offset = 232 + (2 * 8) bytes that we pushed) in the second argument

		;CALL	?ProfilerEnter@@YAX_JPEAX@Z

		; "pop" the registers back off the stack
		MOVDQU	XMM5, OWORD PTR [RSP+216]  ; use unaligned move (slower but easier)
		MOVDQU	XMM4, OWORD PTR [RSP+200]  ; use unaligned move (slower but easier)
		MOVDQU	XMM3, OWORD PTR [RSP+184]  ; use unaligned move (slower but easier)
		MOVDQU	XMM2, OWORD PTR [RSP+168]  ; use unaligned move (slower but easier)
		MOVDQU	XMM1, OWORD PTR [RSP+152]  ; use unaligned move (slower but easier)
		MOVDQU	XMM0, OWORD PTR [RSP+136]  ; use unaligned move (slower but easier)

		MOV		R15, [RSP+128]
		MOV		R14, [RSP+120]
		MOV		R13, [RSP+112]
		MOV		R12, [RSP+104]
		MOV		R11, [RSP+96]
		MOV		R10, [RSP+88]
		MOV		R9, [RSP+80]
		MOV		R8, [RSP+72]
		MOV		RSI, [RSP+64]
		MOV		RDI, [RSP+56]
		MOV		RBP, [RSP+48]
		MOV		RDX, [RSP+40]
		MOV		RCX, [RSP+32]
		MOV		RBX, [RSP+24]

		ADD		RSP, 232

		POP		RAX		; pop the flags off the stack
		SAHF			; restore the flags
		POP		RAX		; pop original RAX

		RET
	Profiler_enter ENDP

	Profiler_exit PROC

		PUSH	RAX		; save RAX before clobbering it with the flags
		LAHF			; get the flags
		PUSH	RAX		; save the flags

		; we save 14 64-bit registers (14 * 8 bytes) + 6 128-bit registers (6 * 16 bytes) and we offset 24 bytes for the overhead of the ProfileEnter function (return address + RCX + RDX)
		; (14 * 8) + (6 * 16) + 24 = 232 bytes
		SUB		RSP, 232

		; "push" the volatile registers onto the stack (see http://msdn.microsoft.com/en-us/library/ms235286.aspx)
		; RBX, RCX, RDX, RBP, RDI, RSI, R8, R9, R10, R11, R12, R13, R14, R15, XXM0 (16), XXM1 (16), XXM2 (16), XXM3 (16), XXM4 (16), XXM5 (16)

		MOV		[RSP+24], RBX
		MOV		[RSP+32], RCX
		MOV		[RSP+40], RDX
		MOV		[RSP+48], RBP
		MOV		[RSP+56], RDI
		MOV		[RSP+64], RSI
		MOV		[RSP+72], R8
		MOV		[RSP+80], R9
		MOV		[RSP+88], R10
		MOV		[RSP+96], R11
		MOV		[RSP+104], R12
		MOV		[RSP+112], R13
		MOV		[RSP+120], R14
		MOV		[RSP+128], R15

		MOVDQU	OWORD PTR [RSP+136], XMM0  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+152], XMM1  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+168], XMM2  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+184], XMM3  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+200], XMM4  ; use unaligned move (slower but easier)
		MOVDQU	OWORD PTR [RSP+216], XMM5  ; use unaligned move (slower but easier)

		XOR		EAX, EAX
		XOR		ECX, ECX
		CPUID  ; slower but more accurate across multiple threads running on different cores
		RDTSC
		SHL		RDX, 20h
		OR		RDX, RAX
		MOV		RCX, RDX  ; store the counter (in first argument)

		MOV		RDX, QWORD PTR [RSP + 248]  ; get the return address off the stack (offset = 232 + (2 * 8) bytes that we pushed) in the second argument

		;CALL	?ProfilerExit@@YAX_JPEAX@Z

		; "pop" the registers back off the stack
		MOVDQU	XMM5, OWORD PTR [RSP+216]  ; use unaligned move (slower but easier)
		MOVDQU	XMM4, OWORD PTR [RSP+200]  ; use unaligned move (slower but easier)
		MOVDQU	XMM3, OWORD PTR [RSP+184]  ; use unaligned move (slower but easier)
		MOVDQU	XMM2, OWORD PTR [RSP+168]  ; use unaligned move (slower but easier)
		MOVDQU	XMM1, OWORD PTR [RSP+152]  ; use unaligned move (slower but easier)
		MOVDQU	XMM0, OWORD PTR [RSP+136]  ; use unaligned move (slower but easier)

		MOV		R15, [RSP+128]
		MOV		R14, [RSP+120]
		MOV		R13, [RSP+112]
		MOV		R12, [RSP+104]
		MOV		R11, [RSP+96]
		MOV		R10, [RSP+88]
		MOV		R9, [RSP+80]
		MOV		R8, [RSP+72]
		MOV		RSI, [RSP+64]
		MOV		RDI, [RSP+56]
		MOV		RBP, [RSP+48]
		MOV		RDX, [RSP+40]
		MOV		RCX, [RSP+32]
		MOV		RBX, [RSP+24]

		ADD		RSP, 232

		POP		RAX		; pop the flags off the stack
		SAHF			; restore the flags
		POP		RAX		; pop original RAX

		RET
	Profiler_exit ENDP

sample1 PROC FRAME
    db      048h; emit a REX prefix, to enable hot-patching

    push rbp
    .pushreg rbp
    sub rsp, 040h
    .allocstack 040h
    lea rbp, [rsp+020h]
    .setframe rbp, 020h
    movdqa [rbp], xmm7
    .savexmm128 xmm7, 020h;the offset is from the base of the frame
    ;not the scaled offset of the frame
    mov [rbp+018h], rsi
    .savereg rsi, 038h
    mov [rsp+010h], rdi
    .savereg rdi, 010h; you can still use RSP as the base of the frame
    ; or any other register you choose
    .endprolog

    ; you can modify the stack pointer outside of the prologue (similar to alloca)
    ; because we have a frame pointer.
    ; if we didn?t have a frame pointer, this would be illegal
    ; if we didn?t make this modification,
    ; there would be no need for a frame pointer

    sub rsp, 060h

    ; we can unwind from the following AV because of the frame pointer

    mov rax, 0
    mov rax, [rax] ; AV!

    ; restore the registers that weren?t saved with a push
    ; this isn?t part of the official epilog, as described in section 2.5

    movdqa xmm7, [rbp]
    mov rsi, [rbp+018h]
    mov rdi, [rbp-010h]

    ; Here?s the official epilog

    lea rsp, [rbp-020h]
    pop rbp
    ret
    sample1 ENDP

; https://msdn.microsoft.com/en-us/library/ms235217.aspx

sampleFrame struct
    Fill dq ? ; fill to 8 mod 16
    SavedRdi dq ? ; Saved Register RDI
    SavedRsi dq ? ; Saved Register RSI
sampleFrame ends


sample2 PROC FRAME
    alloc_stack SIZEOF sampleFrame
    
    ;sub rsp, 24
    ;.allocstack 24
    
    save_reg rdi, sampleFrame.SavedRdi
    save_reg rsi, sampleFrame.SavedRsi
    
    END_PROLOGUE
    ;.endprolog

    ; function body

    mov rsi, sampleFrame.SavedRsi[rsp]
    mov rdi, sampleFrame.SavedRdi[rsp]

    ; Here?s the official epilog

    BEGIN_EPILOGUE

    add rsp, (sizeof sampleFrame)
    ret
sample2 ENDP

text SEGMENT
PUBLIC Example3
PUBLIC Example3_UW
Example3_UW PROC
   ; exception/unwind handler body

   ret 0

Example3_UW ENDP

Example3 PROC FRAME : Example3_UW

   sub rsp, 16
.allocstack 16

.endprolog

   ; function body
    add rsp, 16
   ret 0

Example3 ENDP

sample PROC FRAME
    .allocstack 8			; smallest value
    .setframe rbp, 0		; smallest value
    .savexmm128 xmm7, 16*64*1024-16	; last smaller-sized
    .savereg rsi, 8*64*1024-8	; last smaller-sized
    .endprolog
sample ENDP



text ENDS

end

; vim:set tw=80 ts=8 sw=8 sts=8 expandtab:
