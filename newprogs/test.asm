format binary as ""                     ; Binary file format without extenstion

use32                                   ; Tell compiler to use 32 bit instructions

org 0x0                                 ; the base address of code, always 0x0

; The header

db 'MENUET01'
dd 0x01
dd START
dd I_END
dd 0x100000
dd 0x7fff0
dd 0, 0

; The code area

include 'macros.inc'

START:                                  ; start of execution

event_wait:
        ; @todo call mcall 70, < PARAM > for reading file

        mcall   70, INFOSTRUCT
        mov     ebx, [BUFFER]


close_prog:
        mov     eax,-1                  ; Function -1 : close this program
        mcall
;  *********************************************
;  *************   DATA AREA   *****************
;  *********************************************
;
; Data can be freely mixed with code to any parts of the image.
; Only the header information is required at the beginning of the image.

INFOSTRUCT:
        dd 0
        dd 0
        dd 0
        dd 1
        dd BUFFER
        db  "/hd0/1/hw.txt", 0x0

; @todo
; - How do I parse files?
; - How do I utilize the internet drivers?
; - Scope of C being involved?

I_END: ; this is extremely important! without this, we won't get our ending address.

BUFFER: rb 64
