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

template<typename T>
std::vector<T>
split(const T& str, const T& delimiters) {
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

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Need a file to read\n";
        return -1;
    }

    std::ifstream file(argv[1]);

    std::vector<jit::Instruction*> instrs;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            auto strs = split<std::string>(line, " ");
            auto& op = strs[0];
            if (op.starts_with('.')) {
                if (op.starts_with(".data")) {
                    instrs.push_back(new jit::DataInstr());
                } else if (op.starts_with(".text")) {
                    instrs.push_back(new jit::TextInstr());
                } else if (op.starts_with(".frame")) {
                    try {
                        std::vector<jit::Reg> params;
                        for (size_t i = 3; i < strs.size(); i++) {
                            auto p = strs[i].substr(0, strs[2].find_last_of(','));
                            params.push_back(jit::Reg{ std::stoul(p, nullptr, 10) });
                        }
                        auto size = strs[2].substr(0, strs[2].find_last_of(','));
                        instrs.push_back(new jit::FrameInstr(strs[1], std::stoul(size, nullptr, 10), params));
                    } catch (const std::exception&) {
                        std::cout << "Illegal argument to .frame instruction\n"
                            << line << "\n";
                        return -1;
                    }
                } else if (op.ends_with(':')) {
                    instrs.push_back(new jit::LabelInstr(op.substr(0, op.length() - 1)));
                }
            } else if (op.starts_with("loadI")) {
                try {
                    auto& s = strs[1];
                    auto d = strs[3].substr(3);
                    auto inst = new jit::LoadImmInstr(new jit::IntVal{ std::stoi(s, nullptr, 10) }, std::stoul(d, nullptr, 10));
                    instrs.push_back(inst);
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to loadI instruction\n"
                        << line << "\n";
                    return -1;
                }
            } else if (op.starts_with("i2i")) {
                try {
                    auto s = strs[1].substr(3);
                    auto d = strs[3].substr(3);
                    instrs.push_back(new jit::I2IInstr(std::stoul(s, nullptr, 10), std::stoul(d, nullptr, 10)));
                } catch (const std::exception&) {
                    std::cout << "Illegal argument to i2i instruction\n"
                        << line << "\n";
                    return -1;
                }
            }
        }
        file.close();
    }

    auto interp = jit::Interpreter(instrs);

    if (interp.run().has_value()) {
        std::cout << "Yay it works\n";
    } else {
        std::cout << "Failed to return OK expected...\n";
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
