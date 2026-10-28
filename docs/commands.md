# The ST010's commands, as measured

How each command behaves, worked out by putting questions to the original chip (running in an
emulator; docs/clean-room.md) and recording F1 ROC II's traffic with it. Numbers are 16-bit,
least significant byte first, signed unless said otherwise. Angles: 256 steps a turn (the high
byte of a 16-bit angle; 4000h = 90 degrees). "sin" and "cos" are the chip's table, round(32767
sin(step * 360 degrees / 256)); cos(step) = sin(step + 64).

| Command | Inputs | Results |
|---|---|---|
| 00 | | nothing |
| 01 | x at 000, y at 002 | 010: the angle of (-x, -y) from the y axis (in its high byte). \|x\| and \|y\| are first shifted right together until the larger is under 32 (one that becomes 0 but wasn't is 1); the angle is round((atan2(x', y') + 180 degrees) * 256 / 360 degrees) of those. Left behind: 000 and 002 those two values (swapped when x and y have opposite signs, x < 0 with y = 0 counting), 004 the quadrant (0, 4000h for x >= 0 and y < 0, 8000h for both negative, C000h for x < 0 and y >= 0), 006 y |
| 02 | n at 024 (unsigned); keys at 040, values at 080 | for 2 <= n <= 15, the pairs sorted by key (unsigned), largest first, equal keys keeping their order; otherwise unchanged. 000: 0, or n when it wasn't sorted |
| 03 | x 000, y 002, s 004 | 2 x s at 010 and 2 y s at 014, 32-bit each |
| 04 | x 000, y 002 | 010: (max * 31472 + min * 13036 + 16384) / 32768, rounded down, of \|x\| and \|y\| (an estimate of the length: "alpha max plus beta min"). \|-32768\| stays 8000h; max and min are chosen unsigned, multiplied signed. Left behind: 000 the larger, 002 the smaller magnitude |
| 05 | | not measured (F1 ROC II doesn't use it) |
| 06 | a 000, b 002 | 2 a b at 010, 32-bit |
| 07 | angle 000 | for the 176 lines n of the road: P = round(7885 / (n + 8.8)); P cos / 32768 at 0F0h + 2n and at 510h + 2n, P sin / 32768 at 250h + 2n, -P sin / 32768 at 3B0h + 2n (each rounded down). Left behind: 000 the step, 002 sin, 004 cos |
| 08 | x 000, y 002, angle 004 | x cos / 32768 + y sin / 32768 at 010, y cos / 32768 - x sin / 32768 at 012 (each product rounded down) |

09h-0Fh act as 01h-07h, 10h-FFh as 00h-0Fh. Each command clears 020h and bit 7 of 021h when done.

After power-on the original waits for one 16-bit transfer through its data register before it
takes commands (F1 ROC II's cartridge never makes one: an emulator's start-up difference, not
modelled here).
