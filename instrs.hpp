#pragma once

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include <variant>
#include <optional>

#include "expected.hpp"

namespace jit {
struct Empty {};

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
	std::variant<Empty, int64_t, float, std::string> _val;

	Value() : _kind(ValKind::VK_NULL), _val(Empty{}) {}
	Value(int64_t i) : _kind(ValKind::VK_INT), _val(i) {}
	Value(float f) : _kind(ValKind::VK_FLOAT), _val(f) {}
	Value(ValKind k, std::string s) : _kind(k), _val(s) {}

	int64_t as_int() const& { return std::get<int64_t>(_val); }
	float as_float() const& { return std::get<float>(_val); }
	std::string_view as_string() const& { return std::get<std::string>(_val); }
	std::string_view as_loc() const& { return std::get<std::string>(_val); }

	Value cmp_gt(Value& b);
	Value cmp_ge(Value& b);
	Value cmp_lt(Value& b);
	Value cmp_le(Value& b);
	Value add(Value& b);
	Value mult(Value& b);

	uint64_t to_bytes();

	friend std::ostream& operator<<(std::ostream& os, const Value& instr);
};

struct Reg {
	uint32_t _reg;
	Reg(uint32_t r) : _reg(r) {}

	bool constexpr operator<(const Reg& b) const { return _reg < b._reg; }

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
	// Compare, true if greater than or equal
	IK_CMP_GE,
	// Compare, true if less than
	IK_CMP_LT,
	// Compare, true if less than or equal
	IK_CMP_LE,

	// Add two registers contents
	IK_ADD,
	// Add register and literal contents
	IK_ADDIMM,
	// Multiply two registers contents
	IK_MULT,
	// Multiply register and literal contents
	IK_MULTIMM,

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
	std::string _name;
	Value _value;

	GlobalInstr(std::string n, Value val) : Instruction(InstrKind::IK_GLOBAL), _name(n), _value(val) {}
};
struct FrameInstr : Instruction {
	std::string _name;
	uint32_t _size;
	std::vector<Reg> _params;

	FrameInstr(std::string name, uint32_t size, std::vector<Reg> params)
		: Instruction(InstrKind::IK_FRAME), _name(name), _size(size), _params(params) {}
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
	Value _src;
	Reg _dst;

	LoadImmInstr(Value src, Reg dst)
		: Instruction(InstrKind::IK_LOADIMM), _src(src), _dst(dst) {}
};

struct AddInstr : Instruction {
	Reg _src1;
	Reg _src2;
	Reg _dst;

	AddInstr(Reg src1, Reg src2, Reg dst)
		: Instruction(InstrKind::IK_ADD), _src1(src1), _src2(src2), _dst(dst) {}
};
struct AddImmInstr : Instruction {
	Reg _src1;
	Value _src2;
	Reg _dst;

	AddImmInstr(Reg src1, Value src2, Reg dst)
		: Instruction(InstrKind::IK_ADDIMM), _src1(src1), _src2(src2), _dst(dst) {}
};
struct MultInstr : Instruction {
	Reg _src1;
	Reg _src2;
	Reg _dst;

	MultInstr(Reg src1, Reg src2, Reg dst)
		: Instruction(InstrKind::IK_MULT), _src1(src1), _src2(src2), _dst(dst) {}
};
struct MultImmInstr : Instruction {
	Reg _src1;
	Value _src2;
	Reg _dst;

	MultImmInstr(Reg src1, Value src2, Reg dst)
		: Instruction(InstrKind::IK_MULTIMM), _src1(src1), _src2(src2), _dst(dst) {}
};

struct CmpGTInstr : Instruction {
	Reg _src1;
	Reg _src2;
	Reg _dst;

	CmpGTInstr(Reg src1, Reg src2, Reg dst)
		: Instruction(InstrKind::IK_CMP_GT), _src1(src1), _src2(src2), _dst(dst) {}
};
struct CmpGEInstr : Instruction {
	Reg _src1;
	Reg _src2;
	Reg _dst;

	CmpGEInstr(Reg src1, Reg src2, Reg dst)
		: Instruction(InstrKind::IK_CMP_GE), _src1(src1), _src2(src2), _dst(dst) {}
};
struct CmpLTInstr : Instruction {
	Reg _src1;
	Reg _src2;
	Reg _dst;

	CmpLTInstr(Reg src1, Reg src2, Reg dst)
		: Instruction(InstrKind::IK_CMP_LT), _src1(src1), _src2(src2), _dst(dst) {}
};
struct CmpLEInstr : Instruction {
	Reg _src1;
	Reg _src2;
	Reg _dst;

	CmpLEInstr(Reg src1, Reg src2, Reg dst)
		: Instruction(InstrKind::IK_CMP_LE), _src1(src1), _src2(src2), _dst(dst) {}
};

struct CbrInstr : Instruction {
	Reg _src;
	Value _dst;

	CbrInstr(Reg src, Value loc) : Instruction(InstrKind::IK_CBR), _src(src), _dst(loc) {}
};

struct IWriteInstr : Instruction {
	Reg _src;

	IWriteInstr(Reg src)
		: Instruction(InstrKind::IK_IWRITE), _src(src) {}
};

struct RetInstr : Instruction {
	RetInstr() : Instruction(InstrKind::IK_RET) {}
};

}