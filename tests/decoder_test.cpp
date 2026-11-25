#include "mips_decoder.h"
#include <cstdio>
#include <cstdlib>
#include <string_view>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
using namespace mips_recomp;
int main(){
 CHECK(MipsDecoder::compute_branch_target(0, -1)==0);
 CHECK(MipsDecoder::compute_branch_target(0xfffffffc,-32768)==0xfffe0000u);
 CHECK(MipsDecoder::compute_jump_target(0x0ffffffc,1)==0x10000004);
 auto zero=MipsDecoder::decode(0);CHECK(zero.is_nop);CHECK(std::string_view(zero.mnemonic)=="NOP");
 auto branch=MipsDecoder::decode(0x1022fffe,0x1000);CHECK(branch.is_branch && branch.branch_target==0xffc);
 auto load=MipsDecoder::decode(0x8c83fffc,0);CHECK(load.is_load && load.rt==3 && load.rs==4 && load.sign_ext_imm==-4);
 auto store=MipsDecoder::decode(0xac830004,0);CHECK(store.is_store && !store.is_load);
 auto ret=MipsDecoder::decode(0x03e00008,0);CHECK(ret.is_jr_ra && ret.is_jump && ret.is_delay_slot_user);
 auto gte=MipsDecoder::decode(0x4a000001,0);CHECK(gte.format==InstrFormat::COP2);
 uint32_t words[]={0,0x1022fffe};auto all=MipsDecoder::decode_words(words,0xfffffffc);CHECK(all.size()==2 && all[1].address==0);
 CHECK(MipsDecoder::decode_words({},0).empty());
 for(unsigned op=0;op<64;++op)for(unsigned fn=0;fn<64;++fn){auto x=MipsDecoder::decode((op<<26)|fn,0x80001000);CHECK(x.opcode==op && x.funct==fn && x.raw==((op<<26)|fn));}
 return 0;
}
