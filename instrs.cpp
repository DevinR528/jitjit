#include <iostream>
#include <variant>
#include <cassert>

#include "instrs.hpp"
#include "expected.hpp"

namespace jit {
uint64_t Value::to_bytes() {
	switch (_kind) {
		case ValKind::VK_INT: {
			return as_int();
		}
		case ValKind::VK_FLOAT: {
			uint64_t fbits = 0;
			float f = as_float();
			memcpy(&fbits, &f, sizeof(fbits));
			return fbits;
		}
		case ValKind::VK_STRING: {
			assert(((void)"do something about string", false));
			return (uintptr_t)as_string().data();
		}
		case ValKind::VK_LOCATION: {
			assert(((void)"do something about location", false));
			return (uintptr_t)as_loc().data();
		}
		default: {
			return 0;
		}
	}
}
Value Value::cmp_ge(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR cmp_ge\n";
	}
	return Value((int64_t)(as_int() >= b.as_int()));
}
Value Value::cmp_gt(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR cmp_gt\n";
	}
	return Value((int64_t)(as_int() > b.as_int()));
}
Value Value::cmp_le(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR cmp_le\n";
	}
	return Value((int64_t)(as_int() <= b.as_int()));
}
Value Value::cmp_lt(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR cmp_lt\n";
	}
	return Value((int64_t)(as_int() < b.as_int()));
}

Value Value::mult(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR mult\n";
	}
	return Value(as_int() * b.as_int());
}

Value Value::add(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR add\n";
	}
	return Value(as_int() + b.as_int());
}

std::ostream& operator<<(std::ostream& os, const Reg& reg) {
	os << "Reg(" << reg._reg << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Value& val) {
	os << "Val(";
	switch (val._kind) {
		case ValKind::VK_INT: {
			os << val.as_int();
			break;
		}
		case ValKind::VK_FLOAT: {
			os << val.as_float();
			break;
		}
		case ValKind::VK_STRING: {
			os << val.as_string();
			break;
		}
		case ValKind::VK_LOCATION: {
			os << val.as_string();
			break;
		}
		default: {
			break;
		}
	}
	os << ")";

	return os;
}

std::ostream& operator<<(std::ostream& os, const Instruction& instr) {
	switch (instr._kind) {
		case InstrKind::IK_DATA: {
			os << ".data";
			break;
		}
		case InstrKind::IK_TEXT: {
			os << ".text";
			break;
		}
		case InstrKind::IK_FRAME: {
			auto& frame = (FrameInstr&)instr;
			os << frame._name << " " << frame._size;
			for (auto&& p : frame._params) {
				os << " " << p;
			}
			break;
		}
		case InstrKind::IK_I2I: {
			auto& mov = (I2IInstr&)instr;
			os << "mov " << mov._src << " -> " << mov._dst;
			break;
		}
		case InstrKind::IK_LOADIMM: {
			auto& load = (LoadImmInstr&)instr;
			os << "load " << load._src << " -> " << load._dst;
			break;
		}
		case InstrKind::IK_ADDIMM: {
			auto& add = (AddImmInstr&)instr;
			os << "addI " << add._src1 << " + " << add._src2 << " -> " << add._dst;
			break;
		}
		case InstrKind::IK_ADD: {
			auto& add = (AddInstr&)instr;
			os << "add " << add._src1 << " + " << add._src2 << " -> " << add._dst;
			break;
		}
		case InstrKind::IK_MULTIMM: {
			auto& mult = (MultImmInstr&)instr;
			os << "multI " << mult._src1 << " * " << mult._src2 << " -> " << mult._dst;
			break;
		}
		case InstrKind::IK_MULT: {
			auto& mult = (MultInstr&)instr;
			os << "mult " << mult._src1 << " * " << mult._src2 << " -> " << mult._dst;
			break;
		}

		case InstrKind::IK_CMP_GT: {
			auto& cmp = (CmpGTInstr&)instr;
			os << "cmp_GT " << cmp._src1 << " > " << cmp._src2 << " -> " << cmp._dst;
			break;
		}
		case InstrKind::IK_CMP_GE: {
			auto& cmp = (CmpGEInstr&)instr;
			os << "cmp_GE " << cmp._src1 << " >= " << cmp._src2 << " -> " << cmp._dst;
			break;
		}
		case InstrKind::IK_CMP_LT: {
			auto& cmp = (CmpLTInstr&)instr;
			os << "cmp_LT " << cmp._src1 << " < " << cmp._src2 << " -> " << cmp._dst;
			break;
		}
		case InstrKind::IK_CMP_LE: {
			auto& cmp = (CmpLEInstr&)instr;
			os << "cmp_LE " << cmp._src1 << " <= " << cmp._src2 << " -> " << cmp._dst;
			break;
		}
		case InstrKind::IK_CBR: {
			auto& cbr = (CbrInstr&)instr;
			os << "cbr " << cbr._src << " --> " << cbr._dst;
			break;
		}

		case InstrKind::IK_IWRITE: {
			auto& w = (IWriteInstr&)instr;
			os << "write " << w._src;
			break;
		}

		case InstrKind::IK_RET: {
			os << "ret";
			break;
		}

		case InstrKind::IK_LABEL: {
			auto& lab = (LabelInstr&)instr;
			os << lab._name;
			break;
		}

		default: {
			os << "INVALID INSTRUCTION "
			   << "\n";
			break;
		}
	}
	return os;
}
}