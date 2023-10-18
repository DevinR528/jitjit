#pragma once

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include <variant>

#include "expected.hpp"

namespace jit {
	enum ValKind {
		// Integer value
		VK_INT,
		// String from .data
		VK_STRING,
		// Floating point number
		VK_FLOAT,
		// A label or function name
		VK_LOCATION,

		// Empty value
		VK_NULL
	};
	struct Value {
		ValKind _kind;

		Value(ValKind k) : _kind(k) {}

		friend std::ostream& operator<<(std::ostream& os, const Value& instr);
	};
	struct IntVal : Value {
		int32_t _val;

		IntVal() : Value(ValKind::VK_INT), _val(0) {}
		IntVal(int i) : Value(ValKind::VK_INT), _val(i) {}
	};
	struct StrVal : Value {
		std::string _val;

		StrVal(std::string s) : Value(ValKind::VK_STRING), _val(s) {}
	};
	struct FloatVal : Value {
		float _val;

		FloatVal(float f) : Value(ValKind::VK_FLOAT), _val(f) {}
	};
	struct LocVal : Value {
		std::string _val;

		LocVal(std::string f) : Value(ValKind::VK_LOCATION), _val(f) {}
	};

	struct Reg {
		uint32_t _reg;
		Reg(uint32_t r) : _reg(r) {}
		friend std::ostream& operator<<(std::ostream& os, const Reg& reg);
	};

	enum class InstrKind {
		// The .data section
		IK_DATA,
		// The .text section
		IK_TEXT,
		// Global value
		IK_GLOBAL,
		// The start of a function
		IK_FRAME,
		// Integer to integer move
		IK_I2I,
		// Load immediate
		IK_LOADIMM,
		// Integer write
		IK_IWRITE,
		// Conditional branch when true
		IK_CBR,

		// Compare, true if greater than
		IK_CMP_GT,
		// Compare, true if less than
		IK_CMP_LT,

		// Add two registers contents
		IK_ADD,
		// Multiply two registers contents
		IK_MULT,

		// Return to the last frame
		IK_RET,
		// Label
		IK_LABEL,

		// A no-op instruction
		IK_NOP,
	};
	struct Instruction {
		InstrKind _kind;

		Instruction(InstrKind k) : _kind(k) {}

		friend std::ostream& operator<<(std::ostream& os, const Instruction& instr);
	};
	struct DataInstr : Instruction {
		DataInstr() : Instruction(InstrKind::IK_DATA) {}
	};
	struct TextInstr : Instruction {
		TextInstr() : Instruction(InstrKind::IK_TEXT) {}
	};
	struct GlobalInstr : Instruction {
		Value _value;

		GlobalInstr(Value val) : Instruction(InstrKind::IK_GLOBAL), _value(val) {}
	};
	struct FrameInstr : Instruction {
		std::string _name;
		uint32_t _size;
		std::vector<Reg> _params;

		FrameInstr(std::string name, uint32_t size, std::vector<Reg> params) : Instruction(InstrKind::IK_FRAME), _name(name), _size(size), _params(params) {}
	};
	struct LabelInstr : Instruction {
		std::string _name;

		LabelInstr(std::string val) : Instruction(InstrKind::IK_LABEL), _name(val) {}
	};
	struct I2IInstr : Instruction {
		Reg _src;
		Reg _dst;

		I2IInstr(Reg src, Reg dst) : Instruction(InstrKind::IK_I2I), _src(src), _dst(dst) {}
	};
	struct LoadImmInstr : Instruction {
		Value* _src;
		Reg _dst;

		LoadImmInstr(Value* src, Reg dst) : Instruction(InstrKind::IK_LOADIMM), _src(src), _dst(dst) {}
	};
}