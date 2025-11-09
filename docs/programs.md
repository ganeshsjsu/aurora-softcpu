# Example Programs

## Hello, World

File: `programs/hello.asm`

Demonstrates immediate loads, indirect addressing, memory-mapped IO, and control flow.

Key steps:
1. Load the `message` label address into `r0`.
2. Loop: `LOAD r1, [r0]`, compare against 0, store to console, increment pointer.
3. Halt after null terminator.

## Fibonacci sequence

File: `programs/fibonacci.asm`

Uses register arithmetic, subroutines, stack, shifts, logic, and IO:

- Maintains `a` and `b` in `r0`/`r1`, printing `a` each iteration.
- Calls `print_hex`, which preserves caller registers by pushing `r4–r6`.
- The printer writes "0x" then iterates over four hex digits using shifts and masking, mapping 0–9/10–15 to ASCII.
- Execution halts after `FIB_COUNT` iterations.

The video in `assets/fibonacci_demo.mp4` narrates the control flow and register transitions for this program.

## Timer + LED example (fetch/compute/store cycles)

File: `programs/timer.asm`

This program configures the timer and mirrors its counter onto the LED panel.

### Instruction timeline

| Cycle | Phase | Action |
|-------|-------|--------|
| 1 | **Fetch** | PC=0x0000, fetch `LDI r0, #0x0100`. | 
| 1 | **Compute** | Decode operand: register `r0`, immediate literal word appended. |
| 1 | **Store** | Write literal `0x0100` into `r0`, PC advances to `0x0006`. Timer tick increments divider. |
| 2 | **Fetch** | Fetch `STORE r0, [IO_TIMER_PERIOD]` (opcode + operands + literal address). |
| 2 | **Compute** | Resolve source register (`r0`) and absolute address `0xFF13`. |
| 2 | **Store** | Bus writes two bytes to the timer's period registers. |
| 3 | **Fetch** | `LDI r0, #0x0003` (enable & auto-reload flags). |
| 3 | **Compute** | Literal resolved. |
| 3 | **Store** | Value written to `r0`. |
| 4 | **Fetch** | `STORE r0, [IO_TIMER_CONTROL]`. |
| 4 | **Store** | Bus write toggles timer enable + reset. |
| 5+ | **Loop** | `LOAD r1, [IO_TIMER_COUNTER]` reads counter, `STORE r1, [IO_LED]` mirrors to LEDs, `JMP loop` repeats. Each cycle begins with fetch of the next instruction, compute resolves operands, store commits to device registers, and the timer `tick()` fires automatically before decode.

This cycle-level walkthrough aligns with the general Fetch/Compute/Store model outlined in `docs/architecture.md`.
