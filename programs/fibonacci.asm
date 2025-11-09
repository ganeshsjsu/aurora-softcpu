; Fibonacci sequence demo
.const IO_CONSOLE_DATA 0xFF00
.const HEX_ALPHA_OFFSET 55
.const FIB_COUNT 12

        .org 0x0000
start:
        LDI r0, #0          ; a
        LDI r1, #1          ; b
        LDI r2, #FIB_COUNT
loop:
        MOV r3, r0          ; value to print
        CALL print_hex
        LDI r4, #10
        STORE r4, [IO_CONSOLE_DATA]

        MOV r4, r1
        ADD r1, r0          ; b = a + b
        MOV r0, r4          ; a = old b

        SUBI r2, #1
        JNZ loop

        HALT

; Prints R3 in hexadecimal (0x0000 format) and preserves caller registers
print_hex:
        PUSH r4
        PUSH r5
        PUSH r6
        LDI r4, #'0'
        STORE r4, [IO_CONSOLE_DATA]
        LDI r4, #'x'
        STORE r4, [IO_CONSOLE_DATA]
        LDI r6, #12         ; starting shift amount
hex_loop:
        MOV r5, r3
        SHR r5, r6
        AND r5, #0x000F
        LDI r4, #10
        CMP r5, r4
        JC letter
        ADDI r5, #'0'
        JMP write_digit
letter:
        ADDI r5, #HEX_ALPHA_OFFSET
write_digit:
        STORE r5, [IO_CONSOLE_DATA]
        CMP r6, #0
        JZ done_hex
        SUBI r6, #4
        JMP hex_loop

done_hex:
        LDI r4, #' '
        STORE r4, [IO_CONSOLE_DATA]
        POP r6
        POP r5
        POP r4
        RET
