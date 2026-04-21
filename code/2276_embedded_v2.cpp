#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

// This C++ program contains and executes mov code
// The mov code is embedded in the mov_code array below

#define MEMORYSIZE 512
typedef unsigned char uint8;
typedef unsigned short uint16;

uint8 reg[26];
uint8 mem[MEMORYSIZE];

struct Instruction {
    enum Mode { IMM, REG, MEM_REG, MEM_REG_IMM, MEM_REG_REG, MEM_IMM };
    Mode src_mode, dest_mode;
    uint8 src_reg, dest_reg;
    uint8 src_offset_reg, dest_offset_reg;
    uint16 src_imm, dest_imm;
};

std::vector<Instruction> program;

void execute_mov(const Instruction& inst) {
    // Get source value
    uint8 src_val = 0;
    switch (inst.src_mode) {
        case Instruction::IMM:
            src_val = inst.src_imm;
            break;
        case Instruction::REG:
            if (inst.src_reg == 8) { // I register - input
                int c = getchar();
                src_val = (c == EOF) ? 0 : c;
            } else {
                src_val = reg[inst.src_reg];
            }
            break;
        case Instruction::MEM_REG:
            src_val = mem[reg[inst.src_reg] % MEMORYSIZE];
            break;
        case Instruction::MEM_REG_IMM:
            src_val = mem[(reg[inst.src_reg] + inst.src_imm) % MEMORYSIZE];
            break;
        case Instruction::MEM_REG_REG:
            src_val = mem[(reg[inst.src_reg] + reg[inst.src_offset_reg]) % MEMORYSIZE];
            break;
        case Instruction::MEM_IMM:
            src_val = mem[inst.src_imm % MEMORYSIZE];
            break;
    }

    // Store destination value
    switch (inst.dest_mode) {
        case Instruction::REG:
            if (inst.dest_reg == 14) { // O register - output
                putchar(src_val);
            } else if (inst.dest_reg == 25) { // Z register - halt
                if (src_val != 0) exit(0);
            } else {
                reg[inst.dest_reg] = src_val;
            }
            break;
        case Instruction::MEM_REG:
            mem[reg[inst.dest_reg] % MEMORYSIZE] = src_val;
            break;
        case Instruction::MEM_REG_IMM:
            mem[(reg[inst.dest_reg] + inst.dest_imm) % MEMORYSIZE] = src_val;
            break;
        case Instruction::MEM_REG_REG:
            mem[(reg[inst.dest_reg] + reg[inst.dest_offset_reg]) % MEMORYSIZE] = src_val;
            break;
        case Instruction::MEM_IMM:
            mem[inst.dest_imm % MEMORYSIZE] = src_val;
            break;
        default:
            break;
    }
}

bool parse_instruction(const std::string& line, Instruction& inst) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') return false;

    // Find the < operator
    size_t pos = line.find('<');
    if (pos == std::string::npos) return false;

    std::string dest = line.substr(0, pos);
    std::string src = line.substr(pos + 1);

    // Trim whitespace
    dest.erase(0, dest.find_first_not_of(" \t"));
    dest.erase(dest.find_last_not_of(" \t") + 1);
    src.erase(0, src.find_first_not_of(" \t"));
    src.erase(src.find_last_not_of(" \t") + 1);

    // Parse destination
    if (dest.length() == 1 && dest[0] >= 'A' && dest[0] <= 'Z') {
        inst.dest_mode = Instruction::REG;
        inst.dest_reg = dest[0] - 'A';
    } else if (dest[0] == '[' && dest[dest.length()-1] == ']') {
        std::string inside = dest.substr(1, dest.length() - 2);
        if (inside.length() == 1 && inside[0] >= 'A' && inside[0] <= 'Z') {
            inst.dest_mode = Instruction::MEM_REG;
            inst.dest_reg = inside[0] - 'A';
        } else {
            // Check for [R+R] or [R+I] pattern
            size_t plus = inside.find('+');
            if (plus != std::string::npos) {
                std::string part1 = inside.substr(0, plus);
                std::string part2 = inside.substr(plus + 1);

                if (part1.length() == 1 && part1[0] >= 'A' && part1[0] <= 'Z') {
                    if (part2.length() == 1 && part2[0] >= 'A' && part2[0] <= 'Z') {
                        // [R+R]
                        inst.dest_mode = Instruction::MEM_REG_REG;
                        inst.dest_reg = part1[0] - 'A';
                        inst.dest_offset_reg = part2[0] - 'A';
                    } else {
                        // [R+I]
                        inst.dest_mode = Instruction::MEM_REG_IMM;
                        inst.dest_reg = part1[0] - 'A';
                        inst.dest_imm = std::atoi(part2.c_str());
                    }
                }
            } else {
                // [I]
                inst.dest_mode = Instruction::MEM_IMM;
                inst.dest_imm = std::atoi(inside.c_str());
            }
        }
    } else {
        return false;
    }

    // Parse source
    if (src.length() == 1 && src[0] >= 'A' && src[0] <= 'Z') {
        inst.src_mode = Instruction::REG;
        inst.src_reg = src[0] - 'A';
    } else if (src[0] == '[' && src[src.length()-1] == ']') {
        std::string inside = src.substr(1, src.length() - 2);
        if (inside.length() == 1 && inside[0] >= 'A' && inside[0] <= 'Z') {
            inst.src_mode = Instruction::MEM_REG;
            inst.src_reg = inside[0] - 'A';
        } else {
            // Check for [R+R] or [R+I] pattern
            size_t plus = inside.find('+');
            if (plus != std::string::npos) {
                std::string part1 = inside.substr(0, plus);
                std::string part2 = inside.substr(plus + 1);

                if (part1.length() == 1 && part1[0] >= 'A' && part1[0] <= 'Z') {
                    if (part2.length() == 1 && part2[0] >= 'A' && part2[0] <= 'Z') {
                        // [R+R]
                        inst.src_mode = Instruction::MEM_REG_REG;
                        inst.src_reg = part1[0] - 'A';
                        inst.src_offset_reg = part2[0] - 'A';
                    } else {
                        // [R+I]
                        inst.src_mode = Instruction::MEM_REG_IMM;
                        inst.src_reg = part1[0] - 'A';
                        inst.src_imm = std::atoi(part2.c_str());
                    }
                }
            } else {
                // [I]
                inst.src_mode = Instruction::MEM_IMM;
                inst.src_imm = std::atoi(inside.c_str());
            }
        }
    } else {
        // Immediate value
        inst.src_mode = Instruction::IMM;
        inst.src_imm = std::atoi(src.c_str());
    }

    return true;
}

int main() {
    // Initialize
    memset(reg, 0, sizeof(reg));
    memset(mem, 0, sizeof(mem));

    // Parse mov instructions from the embedded code
    std::istringstream mov_code(R"(# Hello World
A<72
O<A
A<101
O<A
A<108
O<A
A<108
O<A
A<111
O<A
A<32
O<A
A<87
O<A
A<111
O<A
A<114
O<A
A<108
O<A
A<100
O<A
A<33
O<A
Z<1)");

    std::string line;
    while (std::getline(mov_code, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        Instruction inst;
        if (parse_instruction(line, inst)) {
            program.push_back(inst);
        }
    }

    // Execute program in a loop
    while (true) {
        for (const auto& inst : program) {
            execute_mov(inst);
        }
    }

    return 0;
}