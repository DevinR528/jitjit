#include <iostream>

#include "interp.hpp"
#include "expected.hpp"
#include "jit.hpp"

namespace jit {
std::ostream& operator<<(std::ostream& os, const Error& err) {
	os << Error::ERR[err._kind] << ": " << err._msg;
	return os;
}

void Parser::push_frame(FrameInstr* frame) {
	if (_curr_func.has_value()) {
		// TODO: check if func or block already named here

		// Add block to current function then add function to "program"
		auto blk = take(_curr_block);
		_curr_func->_blocks.insert(std::pair{blk._name, blk});
		auto func = take(_curr_func);
		_funcs.insert(std::pair{func._name, func});
	}
	_curr_func = Function(frame->_name, frame->_size, frame->_params);
	_curr_block = Block("__start__");
}

void Parser::push_label(LabelInstr* label) {
	if (_curr_func.has_value()) {
		// TODO: check if block already named here

		// Add block to current function and make new block
		auto blk = take(_curr_block);
		blk._fallthrough = label->_name;
		_curr_func->_blocks.insert(std::pair{blk._name, blk});
	} else {
		std::cout << "NO FUNC TO PUSH TO...\n";
	}
	_curr_block = Block(label->_name);
}

void Parser::push_instr(Instruction* inst) {
	if (_in_data_section) {
		auto& glb = (GlobalInstr&)inst;
		_globals.insert(std::pair{glb._name, glb._value});
	} else if (_curr_block.has_value()) {
		_curr_block->_instrs.push_back(inst);
	} else {
		std::cout << "NO BLOCK TO PUSH TO...\n";
	}
}

void Parser::finalize_prog() {
	if (_curr_func.has_value() && _curr_block.has_value()) {
		// TODO: check if block already named here
		auto blk = take(_curr_block);
		_curr_func->_blocks.insert(std::pair{blk._name, blk});
		auto func = take(_curr_func);
		_funcs.insert(std::pair{func._name, func});
	} else {
		std::cout << "NO FUNC OR BLOCK TO PUSH TO...\n";
	}
}

tl::expected<void, Error> Interpreter::run() {
	auto ok_result = tl::expected<void, Error>();

	for (auto&& p : _funcs) {
		auto&& [fname, func] = p;
		std::cout << "func " << fname << "{\n";
		for (auto&& pair : func._blocks) {
			auto&& [bname, blk] = pair;
			std::cout << "  blk " << bname << "{\n";
			for (auto&& inst : blk._instrs) {
				std::cout << "    " << *inst << "\n";
			}
			std::cout << "  }\n";
		}
		std::cout << "}\n";
	}

	auto inst = get_inst();
	while (true) {
		_inst_idx += 1;

		switch (inst->_kind) {
			case InstrKind::IK_LOADIMM: {
				auto load = (LoadImmInstr*)inst;
				_registers.insert_or_assign(load->_dst, load->_src);
				break;
			}
			case InstrKind::IK_I2I: {
				auto mov = (I2IInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(mov->_src);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				_registers.insert_or_assign(mov->_dst, *res.value());
				break;
			}

			case InstrKind::IK_ADD: {
				auto add = (AddInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(add->_src1);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				tl::expected<Value*, Error> res2 = get_register_value(add->_src2);
				if (!res2) {
					return tl::make_unexpected(res2.error());
				}
				_registers.insert_or_assign(add->_dst, res.value()->add(*res2.value()));
				break;
			}
			case InstrKind::IK_ADDIMM: {
				auto add = (AddImmInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(add->_src1);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				_registers.insert_or_assign(add->_dst, res.value()->add(add->_src2));
				break;
			}
			case InstrKind::IK_MULT: {
				auto mult = (MultInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(mult->_src1);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				tl::expected<Value*, Error> res2 = get_register_value(mult->_src2);
				if (!res2) {
					return tl::make_unexpected(res2.error());
				}
				_registers.insert_or_assign(mult->_dst, res.value()->mult(*res2.value()));
				break;
			}
			case InstrKind::IK_MULTIMM: {
				auto mult = (MultImmInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(mult->_src1);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				_registers.insert_or_assign(mult->_dst, res.value()->mult(mult->_src2));
				break;
			}

			case InstrKind::IK_CMP_GE: {
				auto cmp = (CmpGEInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(cmp->_src1);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				tl::expected<Value*, Error> res2 = get_register_value(cmp->_src2);
				if (!res2) {
					return tl::make_unexpected(res2.error());
				}
				_registers.insert_or_assign(
					cmp->_dst, res.value()->cmp_ge(*res2.value()));
				break;
			}
			case InstrKind::IK_CMP_LT: {
				auto cmp = (CmpLTInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(cmp->_src1);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				tl::expected<Value*, Error> res2 = get_register_value(cmp->_src2);
				if (!res2) {
					return tl::make_unexpected(res2.error());
				}
				_registers.insert_or_assign(
					cmp->_dst, res.value()->cmp_lt(*res2.value()));
				break;
			}

			case InstrKind::IK_CBR: {
				auto cmp = (CbrInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(cmp->_src);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				auto val = res.value();
				if (val->_kind != ValKind::VK_INT) {
					return tl::make_unexpected(
						Error(ErrorKind::EK_INVALID_INST, "cbr with non integer value"));
				}
				
				if (cmp->_dst._kind != ValKind::VK_LOCATION) {
					return tl::make_unexpected(
						Error(ErrorKind::EK_INVALID_INST, "cbr with non location jump"));
				}
				if (val->as_int()) {
					_block_name = cmp->_dst.as_loc();
					_inst_idx = 0;
				}
				break;
			}
			case InstrKind::IK_RET: {
				_stack.pop_back();
				if (_stack.size() == 0) {
					return ok_result;
				}
				break;
			}

			case InstrKind::IK_IWRITE: {
				auto write = (IWriteInstr*)inst;
				tl::expected<Value*, Error> res = get_register_value(write->_src);
				if (!res) {
					return tl::make_unexpected(res.error());
				}
				std::cout << *res.value() << "\n";
				break;
			}
			default:
				for (auto& pair : _registers._reg_map) {
					auto& [reg, val] = pair;
					std::cout << reg << " = " << *val << "\n";
				}
				std::cout << "INVALID INSTR" << *inst << "\n";
				return tl::make_unexpected(
					Error(ErrorKind::EK_INVALID_INST, "invalid instruction"));
		}

		auto& blk = get_block();
		while (_inst_idx == blk._instrs.size()) {
			_inst_idx = 0;
			_block_name = blk._fallthrough;
			blk = get_block();
		}

		if (_inst_idx == 0) {
			blk._exec_count += 1;
		}

		if (_inst_idx == 0 && blk._exec_count > 1) {
			auto j = Jit(blk._name, blk._instrs);
			j.compile();
			auto res = j.execute(_registers._reg_flat.data(), nullptr);
			
			std::cout << "jit code value: " << res << "\n";

			_block_name = blk._fallthrough;
		}
		inst = get_inst();
	}

	return ok_result;
}

}