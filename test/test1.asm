mcro mcb
    line1
    line2
mcroend

mcro mcc
    line3
    line4
    line5
mcroend

DATA_LABEL: .data 5, -3, 10

START:
    mcb

ENTRY_LABEL: .entry

.entry ENTRY_LABEL

.extern ENTRY_LABEL

EXTERN_LABEL: .extern

LOOP: mcc
    jmp START
    cmp LOOP, ENTRY_LABEL

END:
    line1
    line2
    bne LOOP
