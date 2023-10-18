#pragma once

#include <map>
#include <string>
#include <vector>

#include "expected.hpp"
#include "instrs.hpp"

namespace jit {
	struct Error {};

	struct CallInfo {};

	struct Interpreter {
		std::map<std::string, Value> _values;
		std::vector<Value> _stack;
		std::vector<Instruction*> _instrs;
		std::vector<CallInfo> _call_stack;

		uint32_t _instr_idx;

		Interpreter(std::vector<Instruction*> instrs) : _instrs(instrs), _instr_idx(0) {}

		tl::expected<void, Error> run();
	};
}