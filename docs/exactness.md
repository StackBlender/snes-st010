# How exact it is

Checked by `st010_check` and by random commands put to the original chip (running in the ares
emulator, its program taken at run time from an installation outside this repository;
docs/clean-room.md).

## F1 ROC II

From power-on into a race (7,200 frames): 7,764,898 of the 7,764,923 reads the game made of the
chip's RAM give the same byte here. Of the 25 others, 21 come from the sine table (below) and 4
are the game reading 000h while the original was still busy with command 01 (this class answers
at once). Over 20,000 more frames of the race (started from a saved state, so the first frame's
tables weren't computed here): 29,187,809 of 29,191,776.

## Each command

Random inputs, every result compared:

| Command | |
|---|---|
| 02, 03, 04, 06 | exact (sort: every count 0-17; scale 600, length 805, multiply 800 random inputs) |
| 01 | 1,998 of 2,000 random vectors; on all 3,968 vectors of 5-bit values, all but the 8 that come from (8, 31) and (9, 10), in every octant |
| 07, 08 | exact but for the sine table: 240 of its 256 entries are round(32767 sin); the other 16 differ by 1 (rotation: about 90% of random inputs exact) |
| 00 | exact (it does nothing) |
| 05 | not implemented |

The two tables' odd entries are left as they are: making them match would mean copying the
original's values (docs/clean-room.md). Inputs of -32768 to 01 make the original hang; here
they give an answer.
