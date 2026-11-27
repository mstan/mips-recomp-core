# mips-recomp-core

Current licensing and the grant for existing owner contributions are documented in [LICENSE-SCOPE.md](LICENSE-SCOPE.md).

Private reusable R3000 instruction decoder extracted from psxrecomp. The core owns instruction fields, mnemonic/classification, and branch/jump target calculation. `decode_words` accepts host-order instruction words and a guest base address. Container loading, memory maps, GTE, interrupts, and the existing specialized code-generation backend stay in psxrecomp. No additional CPU profile or execution engine is claimed.

Use CMake target `mips-recomp-core::core`. Build with CMake and run CTest. The architecture-only interface is ready to replace psxrecomp decoder calls through an adapter in the later integration experiment.
