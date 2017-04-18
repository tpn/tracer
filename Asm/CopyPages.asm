        title "Copy Pages Non-temporal AVX2 Assembly Routine"

;++
;
; Copyright (c) 2017 Trent Nelson <trent@trent.me>
;
; Module Name:
;
;   CopyPages.asm
;
; Abstract:
;
;   This module implements various routines for copying pages of memory.
;
;--

include ksamd64.inc

;++
;
; VOID
; CopyPagesNonTemporalAvx2_v1(
;     _In_ PCHAR Destination,
;     _In_ PCHAR Source,
;     _In_ ULONG NumberOfPages
;     );
;
; Routine Description:
;
;   This routine copies one or more pages of memory using AVX2 instructions and
;   non-temporal hints.
;
; Arguments:
;
;   Destination (rcx) - Supplies the address of the target memory.
;
;   Source (rdx) - Supplies the address of the source memory.
;
;   NumberOfPages (r8) - Supplies the number of pages to copy.
;
; Return Value:
;
;   None.
;
; Author's Notes:
;
;   The primary goals that influenced how this routine was written were:
;
;       a) Keeping all loop jumps short (that is, limiting hot-path jumps to a
;          maximum distance of +128 to -127 bytes from the RIP).
;
;       b) Aligning hot-path loop entries (Cpx10 and Cpx20) on 16 byte
;          boundaries.
;
;       c) Prediction-friendly jump organization for both hot and cold paths
;          (e.g. make the less frequent branch taken (jump forward)).
;
;       d) Avoiding false loop dependencies by using add/sub instead of inc/dec.
;
;   Satisfying these goals resulted in the two inner loops handling four
;   in-flight registers per iteration.
;
;--

        LEAF_ENTRY CopyPagesNonTemporalAvx2_v1, _TEXT$00

;
; Verify the NumberOfPages is greater than zero.
;

        test    r8, r8                      ; Test NumberOfPages against self.
        jz      short Cpx05                 ; Number of pages is 0, return.
        jmp     short Cpx07                 ; Number of pages >= 1, continue.

Cpx05:  ret

;
; This routine uses the following pattern for processing pages (inspired by
; KeCopyPage()): initialize a counter to -PAGE_SIZE, add PAGE_SIZE to both
; pointer targets, use [<base_reg> + <counter_reg> + constant * 0..3] for the
; four successive prefetch/load/store instructions.
;
; Thus, we move -PAGE_SIZE (0xffffffff`fffff000) to r10 once, up front.  When
; we advance the rcx and rdx destination and source pointers, we sub r10 from
; each value, and before entering each loop, we reset rax to r10, then increment
; it until it hits zero.  Thus, the three instructions after the mov r10 line
; below can be seen at multiple points in this routine.
;

Cpx07:  mov     r10, -PAGE_SIZE
        sub     rcx, r10                    ; Add page size to Destination ptr.
        sub     rdx, r10                    ; Add page size to Source ptr.
        mov     rax, r10                    ; Initialize counter register.


;
; Load the source page into the L0 cache via 16 loop iterations, with each loop
; iteration responsible for 256 bytes.  We prefetch 64 bytes at a time, which
; is cache-line sized.
;

        align   16

Cpx10:  prefetchnta [rdx + rax + 64 * 0]    ; Prefetch   0 -  63 bytes.
        prefetchnta [rdx + rax + 64 * 1]    ; Prefetch  64 - 127 bytes.
        prefetchnta [rdx + rax + 64 * 2]    ; Prefetch 128 - 191 bytes.
        prefetchnta [rdx + rax + 64 * 3]    ; Prefetch 192 - 255 bytes.

        add     rax, 64*4                   ; Add 256 bytes to rax.
        jnz     short Cpx10                 ; Repeat while bytes copied != 4k.

        mov     rax, r10                    ; Reinitialize counter register.
;
; Load four 256-bit, 32-byte YMM registers with memory from the source page
; that was loaded into the L0 cache above using non-temporal prefetch hints.
;

        align   16

Cpx20:  vmovntdqa   ymm0, ymmword ptr [rdx + rax + 32 * 0]  ; Load   0 -  31.
        vmovntdqa   ymm1, ymmword ptr [rdx + rax + 32 * 1]  ; Load  32 -  63.
        vmovntdqa   ymm2, ymmword ptr [rdx + rax + 32 * 2]  ; Load  64 -  95.
        vmovntdqa   ymm3, ymmword ptr [rdx + rax + 32 * 3]  ; Load  96 - 127.

;
; Copy the source data in YMM registers to the destination address.
;

        vmovntdq ymmword ptr [rcx + rax + 32 * 0], ymm0     ; Copy  0 -  31.
        vmovntdq ymmword ptr [rcx + rax + 32 * 1], ymm1     ; Copy 32 -  63.
        vmovntdq ymmword ptr [rcx + rax + 32 * 2], ymm2     ; Copy 64 -  95.
        vmovntdq ymmword ptr [rcx + rax + 32 * 3], ymm3     ; Copy 96 - 127.

;
; Increment the rax counter; multiply the number of registers in flight (4)
; by the register size (32 bytes).  Jump back to the start of the copy loop
; if we haven't copied 4096 bytes yet.
;

        add     rax, 32 * 4             ; Increment counter register.
        jnz     short Cpx20             ; Repeat copy whilst bytes copied != 4k.

;
; Decrement our NumberOfPages counter (r8).  If zero, we've copied all pages,
; and can jump to the end (Cpx80).
;
; N.B. `sub r8, 1` is used instead of `dec r8` because the latter introduces
;      a false dependency on the flags as it doesn't clear CF.
;

        sub     r8, 1                       ; --NumberOfPages.
        jz      short Cpx80                 ; No more pages, finalize.

;
; There are pages remaining.  Update our destination and source pointers by
; a page size again, and initialize rax to -PAGE_SIZE, then jump back to the
; start where we prefetch the next page.
;

        sub     rcx, r10                    ; Add page size to Destination ptr.
        sub     rdx, r10                    ; Add page size to Source ptr.
        mov     rax, r10                    ; Reinitialize counter register.
        jmp     short Cpx10                 ; Jump back to start.

;
; Force a memory barrier and return.
;

Cpx80:  sfence

Cpx90:  ret

        LEAF_END CopyPagesNonTemporalAvx2_v1, _TEXT$00

;++
;
; VOID
; CopyPagesNonTemporalAvx2_v2(
;     _In_ PCHAR Destination,
;     _In_ PCHAR Source,
;     _In_ ULONG NumberOfPages
;     );
;
; Routine Description:
;
;   This routine copies one or more pages of memory using AVX2 instructions and
;   non-temporal hints.
;
; Arguments:
;
;   Destination (rcx) - Supplies the address of the target memory.
;
;   Source (rdx) - Supplies the address of the source memory.
;
;   NumberOfPages (r8) - Supplies the number of pages to copy.
;
; Return Value:
;
;   None.
;
; Author's Notes:
;
;   Experiment with spare bytes in nops.
;
;--

        LEAF_ENTRY CopyPagesNonTemporalAvx2_v2, _TEXT$00

;
; Verify the NumberOfPages is greater than zero.
;

        test    r8, r8                      ; Test NumberOfPages against self.
        jz      short Cpy05                 ; Number of pages is 0, return.
        jmp     short Cpy07                 ; Number of pages >= 1, continue.

Cpy05:  ret

;
; This routine uses the following pattern for processing pages (inspired by
; KeCopyPage()): initialize a counter to -PAGE_SIZE, add PAGE_SIZE to both
; pointer targets, use [<base_reg> + <counter_reg> + constant * 0..3] for the
; four successive prefetch/load/store instructions.
;
; Thus, we move -PAGE_SIZE (0xffffffff`fffff000) to r10 once, up front.  When
; we advance the rcx and rdx destination and source pointers, we sub r10 from
; each value, and before entering each loop, we reset rax to r10, then increment
; it until it hits zero.  Thus, the three instructions after the mov r10 line
; below can be seen at multiple points in this routine.
;

Cpy07:  mov     r10, -PAGE_SIZE
        sub     rcx, r10                    ; Add page size to Destination ptr.
        sub     rdx, r10                    ; Add page size to Source ptr.
        mov     rax, r10                    ; Initialize counter register.
        mov     r11, r10                    ; Set adder register to -PAGE_SIZE.
        not     r11                         ; Convert to PAGE_SIZE (4096).
        shr     r11, 4                      ; Shift right 4 to convert to 256.
        jmp     Cpy10                       ; Jump over the nop.

;
; Load the source page into the L0 cache via 16 loop iterations, with each loop
; iteration responsible for 256 bytes.  We prefetch 64 bytes at a time, which
; is cache-line sized.
;

        align   16

Cpy10:  prefetchnta [rdx + rax + 64 * 0]    ; Prefetch   0 -  63 bytes.
        prefetchnta [rdx + rax + 64 * 1]    ; Prefetch  64 - 127 bytes.
        prefetchnta [rdx + rax + 64 * 2]    ; Prefetch 128 - 191 bytes.
        prefetchnta [rdx + rax + 64 * 3]    ; Prefetch 192 - 255 bytes.

        add     rax, r11                    ; Add 256 bytes to rax.
        jnz     short Cpy10                 ; Repeat while bytes copied != 4k.

        mov     rax, r10                    ; Reinitialize counter register.
        shr     r11, 1                      ; Adder register 256 -> 128.
        jmp     Cpy20                       ; Jump over the nop.
;
; Load four 256-bit, 32-byte YMM registers with memory from the source page
; that was loaded into the L0 cache above using non-temporal prefetch hints.
;

        align   16

Cpy20:  vmovntdqa   ymm0, ymmword ptr [rdx + rax + 32 * 0]  ; Load   0 -  31.
        vmovntdqa   ymm1, ymmword ptr [rdx + rax + 32 * 1]  ; Load  32 -  63.
        vmovntdqa   ymm2, ymmword ptr [rdx + rax + 32 * 2]  ; Load  64 -  95.
        vmovntdqa   ymm3, ymmword ptr [rdx + rax + 32 * 3]  ; Load  96 - 127.

;
; Copy the source data in YMM registers to the destination address.
;

        vmovntdq ymmword ptr [rcx + rax + 32 * 0], ymm0     ; Copy  0 -  31.
        vmovntdq ymmword ptr [rcx + rax + 32 * 1], ymm1     ; Copy 32 -  63.
        vmovntdq ymmword ptr [rcx + rax + 32 * 2], ymm2     ; Copy 64 -  95.
        vmovntdq ymmword ptr [rcx + rax + 32 * 3], ymm3     ; Copy 96 - 127.

;
; Increment the rax counter; multiply the number of registers in flight (4)
; by the register size (32 bytes).  Jump back to the start of the copy loop
; if we haven't copied 4096 bytes yet.
;

        add     rax, r11                ; Increment counter register.
        jnz     short Cpy20             ; Repeat copy whilst bytes copied != 4k.

;
; Decrement our NumberOfPages counter (r8).  If zero, we've copied all pages,
; and can jump to the end (Cpy80).
;
; N.B. `sub r8, 1` is used instead of `dec r8` because the latter introduces
;      a false dependency on the flags as it doesn't clear CF.
;

        sub     r8, 1                       ; --NumberOfPages.
        jz      short Cpy80                 ; No more pages, finalize.

;
; There are pages remaining.  Update our destination and source pointers by
; a page size again, and initialize rax to -PAGE_SIZE, then jump back to the
; start where we prefetch the next page.
;

        shl     r11, 1                      ; Adder register 128->256.
        sub     rcx, r10                    ; Add page size to Destination ptr.
        sub     rdx, r10                    ; Add page size to Source ptr.
        mov     rax, r10                    ; Reinitialize counter register.
        jmp     short Cpy10                 ; Jump back to start.

;
; Force a memory barrier and return.
;

Cpy80:  sfence

Cpy90:  ret

        LEAF_END CopyPagesNonTemporalAvx2_v2, _TEXT$00

;++
;
; VOID
; CopyPagesNonTemporalAvx2_v3(
;     _In_ PCHAR Destination,
;     _In_ PCHAR Source,
;     _In_ ULONG NumberOfPages
;     );
;
; Routine Description:
;
;   This routine copies one or more pages of memory using AVX2 instructions and
;   non-temporal hints.
;
; Arguments:
;
;   Destination (rcx) - Supplies the address of the target memory.
;
;   Source (rdx) - Supplies the address of the source memory.
;
;   NumberOfPages (r8) - Supplies the number of pages to copy.
;
; Return Value:
;
;   None.
;
; Author's Notes:
;
;   Experiment with spare bytes in nops.
;
;--

        LEAF_ENTRY CopyPagesNonTemporalAvx2_v3, _TEXT$00

;
; Verify the NumberOfPages is greater than zero.
;

        test    r8, r8                      ; Test NumberOfPages against self.
        jz      short Cpz05                 ; Number of pages is 0, return.
        jmp     short Cpz07                 ; Number of pages >= 1, continue.

Cpz05:  ret

;
; This routine uses the following pattern for processing pages (inspired by
; KeCopyPage()): initialize a counter to -PAGE_SIZE, add PAGE_SIZE to both
; pointer targets, use [<base_reg> + <counter_reg> + constant * 0..3] for the
; four successive prefetch/load/store instructions.
;
; Thus, we move -PAGE_SIZE (0xffffffff`fffff000) to r10 once, up front.  When
; we advance the rcx and rdx destination and source pointers, we sub r10 from
; each value, and before entering each loop, we reset rax to r10, then increment
; it until it hits zero.  Thus, the three instructions after the mov r10 line
; below can be seen at multiple points in this routine.
;

Cpz07:  mov     r10, -PAGE_SIZE
        mov     r11, 128
        sub     rcx, r10                    ; Add page size to Destination ptr.
        sub     rdx, r10                    ; Add page size to Source ptr.
        mov     rax, r10                    ; Initialize counter register.
        jmp     Cpz10                       ; Jump over the nop.

;
; Load the source page into the L0 cache via 16 loop iterations, with each loop
; iteration responsible for 256 bytes.  We prefetch 64 bytes at a time, which
; is cache-line sized.
;

        align   16

Cpz10:  prefetchnta [rdx + rax + 64 * 0]    ; Prefetch   0 -  63 bytes.
        prefetchnta [rdx + rax + 64 * 1]    ; Prefetch  64 - 127 bytes.

        add     rax, r11                    ; Add 128 bytes to rax.
        jnz     short Cpz10                 ; Repeat while bytes copied != 4k.

        mov     rax, r10                    ; Reinitialize counter register.
        jmp     Cpz20                       ; Jump over the nop.

;
; Load four 256-bit, 32-byte YMM registers with memory from the source page
; that was loaded into the L0 cache above using non-temporal prefetch hints.
;

        align   16

Cpz20:  vmovntdqa   ymm0, ymmword ptr [rdx + rax + 32 * 0]  ; Load   0 -  31.
        vmovntdqa   ymm1, ymmword ptr [rdx + rax + 32 * 1]  ; Load  32 -  63.
        vmovntdqa   ymm2, ymmword ptr [rdx + rax + 32 * 2]  ; Load  64 -  95.
        vmovntdqa   ymm3, ymmword ptr [rdx + rax + 32 * 3]  ; Load  96 - 127.

;
; Copy the source data in YMM registers to the destination address.
;

        vmovntdq ymmword ptr [rcx + rax + 32 * 0], ymm0     ; Copy  0 -  31.
        vmovntdq ymmword ptr [rcx + rax + 32 * 1], ymm1     ; Copy 32 -  63.
        vmovntdq ymmword ptr [rcx + rax + 32 * 2], ymm2     ; Copy 64 -  95.
        vmovntdq ymmword ptr [rcx + rax + 32 * 3], ymm3     ; Copy 96 - 127.

;
; Increment the rax counter; multiply the number of registers in flight (4)
; by the register size (32 bytes).  Jump back to the start of the copy loop
; if we haven't copied 4096 bytes yet.
;

        add     rax, r11                ; Increment counter register.
        jnz     short Cpz20             ; Repeat copy whilst bytes copied != 4k.

;
; Decrement our NumberOfPages counter (r8).  If zero, we've copied all pages,
; and can jump to the end (Cpz80).
;
; N.B. `sub r8, 1` is used instead of `dec r8` because the latter introduces
;      a false dependency on the flags as it doesn't clear CF.
;

        sub     r8, 1                       ; --NumberOfPages.
        jz      short Cpz80                 ; No more pages, finalize.

;
; There are pages remaining.  Update our destination and source pointers by
; a page size again, and initialize rax to -PAGE_SIZE, then jump back to the
; start where we prefetch the next page.
;

        sub     rcx, r10                    ; Add page size to Destination ptr.
        sub     rdx, r10                    ; Add page size to Source ptr.
        mov     rax, r10                    ; Reinitialize counter register.
        jmp     short Cpz10                 ; Jump back to start.

;
; Force a memory barrier and return.
;

Cpz80:  sfence

Cpz90:  ret

        LEAF_END CopyPagesNonTemporalAvx2_v3, _TEXT$00

;++
;
; VOID
; CopyPagesNonTemporalAvx2_v4(
;     _In_ PCHAR Destination,
;     _In_ PCHAR Source,
;     _In_ ULONG NumberOfPages
;     );
;
; Routine Description:
;
;   This routine copies one or more pages of memory using AVX2 instructions and
;   non-temporal hints.
;
; Arguments:
;
;   Destination (rcx) - Supplies the address of the target memory.
;
;   Source (rdx) - Supplies the address of the source memory.
;
;   NumberOfPages (r8) - Supplies the number of pages to copy.
;
; Return Value:
;
;   None.
;
; Author's Notes:
;
;   Experiment with spare bytes in nops.
;
;--

        LEAF_ENTRY CopyPagesNonTemporalAvx2_v4, _TEXT$00

;
; Verify the NumberOfPages is greater than zero.
;

        test    r8, r8                      ; Test NumberOfPages against self.
        jz      short Cpa05                 ; Number of pages is 0, return.
        jmp     short Cpa07                 ; Number of pages >= 1, continue.

Cpa05:  ret

;
; This routine uses the following pattern for processing pages (inspired by
; KeCopyPage()): initialize a counter to -PAGE_SIZE, add PAGE_SIZE to both
; pointer targets, use [<base_reg> + <counter_reg> + constant * 0..3] for the
; four successive prefetch/load/store instructions.
;
; Thus, we move -PAGE_SIZE (0xffffffff`fffff000) to r10 once, up front.  When
; we advance the rcx and rdx destination and source pointers, we sub r10 from
; each value, and before entering each loop, we reset rax to r10, then increment
; it until it hits zero.  Thus, the three instructions after the mov r10 line
; below can be seen at multiple points in this routine.
;

Cpa07:  mov     r10, -PAGE_SIZE
        mov     r11, 128
        sub     rcx, r10                    ; Add page size to Destination ptr.
        sub     rdx, r10                    ; Add page size to Source ptr.
        mov     rax, r10                    ; Initialize counter register.
        jmp     Cpa10                       ; Jump over the nop.

        align   16

Cpa10:  prefetchnta [rdx + rax + 64 * 0]    ; Prefetch   0 -  63 bytes.
        prefetchnta [rdx + rax + 64 * 1]    ; Prefetch  64 - 127 bytes.

        vmovntdqa   ymm0, ymmword ptr [rdx + rax + 32 * 0]  ; Load   0 -  31.
        vmovntdqa   ymm1, ymmword ptr [rdx + rax + 32 * 1]  ; Load  32 -  63.
        vmovntdqa   ymm2, ymmword ptr [rdx + rax + 32 * 2]  ; Load  64 -  95.
        vmovntdqa   ymm3, ymmword ptr [rdx + rax + 32 * 3]  ; Load  96 - 127.

;
; Copy the source data in YMM registers to the destination address.
;

        vmovntdq ymmword ptr [rcx + rax + 32 * 0], ymm0     ; Copy  0 -  31.
        vmovntdq ymmword ptr [rcx + rax + 32 * 1], ymm1     ; Copy 32 -  63.
        vmovntdq ymmword ptr [rcx + rax + 32 * 2], ymm2     ; Copy 64 -  95.
        vmovntdq ymmword ptr [rcx + rax + 32 * 3], ymm3     ; Copy 96 - 127.

;
; Increment the rax counter; multiply the number of registers in flight (4)
; by the register size (32 bytes).  Jump back to the start of the copy loop
; if we haven't copied 4096 bytes yet.
;

        add     rax, r11                ; Increment counter register.
        jnz     short Cpa10             ; Repeat copy whilst bytes copied != 4k.

;
; Decrement our NumberOfPages counter (r8).  If zero, we've copied all pages,
; and can jump to the end (Cpa80).
;
; N.B. `sub r8, 1` is used instead of `dec r8` because the latter introduces
;      a false dependency on the flags as it doesn't clear CF.
;

        sub     r8, 1                       ; --NumberOfPages.
        jz      short Cpa80                 ; No more pages, finalize.

;
; There are pages remaining.  Update our destination and source pointers by
; a page size again, and initialize rax to -PAGE_SIZE, then jump back to the
; start where we prefetch the next page.
;

        sub     rcx, r10                    ; Add page size to Destination ptr.
        sub     rdx, r10                    ; Add page size to Source ptr.
        mov     rax, r10                    ; Reinitialize counter register.
        jmp     short Cpa10                 ; Jump back to start.

;
; Force a memory barrier and return.
;

Cpa80:  sfence

Cpa90:  ret

        LEAF_END CopyPagesNonTemporalAvx2_v4, _TEXT$00



; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=:;            :

end
