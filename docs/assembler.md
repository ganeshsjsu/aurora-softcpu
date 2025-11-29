# SoftCPU-16 Assembler

SoftCPU's two-pass assembler converts readable assembly into binary images that the emulator can load directly.

## Pipeline

1. **Lex & normalize:** strip comments (semicolon or `//`), split labels, and normalize whitespace.
2. **Pass 1:** maintain a location-counter, emit instruction headers, directives, and track unresolved symbols (labels/constants not yet defined).
3. **Pass 2:** resolve pending operands, patch immediates/addresses/offsets.
4. **Output:** raw little-endian byte stream suitable for `softcpu run`.

## Supported directives

| Directive | Description |
|-----------|-------------|
| `.org addr` | Sets the assembler location counter. |
| `.word val, val...` | Emits 16-bit values (accepts immediates, symbols, char literals). |
| `.byte val, ...` | Emits bytes. |
| `.ascii "text"` | Emits literal bytes. |
| `.asciiz "text"` | Same as `.ascii` with null terminator. |
| `.fill count, value` | Repeats a byte pattern. |
| `.const name, value` / `.equ` | Creates absolute symbols available to instructions and later directives. |

## Operands & literals

- **Registers:** `r0`–`r7`, `sp` alias of `r7`.
- **Immediate:** prefix with `#` (e.g., `#42`, `#0x1234`). Characters use `'A'`. Binary (`0b1010`), hex (`0xFF` or `$FF`), or decimal.
- **Memory:** `[r0]`, `[r1 + 4]`, `[LABEL]`, or absolute addresses `0x2000`.
- **Ports:** `port.console`, `port.leds`, or numeric (`port:3`).

## Labels

Labels terminate with `:`. The assembler automatically exports them as relocatable symbols for future references.

```
loop:
    CMP r0, #10
    JC done
    ADDI r0, #1
    JMP loop
```

## Error reporting

All diagnostics point to line numbers and are echoed during `softcpu assemble`. The CLI exits non-zero if any errors remain unresolved.

## Tooling tips

- Combine `softcpu assemble` + `softcpu run --trace` to verify instruction encodings.
- Use `.const` for IO base addresses to avoid literal duplication.
- `softcpu dump` is handy for inspecting emitted bytes or stack frames.
