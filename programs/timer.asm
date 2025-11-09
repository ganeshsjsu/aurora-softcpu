; Timer driven LED demo
.const IO_TIMER_COUNTER 0xFF10
.const IO_TIMER_CONTROL 0xFF12
.const IO_TIMER_PERIOD 0xFF13
.const IO_LED 0xFF20

        .org 0x0000
start:
        LDI r0, #0x0100        ; timer period (256 cycles)
        STORE r0, [IO_TIMER_PERIOD]
        LDI r0, #0x0003        ; enable | auto reload
        STORE r0, [IO_TIMER_CONTROL]

loop:
        LOAD r1, [IO_TIMER_COUNTER]
        STORE r1, [IO_LED]
        JMP loop
