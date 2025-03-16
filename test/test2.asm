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
