# snes-st010

A free, clean-room implementation of the ST010, the Seta coprocessor (an NEC uPD96050) in F1 ROC
II: Race of Champions, under the GPL. It answers the chip's commands with the same results as the
original, so emulators can run the game without the original chip's program, which is the chip
maker's code.

F1 ROC II's reads of the chip, from power-on into a race: 99.9997% the same (25 of 7.8 million
differ, from two tables of the chip's whose odd entries aren't copied). The commands:
[docs/commands.md](docs/commands.md); how exact: [docs/exactness.md](docs/exactness.md).

```cpp
#include "st010/St010.h"

st010::St010 chip;
chip.writeRam(address, byte);   // the chip's 4 KiB of RAM as the SNES maps it; setting bit 7 of
uint8_t b = chip.readRam(address);  // 021h runs the command in 020h at once
uint8_t* save = chip.ram();     // battery-backed: keep it with the game's save
auto state = chip.saveState();  // for save states
chip.loadState(state.data(), state.size());
```

```sh
cmake -S . -B build && cmake --build build && (cd build && ctest)
build/st010_check RECORDING...   # replay traffic recorded with the original chip
```

Made by [StackBlender](https://stackblender.com) for Super Apollo, an emulator that plays
SNES games fast with their music at normal speed. Its other chips are published the same
way: [snes-dsp1](https://github.com/StackBlender/snes-dsp1),
[snes-dsp2](https://github.com/StackBlender/snes-dsp2),
[snes-cx4](https://github.com/StackBlender/snes-cx4),
[snes-dsp4](https://github.com/StackBlender/snes-dsp4).

## How it is made

Clean room: written from the public list of the chip's commands (their names) and from measuring
what the original does, never from the original program's bytes or a disassembly of it, which
nobody on this project reads. It is checked
against the original running in the [ares](https://github.com/ares-emulator/ares) emulator,
answer for answer; the tests take the original from the emulator at run time, and it is
never in this repository. The rules: [docs/clean-room.md](docs/clean-room.md).

## License

GNU General Public License, version 3 or (at your option) any later version; see
[LICENSE](LICENSE). For other terms, contact [StackBlender](https://stackblender.com).
