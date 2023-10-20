#include <iostream>
#include <variant>
#include <cassert>

#include "instrs.hpp"
#include "expected.hpp"

namespace jit {
uint64_t Value::to_bytes() {
	switch (_kind) {
		case ValKind::VK_INT: {
			auto v = (IntVal*)this;
			return v->_val;
		}
		case ValKind::VK_FLOAT: {
			auto v = (FloatVal*)this;
			uint64_t fbits = 0;
			memcpy(&fbits, &v->_val, sizeof(fbits));
			return fbits;
		}
		case ValKind::VK_STRING: {
			auto v = (StrVal*)this;
			assert(((void)"do something about string", false));
			return (uintptr_t)v->_val.data();
		}
		default: {
			return 0;
		}
	}
}
Value* Value::cmp_ge(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR cmp_ge\n";
	}
	IntVal* self = (IntVal*)this;
	IntVal& other = (IntVal&)b;
	return new IntVal(self->_val >= other._val);
}
Value* Value::cmp_gt(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR cmp_gt\n";
	}
	IntVal* self = (IntVal*)this;
	IntVal& other = (IntVal&)b;
	return new IntVal(self->_val > other._val);
}
Value* Value::cmp_le(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR cmp_le\n";
	}
	IntVal* self = (IntVal*)this;
	IntVal& other = (IntVal&)b;
	return new IntVal(self->_val <= other._val);
}
Value* Value::cmp_lt(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR cmp_lt\n";
	}
	IntVal* self = (IntVal*)this;
	IntVal& other = (IntVal&)b;
	return new IntVal(self->_val < other._val);
}

Value* Value::mult(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR mult\n";
	}
	IntVal* self = (IntVal*)this;
	IntVal& other = (IntVal&)b;
	return new IntVal(self->_val * other._val);
}

Value* Value::add(Value& b) {
	// TODO: this should tl::expected<Value*, Error>...
	if (_kind != ValKind::VK_INT || b._kind != ValKind::VK_INT) {
		std::cout << "NOT INT FOR add\n";
	}
	IntVal* self = (IntVal*)this;
	IntVal& other = (IntVal&)b;
	return new IntVal(self->_val + other._val);
}

std::ostream& operator<<(std::ostream& os, const Reg& reg) {
	os << "Reg(" << reg._reg << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const Value& val) {
	os << "Val(";
	switch (val._kind) {
		case ValKind::VK_INT: {
			auto& i = (IntVal&)val;
			os << i._val;
			break;
		}
		case ValKind::VK_STRING: {
			auto& i = (StrVal&)val;
			os << i._val;
			break;
		}
		case ValKind::VK_FLOAT: {
			auto& i = (FloatVal&)val;
			os << i._val;
			break;
		}
		case ValKind::VK_LOCATION: {
			auto& i = (LocVal&)val;
			os << i._val;
			break;
		}
		default:
			break;
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
			os << "load " << *load._src << " -> " << load._dst;
			break;
		}
		case InstrKind::IK_ADDIMM: {
			auto& add = (AddImmInstr&)instr;
			os << "addI " << add._src1 << " + " << *add._src2 << " -> " << add._dst;
			break;
		}
		case InstrKind::IK_ADD: {
			auto& add = (AddInstr&)instr;
			os << "add " << add._src1 << " + " << add._src2 << " -> " << add._dst;
			break;
		}
		case InstrKind::IK_MULTIMM: {
			auto& mult = (MultImmInstr&)instr;
			os << "multI " << mult._src1 << " * " << *mult._src2 << " -> " << mult._dst;
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
			os << "cbr " << cbr._src << " --> " << *cbr._dst;
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