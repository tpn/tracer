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

end

; vim:set tw=80 ts=8 sw=8 sts=8 expandtab:
