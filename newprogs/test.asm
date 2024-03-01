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
        mov     eax, BUFFER

print_buf:
        add     eax, 1
        mov     ebx, [eax - 1]
        cmp     ebx, 0
        jnz     print_buf

close_prog:
        mov     eax, -1                  ; Function -1 : close this program
        mcall
;  *********************************************
;  *************   DATA AREA   *****************
;  *********************************************

INFOSTRUCT:
        dd 0
        dd 0
        dd 0
        dd 63 ; does this go to EOF or EOL?
        dd BUFFER
        db  "/hd0/1/hw.txt", 0x0

I_END: ; this is extremely important! without this, we won't get our

BUFFER: db 64 dup(0)
