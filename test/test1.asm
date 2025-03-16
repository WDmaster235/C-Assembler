mcro mcb
    line1
    line2
mcroend

; mcro mov
;     line1
;     line2
; mcroend

mcro mcc
    line3
    line4
    line5
mcroend

DATA_LABEL: .data 5, -3, 10

mov:
    mcb

LOOP:
    mcc
    jmp START
    cmp LOOP, START

END:
    line1
    line2
    bne LOOP
