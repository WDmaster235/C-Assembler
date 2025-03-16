<<<<<<< HEAD
    line1
    line2
    line1
    line2
    line1
    line2
DATA_LABEL: .data 5, -3, 10
    line1
    line2
mov:
    line1
    line2
    line1
    line2
LOOP:
    mcc
    jmp START
    cmp LOOP, START
    line1
    line2
END:
    line1
    line2
    bne LOOP
=======
MAIN: add r3, LIST
LOOP: prn #48
mcro a_mc
cmp K, #-6
 bne &END
mcroend
 lea STR, r6
 inc r6
 mov r3, K
 sub r1, r4
 bne END
cmp K, #-6
 bne &END
 dec K
 jmp &LOOP
END: stop
STR: .string “abcd”
LIST: .data 6, -9
 .data -100
K: .data 31 
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
