MAIN: add r3, LIST
LOOP: prn #48
mcro a_mc
cmp K, #-6
 bne &END
mcroend
<<<<<<< HEAD

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
=======
 lea STR, r6
 inc r6
 mov r3, K
 sub r1, r4
 bne END
a_mc
 dec K
 jmp &LOOP
END: stop
STR: .string “abcd”
LIST: .data 6, -9
 .data -100
K: .data 31 
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
