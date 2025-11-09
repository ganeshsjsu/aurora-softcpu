# Emulator and Tooling

The `softcpu` binary links together the Aurora core (memory, bus, devices, CPU) and exposes helper commands.

## Components

- **Memory:** 64 KiB byte array with little-endian helper methods. Safe block loading prevents overruns.
- **Bus:** Arbitrates between RAM and IO devices. IO devices register a base + size, and the bus forwards read/write/tick events.
- **Devices:**
  - `ConsoleDevice` – writes a character buffer and mirrors output to stdout.
  - `TimerDevice` – programmable divider with enable/auto-reload, period registers, and a simple counter.
  - `LedPanel` – holds an 8-bit latch.
- **CPU:** Couples register file, ALU, and control unit. Each `step()` ticks devices, fetches, decodes, executes, and updates flags/PC.

## Commands

| Command | Description |
|---------|-------------|
| `softcpu assemble <file> -o <bin>` | Produces a binary image. `--origin` overrides starting address. |
| `softcpu run <bin> [--origin addr] [--entry addr] [--cycles N] [--trace]` | Loads binary, resets CPU, sets PC, and executes until HALT or cycle limit. Trace prints each opcode. |
| `softcpu dump <bin> --start addr --length N [--origin addr]` | Hex-dumps a span of memory after loading a binary.

## Load, run, dump workflow

```
./softcpu assemble programs/hello.asm -o build/hello.bin
./softcpu run build/hello.bin --origin 0 --entry 0 --trace
./softcpu dump build/hello.bin --origin 0 --start 0 --length 32
```

## Memory-mapped IO

| Device | Range | Registers |
|--------|-------|-----------|
| Console | `0xFF00` | `0xFF00` data (write), `0xFF01` status (bit0=ready). |
| Timer | `0xFF10` | `0xFF10/11` counter, `0xFF12` control, `0xFF13/14` period. |
| LEDs | `0xFF20` | `0xFF20` latch. |

IO writes via `STORE` or `OUT` are forwarded byte-by-byte. The timer device increments every CPU cycle and supports auto-reload.

## Debug aids

- `--trace` prints `PC` and instruction mnemonic, interleaved with console output for live debugging.
- `SYS 2` prints register state (`[R0=...]`) to stdout.
- The assembler injects default symbols `IO_CONSOLE_DATA`, `IO_TIMER_COUNTER`, `IO_TIMER_CONTROL`, etc., for ergonomic code.
