#include <iostream>
#include <variant>

#include "instrs.hpp"
#include "expected.hpp"

namespace jit {
	std::ostream& operator<<(std::ostream& os, const Reg& reg) {
		os << "Reg(" << reg._reg << ")";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Value& val) {
		os << "Val(";
		switch (val._kind)
		{
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

		case InstrKind::IK_LABEL: {
			auto& lab = (LabelInstr&)instr;
			os << lab._name;
			break;
		}
		default: {
			break;
		}
		}
		return os;
	}
}