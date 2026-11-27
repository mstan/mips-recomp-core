#![no_std]
include!("selectors.rs");

#[inline(always)]
pub const fn field(word: u32, shift: u32, width: u32) -> u32 {
    (word >> shift) & ((1_u32 << width) - 1)
}

#[inline(always)]
pub const fn signed_immediate(immediate: u16) -> i32 {
    immediate as i16 as i32
}

#[inline(always)]
pub const fn branch_target32(pc: u32, immediate: u16) -> u32 {
    pc.wrapping_add(4)
        .wrapping_add((signed_immediate(immediate) << 2) as u32)
}

#[inline(always)]
pub const fn branch_target64(pc: u64, immediate: u16) -> u64 {
    pc.wrapping_add(4)
        .wrapping_add((signed_immediate(immediate) as i64 * 4) as u64)
}

#[inline(always)]
pub const fn jump_target32(pc: u32, index: u32) -> u32 {
    (pc.wrapping_add(4) & 0xf000_0000) | (index << 2)
}

#[inline(always)]
pub const fn jump_target64(pc: u64, index: u32) -> u64 {
    (pc.wrapping_add(4) & 0xffff_ffff_f000_0000) | ((index as u64) << 2)
}
