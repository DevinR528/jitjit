#include <map>
#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <utility>

#include "expected.hpp"
#include "instrs.hpp"
#include "jit.hpp"

namespace jit {

static constexpr uint8_t encode(const MCReg& r) { return std::to_underlying(r) & 0x7; }

void iwrite_call(int64_t x) { std::cout << "yo: " << x << "\n"; }

// This is a register to register move.
//
// mov dst, src
void Jit::write_mov(const MCReg& src, const MCReg& dst) {
	write_byte(0x48 | (std::to_underlying(src) >= 8 ? 1 << 2 : 0) |
			   (std::to_underlying(dst) >= 8 ? 1 << 0 : 0));
	write_byte(0x89);
	write_byte(0xc0 | (encode(src) << 3) | encode(dst));
}

// This is a memory to register move.
//
// `mov reg, [reg+offset]`
void Jit::write_load_jsval(const Reg& from, const MCReg& to) {
	write_byte(0x48
			   // This is dst
			   | (std::to_underlying(to) >= 8 ? 1 << 2 : 0)
			   // This is src
			   | (std::to_underlying(MCReg::RCX) >= 8 ? 1 << 0 : 0));
	// This is a MOV
	write_byte(0x8b);
	// The ModR/M byte for MOV is 0b10rrrmmm where rrr is dst and mmm is src
	write_byte(0x80 | (encode(to) << 3) | encode(MCReg::RCX));
	// Offset baybeh
	write_dword((from._reg * sizeof(Value)) + 8);
}

// This is a immediate to register move.
//
// `mov reg, 0x1234`
void Jit::write_load_imm(uint64_t from, const MCReg& to) {
	write_byte(0x48 | (std::to_underlying(to) >= 8 ? 1 << 2 : 0));
	write_byte(0xb8 | encode(to));
	write_qword(from);
}

// This is a register to memory move.
//
// `mov [reg+offset], reg`
void Jit::write_store_jsval(const MCReg& from, const Reg& to) {
	// mov rcx,qword ptr [rax+0A0h] 
	write_byte(0x48 | (std::to_underlying(from) >= 8 ? 1 << 2 : 0)
					| (std::to_underlying(MCReg::RCX) >= 8 ? 1 << 0 : 0));
	write_byte(0x89);
	write_byte(0x80 | (encode(from) << 3) | encode(MCReg::RCX));
	write_dword(to._reg * sizeof(Value) + 8);
}

// Add src and dst collecting into dst.
//
// add dst, src
void Jit::write_add(const MCReg& src, const MCReg& dst) {
	write_byte(0x48 | (std::to_underlying(src) >= 8 ? 1 << 2 : 0)
					| (std::to_underlying(dst) >= 8 ? 1 << 0 : 0));
	write_byte(0x01);
	write_byte(0xc0 | (encode(src) << 3) | encode(dst));
}

// Add imm and dst collecting into dst.
//
// add dst, imm
void Jit::write_addimm(uint32_t src, const MCReg& dst) {
	write_byte(0x48 | (std::to_underlying(dst) >= 8 ? 1 << 0 : 0));
	write_byte(0x81);
	write_byte(0xc0 | encode(dst));
	write_dword(src);
}

// Sub imm and dst collecting into dst.
//
// sub dst, imm
void Jit::write_subimm(uint32_t src, const MCReg& dst) {
	write_byte(0x48 | (std::to_underlying(dst) >= 8 ? 1 << 0 : 0));
	write_byte(0x81);
	write_byte(0xe8 | encode(dst));
	write_dword(src);
}

// Mult src and dst collecting into dst.
//
// mult dst, src
void Jit::write_mult(const MCReg& src, const MCReg& dst) {
	write_byte(0x48 | (std::to_underlying(src) >= 8 ? 1 << 2 : 0) |
			   (std::to_underlying(dst) >= 8 ? 1 << 0 : 0));
	write_byte(0x0f);
	write_byte(0xaf);
	write_byte(0xc0 | (encode(src) << 3) | encode(dst));
}

// Compare src and dst collecting into dst.
//
// cmp dst, src
void Jit::write_cmplt(const MCReg& lhs, const MCReg& rhs) {
	// cmp
	write_byte(0x48 | (std::to_underlying(lhs) >= 8 ? 1 << 2 : 0)
					| (std::to_underlying(rhs) >= 8 ? 1 << 0 : 0));
	write_byte(0x39);
	write_byte(0xc0 | (encode(lhs) << 3) | encode(rhs));

	// setl

	//write_byte(0x48 | (std::to_underlying(rhs) >= 8 ? 1 << 0 : 0));
	//write_byte(0x0f);
	//write_byte(0xaf);
	//write_byte(0xc0 | encode(rhs));
}

void Jit::write_jmp(const InstrKind& kind) {
	write_byte(0x0f);
	uint8_t jmp_byte = 0;
	if (kind == InstrKind::IK_CMP_LT) {
		jmp_byte = 0x8f;
	} else if (kind == InstrKind::IK_CMP_LE) {
		jmp_byte = 0x8e;
	} else if (kind == InstrKind::IK_CMP_GT) {
		jmp_byte = 0x8f;
	} else if (kind == InstrKind::IK_CMP_GE) {
		jmp_byte = 0x8d;
	}
	write_byte(jmp_byte);
	write_dword(-((int64_t)_code_idx + 4 - 11));
}

tl::expected<void, Error> Jit::compile() {
	if (!_code) {
		if (!set_exec_mem().has_value()) {
			std::cout << "Failed to set page\n";
		}
	}

	// Push base pointer
	write_byte(0x50 | encode(MCReg::RBP));
	// Do preamble stuff
	//write_mov(MCReg::RBP, MCReg::RSP);
	// 
	// push rdi
	write_byte(0x50 | encode(MCReg::RDI));
	write_subimm(32, MCReg::RSP);

	InstrKind jump_kind;
	for (auto& inst : _instrs) {
		switch (inst->_kind) {
			case InstrKind::IK_I2I: {
				auto mov = (I2IInstr*)inst;
				write_load_jsval(mov->_src, MCReg::R8);
				write_store_jsval(MCReg::R8, mov->_dst);
				break;
			}

			case InstrKind::IK_MULT: {
				auto mult = (MultInstr*)inst;
				write_load_jsval(mult->_src1, MCReg::R8);
				write_load_jsval(mult->_src2, MCReg::RAX);
				write_mult(MCReg::R8, MCReg::RAX);
				write_store_jsval(MCReg::RAX, mult->_dst);
				break;
			}
			case InstrKind::IK_ADD: {
				auto add = (AddInstr*)inst;
				write_load_jsval(add->_src1, MCReg::R8);
				write_load_jsval(add->_src2, MCReg::RAX);
				write_add(MCReg::R8, MCReg::RAX);
				write_store_jsval(MCReg::RAX, add->_dst);
				break;
			}
			case InstrKind::IK_ADDIMM: {
				auto add = (AddImmInstr*)inst;
				write_load_jsval(add->_src1, MCReg::RAX);
				write_addimm((uint32_t)add->_src2.to_bytes(), MCReg::RAX);
				write_store_jsval(MCReg::RAX, add->_dst);
				break;
			}

			case InstrKind::IK_CMP_LT: {
				jump_kind = InstrKind::IK_CMP_LT;
				auto cmp = (CmpLTInstr*)inst;
				write_load_jsval(cmp->_src1, MCReg::R8);
				write_load_jsval(cmp->_src2, MCReg::RAX);
				write_cmplt(MCReg::R8, MCReg::RAX);
				// TODO: make this work
				//write_store_jsval(MCReg::RAX, cmp->_dst);
				break;
			}
			case InstrKind::IK_CBR: {
				auto cbr = (CbrInstr*)inst;
				write_jmp(jump_kind);
				break;
			}

			case InstrKind::IK_IWRITE: {
				auto wrt = (IWriteInstr*)inst;
				
				// Push rcx and rdx
				write_byte(0x50 | encode(MCReg::RDX));
				write_byte(0x50 | encode(MCReg::RCX));

				write_subimm(16, MCReg::RSP);

				write_load_jsval(wrt->_src, MCReg::RCX);

				// Load call address into rax
				write_load_imm((intptr_t)iwrite_call, MCReg::RAX);

				// Call rax
				write_byte(0xff);
				write_byte(0xd0);

				write_addimm(16, MCReg::RSP);

				// Pop rcx and rdx back to registers
				write_byte(0x58 | encode(MCReg::RCX));
				write_byte(0x58 | encode(MCReg::RDX));
				break;
			}

			default: {
				std::cout << "INVALID INSTR" << *inst << "\n";
				return tl::make_unexpected(
					Error(ErrorKind::EK_INVALID_INST, "invalid instruction"));
			}
		}
	}
	
	// After the conditional jmp what was a fall through is now
	// RET. We know this is safe since the only way to jit is
	// a backedge (loop) since they must run more than 1
	//
	// leave
	//write_byte(0xc9);
	// Undo the sub rsp, 8 to set up stack
	write_addimm(32, MCReg::RSP);
	// Pop rdi
	write_byte(0x58 | encode(MCReg::RDI));
	// pop base pointer
	write_byte(0x58 | encode(MCReg::RBP));
	// ret
	write_byte(0xc3);

	return tl::expected<void, Error>();
}

}