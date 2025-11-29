; Hello World for SoftCPU-16
.const IO_CONSOLE_DATA 0xFF00

        .org 0x0000
start:
        LDI r0, #message
next_char:
        LOAD r1, [r0]
        CMP r1, #0
        JZ done
        STORE r1, [IO_CONSOLE_DATA]
        ADDI r0, #1
        JMP next_char

done:
        HALT

message:
        .asciiz "Hello, World!\n"
