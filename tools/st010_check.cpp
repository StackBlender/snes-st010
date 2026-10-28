// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) StackBlender LLC
// Replays recorded traffic with the original chip through St010 and counts the RAM reads that
// differ. A recording has one line per access to the chip's RAM: "pw AAA VV" (the SNES wrote VV
// at AAA) or "pr AAA VV" (it read VV); other lines are ignored. Reads of 020h and 021h aren't
// compared: they show the original still busy, where this class answers at once. Recordings
// come from the original chip (docs/clean-room.md) and stay out of this repository. The RAM
// starts as a blank save, all FFh (as the recordings' emulator gives it).
//   st010_check RECORDING...
#include "st010/St010.h"

#include <cstdio>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
	if(argc < 2) return std::fprintf(stderr, "usage: st010_check RECORDING...\n"), 2;
	int failed = 0;
	for(int f = 1; f < argc; f++) {
		std::ifstream in(argv[f]);
		st010::St010 chip;
		for(size_t i = 0; i < st010::St010::RamSize; i++) chip.ram()[i] = 0xff;
		std::string kind, a, v;
		long reads = 0, same = 0, line = 0, firstDiff = -1;
		while(in >> kind) {
			if(kind != "pw" && kind != "pr") { std::getline(in, a); continue; }
			in >> a >> v;
			line++;
			uint16_t address = uint16_t(std::stoul(a, nullptr, 16));
			uint8_t value = uint8_t(std::stoul(v, nullptr, 16));
			if(kind == "pw") { chip.writeRam(address, value); continue; }
			if(address == 0x20 || address == 0x21) continue;
			reads++;
			if(chip.readRam(address) == value) same++;
			else if(firstDiff < 0) firstDiff = line;
		}
		std::printf("%s: %ld of %ld reads the same", argv[f], same, reads);
		if(firstDiff >= 0) std::printf(" (first difference at access %ld)", firstDiff), failed++;
		std::printf("\n");
	}
	return failed ? 1 : 0;
}
