#include "mips_decoder.h"
#include "mips_fields.h"
#include "mips_targets.h"
#include "mips_integer_selectors.h"

namespace mips_recomp {

// Register names (MIPS O32 ABI)
static const char* kRegisterNames[32] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0",   "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0",   "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8",   "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
};

const char* MipsDecoder::register_name(uint8_t reg) {
    if (reg >= 32) return "$??";
    return kRegisterNames[reg];
}

bool MipsDecoder::is_nop(uint32_t instr) {
    return instr == 0;
}

uint32_t MipsDecoder::compute_branch_target(uint32_t address, int16_t imm16) {
    // Branch target = PC + 4 + (sign_extended_offset << 2)
    return integer::branch_target(address, imm16);
}

uint32_t MipsDecoder::compute_jump_target(uint32_t address, uint32_t target26) {
    // Jump target = ((PC+4) & 0xF0000000) | (target26 << 2)
    return integer::jump_target(address, target26);
}

const char* MipsDecoder::get_mnemonic(const DecodedInstruction& instr) {
    return instr.mnemonic ? instr.mnemonic : "???";
}

// ---------------------------------------------------------------------------
// SPECIAL opcode (opcode == 0): funct field [5:0] determines operation
// ---------------------------------------------------------------------------
void MipsDecoder::decode_special(DecodedInstruction& out) {
    out.format = InstrFormat::SPECIAL;
    switch (out.funct) {
#define MIPS_MNEMONIC(value, operation, text) case value: out.mnemonic = text; break;
        MIPS_COMMON_SPECIAL(MIPS_MNEMONIC)
#undef MIPS_MNEMONIC
        default:
            out.mnemonic = "SPECIAL?";
            out.format   = InstrFormat::UNKNOWN;
            break;
    }
}

// ---------------------------------------------------------------------------
// REGIMM opcode (opcode == 1): rt field [20:16] determines operation
// ---------------------------------------------------------------------------
void MipsDecoder::decode_regimm(DecodedInstruction& out) {
    out.format = InstrFormat::REGIMM;
    switch (out.rt) {
#define MIPS_MNEMONIC(value, operation, text) case value: out.mnemonic = text; break;
        MIPS_COMMON_REGIMM(MIPS_MNEMONIC)
#undef MIPS_MNEMONIC
        default:
            out.mnemonic = "REGIMM?";
            out.format   = InstrFormat::UNKNOWN;
            break;
    }
}

// ---------------------------------------------------------------------------
// COP0 (opcode == 0x10): rs field [25:21] determines sub-operation
// ---------------------------------------------------------------------------
void MipsDecoder::decode_cop0(DecodedInstruction& out) {
    out.format = InstrFormat::COP0;
    // rs field holds the COP sub-opcode
    switch (out.rs) {
        case 0x00: out.mnemonic = "MFC0"; break;
        case 0x04: out.mnemonic = "MTC0"; break;
        case 0x10:
            // CO bit set — could be RFE (funct==0x10) or TLBR/TLBWI/etc.
            if (out.funct == 0x10) out.mnemonic = "RFE";
            else                   out.mnemonic = "COP0CO";
            break;
        default: out.mnemonic = "COP0"; break;
    }
}

// ---------------------------------------------------------------------------
// COP2 / GTE (opcode == 0x12): rs field determines sub-operation
// ---------------------------------------------------------------------------
void MipsDecoder::decode_cop2(DecodedInstruction& out) {
    out.format = InstrFormat::COP2;
    switch (out.rs) {
        case 0x00: out.mnemonic = "MFC2"; break;
        case 0x02: out.mnemonic = "CFC2"; break;
        case 0x04: out.mnemonic = "MTC2"; break;
        case 0x06: out.mnemonic = "CTC2"; break;
        default:
            // CO bit (bit 25) set => GTE command word
            if (out.rs & 0x10) out.mnemonic = "GTE";
            else               out.mnemonic = "COP2";
            break;
    }
}

// ---------------------------------------------------------------------------
// Classification flags (called after mnemonic/format are set)
// ---------------------------------------------------------------------------
void MipsDecoder::set_classification_flags(DecodedInstruction& out) {
    out.is_branch         = false;
    out.is_jump           = false;
    out.is_load           = false;
    out.is_store          = false;
    out.is_alu            = false;
    out.is_nop            = (out.raw == 0);
    out.is_syscall        = false;
    out.is_break          = false;
    out.is_jr_ra          = false;
    out.is_delay_slot_user = false;

    if (out.format == InstrFormat::UNKNOWN) return;
    if (out.is_nop) return;

    // SPECIAL (opcode == 0)
    if (out.opcode == 0x00) {
        switch (out.funct) {
#define MIPS_JUMP(value, operation) case value: out.is_jump = true; out.is_delay_slot_user = true; break;
            MIPS_SPECIAL_JUMP(MIPS_JUMP)
#undef MIPS_JUMP
#define MIPS_SYSCALL(value, operation) case value: out.is_syscall = true; break;
            MIPS_SPECIAL_SYSCALL(MIPS_SYSCALL)
#undef MIPS_SYSCALL
#define MIPS_BREAK(value, operation) case value: out.is_break = true; break;
            MIPS_SPECIAL_BREAK(MIPS_BREAK)
#undef MIPS_BREAK
            default: out.is_alu = true; break;
        }
        out.is_jr_ra = out.funct == 0x08 && out.rs == 31;

        return;
    }

    // REGIMM (opcode == 1) — all are conditional branches
    if (out.opcode == 0x01) {
        out.is_branch         = true;
        out.is_delay_slot_user = true;
        return;
    }

    switch (out.opcode) {
#define MIPS_JUMP(value, operation) case value: out.is_jump = true; out.is_delay_slot_user = true; break;
        MIPS_PRIMARY_JUMP(MIPS_JUMP)
#undef MIPS_JUMP
#define MIPS_BRANCH(value, operation) case value: out.is_branch = true; out.is_delay_slot_user = true; break;
        MIPS_PRIMARY_BRANCH(MIPS_BRANCH)
#undef MIPS_BRANCH
#define MIPS_ALU(value, operation) case value: out.is_alu = true; break;
        MIPS_PRIMARY_ALU(MIPS_ALU)
#undef MIPS_ALU
#define MIPS_LOAD(value, operation) case value: out.is_load = true; break;
        MIPS_PRIMARY_LOAD(MIPS_LOAD)
#undef MIPS_LOAD
#define MIPS_STORE(value, operation) case value: out.is_store = true; break;
        MIPS_PRIMARY_STORE(MIPS_STORE)
#undef MIPS_STORE
        // Coprocessor memory operations remain the R3000/GTE profile's extension.
        case 0x32: out.is_load = true; break;
        case 0x3a: out.is_store = true; break;
        default: break;
    }
}

// ---------------------------------------------------------------------------
// Main decode function
// ---------------------------------------------------------------------------
DecodedInstruction MipsDecoder::decode(uint32_t instr, uint32_t address) {
    DecodedInstruction out{};
    out.raw      = instr;
    out.address  = address;
    out.mnemonic = "???";
    out.format   = InstrFormat::UNKNOWN;

    // Extract all bit fields
    out.opcode = integer::opcode(instr);
    out.rs = integer::rs(instr);
    out.rt = integer::rt(instr);
    out.rd = integer::rd(instr);
    out.shamt = integer::sa(instr);
    out.funct = integer::funct(instr);
    out.uimm16   = (uint16_t)(instr & 0xFFFF);
    out.imm16    = (int16_t)(instr & 0xFFFF);
    out.target26 = integer::index(instr);
    out.sign_ext_imm = (int32_t)out.imm16;

    // Dispatch to format-specific decoder
    switch (out.opcode) {
#define MIPS_PRIMARY(value, operation, text) case value: out.mnemonic = text; out.format = value < 4 ? InstrFormat::J : InstrFormat::I; break;
        MIPS_COMMON_PRIMARY(MIPS_PRIMARY)
#undef MIPS_PRIMARY
        case 0x00: decode_special(out); break;
        case 0x01: decode_regimm(out);  break;
        case 0x10: decode_cop0(out); break;
        case 0x12: decode_cop2(out); break;
        case 0x32: out.mnemonic = "LWC2";  out.format = InstrFormat::I; break;
        case 0x3A: out.mnemonic = "SWC2";  out.format = InstrFormat::I; break;
        default:
            out.mnemonic = "???";
            out.format   = InstrFormat::UNKNOWN;
            break;
    }

    // Compute branch/jump targets
    if (out.opcode == 0x02 || out.opcode == 0x03) {
        // J / JAL
        out.jump_target = compute_jump_target(address, out.target26);
    }
    if (out.opcode == 0x04 || out.opcode == 0x05 ||
        out.opcode == 0x06 || out.opcode == 0x07 ||
        out.opcode == 0x01) {
        // BEQ/BNE/BLEZ/BGTZ + REGIMM (BLTZ/BGEZ/etc.)
        out.branch_target = compute_branch_target(address, out.imm16);
    }

    // Set classification flags
    set_classification_flags(out);

    // Override mnemonic for NOP (0x00000000 = SLL $zero,$zero,0)
    if (out.raw == 0) {
        out.mnemonic = "NOP";
    }

    return out;
}

std::vector<DecodedInstruction> MipsDecoder::decode_words(
    std::span<const uint32_t> words, uint32_t base_address) {
    std::vector<DecodedInstruction> result;
    result.reserve(words.size());
    for (uint32_t word : words) {
        result.push_back(decode(word, base_address));
        base_address += 4u;
    }
    return result;
}
} // namespace mips_recomp
