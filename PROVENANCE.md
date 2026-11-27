# Provenance

The common integer encoding schema, generated language bindings, target helpers
and their contracts are Matthew Stanley contributions under the repository's
MIT grant. They describe portable encoding facts and retain caller-owned CPU
profile checks and extensions. No device or coprocessor implementation is moved
into these bindings.

Architecture decoder extracted from RetroPortingToolKit/psxrecomp `cea1b825e48480530a38815499b8112992a1b2ff`, `recompiler/include/mips_decoder.h` and `recompiler/src/mips_decoder.cpp`. Original PolyForm Noncommercial license retained.

Executable loading remains in psxrecomp. Unsigned branch-offset arithmetic removes signed-left-shift undefined behavior.
