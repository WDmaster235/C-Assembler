mcro mcb
    line1
    line2
mcroend

mcro mcc
    line3
    line4
    line5
mcroend

; mcro mov
;     line6
; mcroend

START:
    line1
    line2

LOOP:
    line3
    line4
    line5
    jmp START
    cmp LOOP, START

END:
    line1
    line2
    bne LOOP
