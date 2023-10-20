// jitjit.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

#include "interp.hpp"
#include "expected.hpp"
#include "instrs.hpp"
#include "jit.hpp"

template<typename T>
std::vector<T> split(const T& str, const T& delimiters) {
    std::vector<T> v;
    typename T::size_type start = 0;
    auto pos = str.find_first_of(delimiters, start);
    while (pos != T::npos) {
        // ignore empty tokens
        if (pos != start) {
            v.emplace_back(str, start, pos - start);
        }
        start = pos + 1;
        pos = str.find_first_of(delimiters, start);
    }
    // ignore trailing delimiter
    if (start < str.length()) {
        // add what's left of the string
        v.emplace_back(str, start, str.length() - start);
    }
    return v;
}

tl::expected<uint32_t, jit::Error> str_to_u32(const std::string& s) {
    try {
        return std::stoul(s, nullptr, 10);
    } catch (const std::exception&) {
        return tl::make_unexpected(jit::Error(jit::ErrorKind::EK_CAST, "failed to cast " + s));
    }
}
tl::expected<int32_t, jit::Error> str_to_i32(const std::string& s) {
    try {
        return std::stoi(s, nullptr, 10);
    }
    catch (const std::exception&) {
        return tl::make_unexpected(jit::Error(jit::ErrorKind::EK_CAST, "failed to cast " + s));
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Need a file to read\n";
        return -1;
    }

    std::ifstream file(argv[1]);

    auto parser = jit::Parser();
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            auto strs = split<std::string>(line, " ");
            auto& op = strs[0];
            if (op.starts_with('.')) {
                if (op.starts_with(".data")) {
                    parser.start_data(new jit::DataInstr());
                } else if (op.starts_with(".text")) {
                    parser.start_text(new jit::TextInstr());
                } else if (op.starts_with(".frame")) {
                    try {
                        std::vector<jit::Reg> params;
                        for (size_t i = 3; i < strs.size(); i++) {
                            auto p = strs[i].substr(0, strs[2].find_last_of(','));
                            params.push_back(jit::Reg{ std::stoul(p, nullptr, 10) });
                        }
                        auto size = strs[2].substr(0, strs[2].find_last_of(','));
                        auto name = strs[1].substr(0, strs[1].find_last_of(','));
                        parser.push_frame(new jit::FrameInstr(name, std::stoul(size, nullptr, 10), params));
                    } catch (const std::exception&) {
                        std::cout << "Illegal argument to .frame instruction\n"
                            << line << "\n";
                        return -1;
                    }
                } else if (op.ends_with(':')) {
                    parser.push_label(new jit::LabelInstr(op.substr(0, op.length() - 1)));
                } else {
                    std::cout << "INVALID INSTRUCTION WITH ." << line << "\n";
                }
            } else if (op.starts_with("loadI")) {
                try {
                    auto& s = strs[1];
                    auto d = strs[3].substr(3);
                    auto inst = new jit::LoadImmInstr(
                        jit::Value{ (int64_t)std::stol(s, nullptr, 10) },
                        std::stoul(d, nullptr, 10)
                    );
                    parser.push_instr(inst);
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to loadI instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("i2i")) {
                try {
                    auto s = strs[1].substr(3);
                    auto d = strs[3].substr(3);
                    parser.push_instr(new jit::I2IInstr(std::stoul(s, nullptr, 10), std::stoul(d, nullptr, 10)));
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to i2i instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("addI")) {
                try {
                    auto s1 = strs[1].substr(3, strs[1].find_last_of(','));
                    auto s2 = strs[2].substr(0, strs[2].find_last_of(','));
                    auto d = strs[4].substr(3);
                    parser.push_instr(new jit::AddImmInstr(
                        std::stoul(s1, nullptr, 10),
                        jit::Value{ (int64_t)std::stol(s2, nullptr, 10) },
                        std::stoul(d, nullptr, 10))
                    );
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to addI instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("add")) {
                try {
                    auto s1 = strs[1].substr(3, strs[1].find_last_of(','));
                    auto s2 = strs[2].substr(3, strs[2].find_last_of(','));
                    auto d = strs[4].substr(3);
                    parser.push_instr(new jit::AddInstr(
                        std::stoul(s1, nullptr, 10),
                        std::stoul(s2, nullptr, 10),
                        std::stoul(d, nullptr, 10))
                    );
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to add instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("multI")) {
                try {
                    auto s1 = strs[1].substr(3, strs[1].find_last_of(','));
                    auto s2 = strs[2].substr(0, strs[2].find_last_of(','));
                    auto d = strs[4].substr(3);
                    parser.push_instr(new jit::MultImmInstr(
                        std::stoul(s1, nullptr, 10),
                        jit::Value{ (int64_t)std::stol(s2, nullptr, 10) },
                        std::stoul(d, nullptr, 10))
                    );
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to multI instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("mult")) {
                try {
                    auto s1 = strs[1].substr(3, strs[1].find_last_of(','));
                    auto s2 = strs[2].substr(3, strs[2].find_last_of(','));
                    auto d = strs[4].substr(3);
                    parser.push_instr(new jit::MultInstr(
                        std::stoul(s1, nullptr, 10),
                        std::stoul(s2, nullptr, 10),
                        std::stoul(d, nullptr, 10))
                    );
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to mult instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("cmp_")) {
                try {
                    auto s1 = std::stoul(strs[1].substr(3, strs[1].find_last_of(',')), nullptr, 10);
                    auto s2 = std::stoul(strs[2].substr(3, strs[2].find_last_of(',')), nullptr, 10);
                    auto d = std::stoul(strs[4].substr(3), nullptr, 10);
                    auto ctor = [=]() -> jit::Instruction* {
                        if (op.ends_with("GE")) return new jit::CmpGEInstr(s1, s2, d);
                        else if (op.ends_with("GT")) return new jit::CmpGTInstr(s1, s2, d);
                        else if (op.ends_with("LT")) return new jit::CmpLTInstr(s1, s2, d);
                        else if (op.ends_with("LE")) return new jit::CmpLEInstr(s1, s2, d);
                        else return nullptr;
                    };
                    parser.push_instr(ctor());
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to cmp instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("cbr")) {
                try {
                    auto s = strs[1].substr(3);
                    auto d = strs[3];
                    parser.push_instr(new jit::CbrInstr(
                        std::stoul(s, nullptr, 10),
                        jit::Value{ jit::ValKind::VK_LOCATION, d }
                    ));
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to cbr instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("iwrite")) {
                try {
                    auto s = strs[1].substr(3);
                    parser.push_instr(new jit::IWriteInstr(std::stoul(s, nullptr, 10)));
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to iwrite instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("ret")) {
                parser.push_instr(new jit::RetInstr());
            }
        }
        parser.finalize_prog();
        file.close();
    }

    auto interp = jit::Interpreter(std::move(parser._funcs), std::move(parser._globals));
    auto res = interp.run();
    if (!res.has_value()) {
        std::cout << "Failed to return OK expected...\n" << res.error();
    }
}
