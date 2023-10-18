#include <iostream>

#include "interp.hpp"
#include "expected.hpp"

namespace jit {

	tl::expected<void, Error> Interpreter::run() {
		for (auto s : _instrs) {
			std::cout << *s << "\n";
		}
		auto res = tl::expected<void, Error>();
		return res;
	}

}