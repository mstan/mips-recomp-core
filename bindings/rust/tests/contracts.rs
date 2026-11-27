use mips_recomp_core as m;

#[test]
fn instruction_fields_and_signed_immediate() {
    assert_eq!(m::primary::JAL, 3);
    assert_eq!(m::special::JR, 8);
    assert_eq!(m::regimm::BGEZAL, 0x11);
    let word = 0x8c83_fffc;
    assert_eq!((m::opcode(word), m::rs(word), m::rt(word)), (0x23, 4, 3));
    assert_eq!(m::immediate(word), 0xfffc);
    assert_eq!(m::signed_immediate(0xfffc), -4);
    assert_eq!(m::index(0x0bff_ffff), 0x03ff_ffff);
}

#[test]
fn targets_wrap_at_the_address_width() {
    assert_eq!(m::branch_target32(0xffff_fffc, 0x8000), 0xfffe_0000);
    assert_eq!(
        m::branch_target64(0xffff_ffff_ffff_fffc, 0x8000),
        0xffff_ffff_fffe_0000
    );
    assert_eq!(m::branch_target64(0, 0xffff), 0);
    assert_eq!(m::jump_target32(0x0fff_fffc, 1), 0x1000_0004);
    assert_eq!(
        m::jump_target64(0xffff_ffff_0fff_fffc, 1),
        0xffff_ffff_1000_0004
    );
}

#[test]
fn extensions_use_the_callers_operation_type() {
    #[derive(Debug, PartialEq)]
    enum Op {
        J,
        Jal,
        Beq,
        Bne,
        Blez,
        Bgtz,
        Addi,
        Addiu,
        Slti,
        Sltiu,
        Andi,
        Ori,
        Xori,
        Lui,
        Lb,
        Lh,
        Lwl,
        Lw,
        Lbu,
        Lhu,
        Lwr,
        Sb,
        Sh,
        Swl,
        Sw,
        Swr,
        Extension,
    }
    assert_eq!(m::mips_primary_match!(0x23,Op,{_=>Op::Extension}), Op::Lw);
    assert_eq!(
        m::mips_primary_match!(0x1c,Op,{_=>Op::Extension}),
        Op::Extension
    );
    assert_eq!(m::mips_primary_option_match!(0x1c,Op,{_=>None}), None);
}

#[test]
fn control_flow_and_direct_call_policy() {
    #[derive(Clone, Copy)]
    enum Op {
        J,
        Jal,
        Jr,
        Jalr,
        Beq,
        Bne,
        Blez,
        Bgtz,
        Bltz,
        Bgez,
        Bltzal,
        Bgezal,
        Beql,
    }
    assert!(m::mips_common_control!(Op::Beq, Op;));
    assert!(!m::mips_common_jump!(Op::Beq, Op;));
    assert!(m::mips_common_jump!(Op::Jalr, Op;));
    assert!(m::mips_common_direct_call!(Op::Bltzal, Op;));
    assert!(!m::mips_common_direct_call!(Op::Jalr, Op;));
    assert!(m::mips_common_control!(Op::Beql, Op; Op::Beql));
}
