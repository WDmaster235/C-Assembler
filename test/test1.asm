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
    mcb  

LOOP:
    mcc  
    jmp START
    cmp LOOP, START

END:
    line1
    line2
    bne LOOP
