#include "mips_recomp/decoder.h"
int main(){return mips_recomp::MipsDecoder::decode(0x03e00008,0x80001000).is_jr_ra?0:1;}
