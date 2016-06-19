        title "Tracer Control Driver Utility Routines"
;++
;
; Copyright (c) Trent Nelson, 2016.
;
; Module Name:
;
;   DriverUtil.asm
;
; Abstract:
;
;   This module implements miscellaneous assembly routines called by the
;   TracerControl kernel driver.
;
;--


include ksamd64.inc

;++
;
; VOID
; ReadCr3(_Out_ PULONGLONG Buffer)
;
; Routine Description:
;
;   This routine reads the Cr3 control register and writes the contents into
;   the destination address pointed to by Buffer.
;
; Arguments:
;
;   Buffer - A pointer to the destination address.
;
; Return Value:
;
;   None
;
;--

        LEAF_ENTRY ReadCr3, _TEXT$00

        mov     rax, Cr3        ; Save Cr3 into rax.
        mov     [rcx], rax      ; Write to user's buffer.
        xor     rax, rax        ; Clear rax as we don't return a value.
        ret

        LEAF_END ReadCr3, _TEXT$00

; vim:set tw=80 ts=8 sw=8 sts=8 expandtab syntax=masm formatoptions=croql      :

end
