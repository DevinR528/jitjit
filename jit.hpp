#pragma once

#include <map>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <iostream>
#include <utility>

#include <Windows.h>
#include <Memoryapi.h>

#include "expected.hpp"
#include "instrs.hpp"
#include "interp.hpp"

namespace jit {

typedef uint64_t (*JitCall)(Value* registers, uint64_t* locals);

enum class MCReg {
	RAX = 0,
	RCX = 1,
	RDX = 2,
	RBX = 3,
	RSP = 4,
	RBP = 5,
	RSI = 6,
	RDI = 7,
	R8 = 8,
	R9 = 9,
	R10 = 10,
	R11 = 11,
	R12 = 12,
	R13 = 13,
	R14 = 14,
	R15 = 15,
};

struct Jit {
	
	const std::string& _blk_name;
	const std::vector<Instruction*>& _instrs;
	uint8_t* _code = nullptr;
	uint32_t _code_idx = 0;

	Jit(const std::string& name, const std::vector<Instruction*>& instrs)
		: _blk_name(name), _instrs(instrs) {}

	~Jit() {
		if (_code) {
			if (VirtualFree((LPVOID)_code, 0, MEM_RELEASE) == 0) {
				std::cout << "FAILED TO FREE CODE PAGE\n";
			}
		}
	}

	[[nodiscard]]
	tl::expected<void, Error> set_exec_mem() {
		LPVOID res =
			VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!res) {
			return tl::make_unexpected(Error(ErrorKind::EK_OOM, "mmap failed"));
		}
		_code = (uint8_t*)res;
		return tl::expected<void, Error>();
	}

	void write_byte(uint8_t b) {
		_code[_code_idx] = b;
		_code_idx += 1;
	}

	void write_dword(uint32_t val) {
		for (size_t i = 0; i < (4 * 8); i += 8) {
			write_byte((val >> i) & 0xff);
		}
	}

	void write_qword(uint64_t val) {
		for (size_t i = 0; i < (8 * 8); i += 8) {
			write_byte((val >> i) & 0xff);
		}
	}

	void write_load_jsval(const Reg& from, const MCReg& to);
	void write_load_imm(uint64_t from, const MCReg& to);
	void write_store_jsval(const MCReg& from, const Reg& to);
	void write_mov(const MCReg& from, const MCReg& to);
	void write_mult(const MCReg& from, const MCReg& to);
	void write_add(const MCReg& from, const MCReg& to);
	void write_addimm(uint32_t from, const MCReg& to);
	void write_subimm(uint32_t from, const MCReg& to);
	void write_cmplt(const MCReg& from, const MCReg& to);
	void write_jmp(const InstrKind& kind);

	tl::expected<void, Error> compile();

	uint64_t execute(Value* registers, uint64_t* globals) {
		auto val = ((JitCall)_code)(registers, globals);
		return val;
	}
};

}