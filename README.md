# mips-recomp-core

Current licensing and the grant for existing owner contributions are documented in [LICENSE-SCOPE.md](LICENSE-SCOPE.md).

Private reusable R3000 instruction decoder extracted from psxrecomp. The core owns instruction fields, mnemonic/classification, and branch/jump target calculation. `decode_words` accepts host-order instruction words and a guest base address. Container loading, memory maps, GTE, interrupts, and the existing specialized code-generation backend stay in psxrecomp. No additional CPU profile or execution engine is claimed.

Use CMake target `mips-recomp-core::core`. Build with CMake and run CTest. The architecture-only interface is ready to replace psxrecomp decoder calls through an adapter in the later integration experiment.
# Integer frontend extensions

`isa/integer.json` owns the compatible MIPS integer selectors and fields.
`tools/generate_integer.py --check` checks the committed C++, Rust and Python
bindings. C++ uses compile-time selector lists; Rust macros expand into the
caller's existing operation type and extension arms. Neither requires a runtime
opcode conversion or a C ABI boundary. The dependency-free Rust crate lives in
`bindings/rust` and supports `no_std`.

The schema also owns common control-flow and memory/ALU classifications. Rust's
`mips_common_control!`, `mips_common_jump!` and `mips_common_direct_call!` accept
caller extension patterns. The direct-call predicate excludes JALR: its link
policy depends on the destination register. Python metadata uses the same
control kinds while retaining profile masks and execution bindings in its caller.

Rust also exposes `primary`, `special` and `regimm` selector constants from the
same schema. A typed adapter can use these in its existing ordered match when
block layout matters, without duplicating numeric encoding definitions.

The target helpers preserve unsigned address-width wrapping and use PC+4 for
the jump region. CPU-specific reserved encodings, canonical-field validation,
coprocessors, execution, metadata bindings and instruction extensions remain
caller-owned. Common selector presence does not authorize an encoding in every
CPU profile.
