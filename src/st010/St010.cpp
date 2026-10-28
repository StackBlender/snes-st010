// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) StackBlender LLC
#include "st010/St010.h"

#include <algorithm>
#include <cmath>

namespace st010 {

namespace {

constexpr double Pi = 3.14159265358979323846;

// x / 2^15, rounded down (also for negative x)
int32_t shift15(int64_t x) { return int32_t(x >= 0 ? x >> 15 : -((-x + 32767) >> 15)); }

uint16_t magnitude(int16_t v) { return uint16_t(v < 0 ? -int32_t(v) : v); }  // -32768 stays 8000h

} // namespace

St010::St010() {}

void St010::writeRam(uint16_t address, uint8_t value) {
	address &= 0xfff;
	m_ram[address] = value;
	if(address == 0x21 && value & 0x80) execute();
}

// round(32767 sin(step * 360 degrees / 256))
int16_t St010::sine(uint8_t step) {
	static const auto table = [] {
		std::array<int16_t, 256> t{};
		for(int i = 0; i < 256; i++) t[i] = int16_t(std::floor(32767.0 * std::sin(2 * Pi * i / 256) + 0.5));
		return t;
	}();
	return table[step];
}

// "alpha max plus beta min": (max * 31472 + min * 13036) / 32768, rounded, of |x| and |y|
uint16_t St010::distance(int16_t x, int16_t y) {
	uint16_t ax = magnitude(x), ay = magnitude(y);
	int32_t big = int16_t(std::max(ax, ay)), small = int16_t(std::min(ax, ay));
	return uint16_t(shift15(int64_t(big) * 31472 + int64_t(small) * 13036 + 16384));
}

// The angle of (-x, -y) from the y axis, 256 steps a turn: round((atan2(x, y) + 180 degrees)
// * 256 / 360 degrees). x and y are first scaled as command 01 does (both under 32).
uint8_t St010::direction(int16_t x, int16_t y) {
	if(!x && !y) return 0x80;
	return uint8_t(int(std::floor((std::atan2(double(x), double(y)) + Pi) * 256 / (2 * Pi) + 0.5)) & 255);
}

// The road's distance at screen line n: 7885 / (n + 8.8), rounded
int16_t St010::perspective(int line) {
	int32_t d = 5 * line + 44;
	return int16_t((2 * 39425 + d) / (2 * d));
}

void St010::execute() {
	uint8_t command = m_ram[0x20] & 0x0f;  // 10h-FFh act as 00h-0Fh; 09h-0Fh as 01h-07h
	if(command >= 9) command -= 8;
	switch(command) {
	case 0x00:  // nothing (measured: it doesn't clear 010h, as the public list says it does)
		break;
	case 0x01: {  // the direction of a vector
		int16_t x = r16(0x00), y = r16(0x02);
		uint16_t ax = magnitude(x), ay = magnitude(y), big = std::max(ax, ay);
		int shift = 0;
		while((big >> shift) >= 32) shift++;
		uint16_t nx = uint16_t(ax >> shift), ny = uint16_t(ay >> shift);
		if(ax && !nx) nx = 1;
		if(ay && !ny) ny = 1;
		bool swap = (x < 0 && y >= 0) || (x > 0 && y < 0);
		w16(0x00, swap ? ny : nx);
		w16(0x02, swap ? nx : ny);
		w16(0x04, x >= 0 ? (y >= 0 ? 0x0000 : 0x4000) : (y >= 0 ? 0xc000 : 0x8000));
		w16(0x06, uint16_t(y));
		w16(0x10, uint16_t(direction(int16_t(x >= 0 ? nx : -nx), int16_t(y >= 0 ? ny : -ny)) << 8));
		break;
	}
	case 0x02: {  // sort up to 15 key/value pairs by key, largest first (equal keys keep their order)
		uint16_t n = uint16_t(r16(0x24));
		if(n >= 2 && n < 16) {
			std::vector<std::pair<uint16_t, uint16_t>> pairs;
			for(unsigned i = 0; i < n; i++) pairs.push_back({uint16_t(r16(0x40 + 2 * i)), uint16_t(r16(0x80 + 2 * i))});
			std::stable_sort(pairs.begin(), pairs.end(), [](auto& a, auto& b) { return a.first > b.first; });
			for(unsigned i = 0; i < n; i++) w16(0x40 + 2 * i, pairs[i].first), w16(0x80 + 2 * i, pairs[i].second);
		}
		w16(0x00, n >= 2 && n < 16 ? 0 : n);
		break;
	}
	case 0x03: {  // scale a vector: 2 x s, 2 y s
		int64_t x = r16(0x00), y = r16(0x02), s = r16(0x04);
		w32(0x10, uint32_t(2 * x * s));
		w32(0x14, uint32_t(2 * y * s));
		break;
	}
	case 0x04: {  // the length of a vector, estimated
		int16_t x = r16(0x00), y = r16(0x02);
		uint16_t ax = magnitude(x), ay = magnitude(y);
		w16(0x00, std::max(ax, ay));  // the larger and the smaller magnitude are left behind
		w16(0x02, std::min(ax, ay));
		w16(0x10, distance(x, y));
		break;
	}
	case 0x05:  // "simulated driver coordinate calculation": not measured (F1 ROC II doesn't use it)
		break;
	case 0x06:  // multiply: 2 a b
		w32(0x10, uint32_t(2 * int64_t(r16(0x00)) * int64_t(r16(0x02))));
		break;
	case 0x07: {  // the road's per-line rotation and scale, four tables of 176 lines
		uint8_t step = uint8_t(uint16_t(r16(0x00)) >> 8);
		int32_t s = sine(step), c = cosine(step);
		w16(0x00, step);
		w16(0x02, uint16_t(s));
		w16(0x04, uint16_t(c));
		for(int n = 0; n < 176; n++) {
			int64_t p = perspective(n);
			w16(0x0f0 + 2 * n, uint16_t(shift15(p * c)));
			w16(0x250 + 2 * n, uint16_t(shift15(p * s)));
			w16(0x3b0 + 2 * n, uint16_t(shift15(-p * s)));
			w16(0x510 + 2 * n, uint16_t(shift15(p * c)));
		}
		break;
	}
	case 0x08: {  // rotate a vector
		int64_t x = r16(0x00), y = r16(0x02);
		uint8_t step = uint8_t(uint16_t(r16(0x04)) >> 8);
		int64_t s = sine(step), c = cosine(step);
		w16(0x10, uint16_t(shift15(x * c) + shift15(y * s)));
		w16(0x12, uint16_t(shift15(y * c) - shift15(x * s)));
		break;
	}
	}
	m_ram[0x20] = 0;
	m_ram[0x21] &= 0x7f;
}

std::vector<uint8_t> St010::saveState() const {
	std::vector<uint8_t> s = {'S', 'T', '1', 1};
	s.insert(s.end(), m_ram.begin(), m_ram.end());
	return s;
}

bool St010::loadState(const uint8_t* data, size_t size) {
	if(size != 4 + RamSize || data[0] != 'S' || data[1] != 'T' || data[2] != '1' || data[3] != 1) return false;
	std::copy(data + 4, data + 4 + RamSize, m_ram.begin());
	return true;
}

} // namespace st010
