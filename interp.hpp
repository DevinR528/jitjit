#pragma once

#include <map>
#include <string>
#include <vector>
#include <optional>

#include "expected.hpp"
#include "instrs.hpp"

namespace jit {
#define TRY_OR_BAIL(expr)                                                                \
	({                                                                                   \
		auto&& x = expr;                                                                 \
		if (!x) {                                                                        \
			return tl::make_unexpected(x.error());                                       \
		}                                                                                \
		std::move(*x);                                                                   \
	})

template <typename T> T take(std::optional<T>& opt) {
	T val = std::move(*opt);
	opt.reset();
	return val;
}

enum ErrorKind {
	EK_CAST,
	EK_INVALID_REG,
	EK_INVALID_INST,
	EK_OOM,

	EK_SIZE
};

struct Error {
	static constexpr const char* ERR[ErrorKind::EK_SIZE] = {
		"EK_CAST", "EK_INVALID_REG", "EK_INVALID_INST", "EK_OOM"
	};
	ErrorKind _kind;
	std::string _msg;

	Error(ErrorKind k, std::string msg) : _kind(k), _msg(msg) {}

	friend std::ostream& operator<<(std::ostream& os, const Error& instr);
};

struct Registers {
	std::map<Reg, Value*> _reg_map;
	std::vector<uint64_t> _reg_flat;

	std::pair<std::map<Reg, Value*>::iterator, bool> insert_or_assign(Reg r, Value* v) {
		if (_reg_flat.size() <= r._reg) {
			_reg_flat.resize(r._reg + 1);
		}
		_reg_flat[r._reg] = v->to_bytes();
		return _reg_map.insert_or_assign(r, v);
	}
};

struct CallInfo {};

struct Block {
	std::string _name;
	std::string _fallthrough;
	uint32_t _exec_count;
	std::vector<Instruction*> _instrs;

	Block(std::string n) : _name(n), _exec_count(0) {}
};

struct Function {
	std::string _name;
	uint32_t _size;
	std::vector<Reg> _args;
	std::map<std::string, Block> _blocks;

	Function(std::string n, uint32_t s, std::vector<Reg> a)
		: _name(n), _size(s), _args(a) {}
};

struct Parser {
	bool _in_data_section = false;
	std::map<std::string, Value*> _globals;
	std::map<std::string, Function> _funcs;

	std::optional<Function> _curr_func;
	std::optional<Block> _curr_block;

	void start_data(Instruction* data) { _in_data_section = true; }
	void start_text(Instruction* text) { _in_data_section = false; }

	void push_frame(FrameInstr* inst);
	void push_label(LabelInstr* inst);
	void push_instr(Instruction* inst);

	void finalize_prog();
};

struct Interpreter {
	std::map<std::string, Value*> _globals;
	Registers _registers;
	std::vector<std::vector<Value*>> _stack;

	std::map<std::string, Function> _funcs;
	std::vector<CallInfo> _call_stack;

	std::string _func_name;
	std::string _block_name;
	uint32_t _inst_idx;

	Interpreter(
		std::map<std::string, Function> funcs, std::map<std::string, Value*> globals)
		: _globals(globals), _funcs(funcs), _func_name("main"), _block_name("__start__"),
		  _inst_idx(0) {
		_stack.push_back(std::vector<Value*>());
	}

	// TODO: tl::expected<Function&, Error> for all these
	Function& get_func() { return _funcs.find(_func_name)->second; }

	Block& get_block() { return get_func()._blocks.find(_block_name)->second; }

	Instruction* get_inst() { return get_block()._instrs[_inst_idx]; }

	tl::expected<Value*, Error> get_register_value(const Reg& reg) {
		auto x = _registers._reg_map.find(reg);
		if (x == _registers._reg_map.end()) {
			return tl::make_unexpected(
				Error(ErrorKind::EK_INVALID_REG, "missing register"));
		}
		return x->second;
	}


	tl::expected<void, Error> run();
};
}