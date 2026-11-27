#pragma once
#include <cstdint>
#include <type_traits>

namespace mips_recomp::integer {
template<class Address>
constexpr Address branch_target(Address pc, int16_t immediate) {
    static_assert(std::is_unsigned_v<Address> && sizeof(Address) >= 4);
    return pc + Address{4} + static_cast<Address>(static_cast<int32_t>(immediate) * 4);
}
template<class Address>
constexpr Address jump_target(Address pc, uint32_t index) {
    static_assert(std::is_unsigned_v<Address> && sizeof(Address) >= 4);
    return ((pc + Address{4}) & ~Address{0x0fffffff}) | (static_cast<Address>(index) << 2);
}
}
