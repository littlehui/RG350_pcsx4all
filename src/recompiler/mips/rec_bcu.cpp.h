static u32 psxBranchTest_rec(u32 cycles, u32 pc)
{
	/* Misc helper */
	psxRegs.pc = pc;
	psxRegs.cycle += cycles;

	psxBranchTest();

	u32 compiledpc = (u32)PC_REC32(psxRegs.pc);
	if (compiledpc) {
		//DEBUGF("returning to 0x%x (t2 0x%x t3 0x%x)\n", compiledpc, psxRegs.GPR.n.t2, psxRegs.GPR.n.t3);
		return compiledpc;
	}

	u32 a = recRecompile();
	//DEBUGF("returning to 0x%x (t2 0x%x t3 0x%x)\n", a, psxRegs.GPR.n.t2, psxRegs.GPR.n.t3);
	return a;
}

static void recSYSCALL()
{
	regClearJump();

	LI32(TEMP_1, pc - 4);
	SW(TEMP_1, PERM_REG_1, offpc);

	LI16(MIPSREG_A1, (branch == 1 ? 1 : 0));
	LI16(MIPSREG_A0, 0x20);
	CALLFunc((u32)psxException);

	LW(MIPSREG_A1, PERM_REG_1, offpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);

	end_block = 1;
}

/* Set a pending branch */
static INLINE void SetBranch()
{
	branch = 1;
	//psxRegs.code = *(u32*)(psxMemRLUT[pc>>16] + (pc&0xffff));
	psxRegs.code = *(u32 *)((char *)PSXM(pc));
	DISASM_PSX
	pc+=4;

	recBSC[psxRegs.code>>26]();
	branch = 0;
}

static INLINE void iJumpNormal(u32 branchPC)
{
	branch = 1;
	//psxRegs.code = *(u32*)(psxMemRLUT[pc>>16] + (pc&0xffff));
	psxRegs.code = *(u32 *)((char *)PSXM(pc));
	DISASM_PSX
	pc+=4;

	recBSC[psxRegs.code>>26]();

	branch = 0;

	regClearJump();

	LI32(MIPSREG_A1, branchPC);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);

	end_block = 1;
}

static INLINE void iJumpAL(u32 branchPC, u32 linkpc)
{
	branch = 1;
	//psxRegs.code = *(u32*)(psxMemRLUT[pc>>16] + (pc&0xffff));
	psxRegs.code = *(u32 *)((char *)PSXM(pc));
	DISASM_PSX
	pc+=4;

	recBSC[psxRegs.code>>26]();

	branch = 0;

	regClearJump();

	LI32(TEMP_1, linkpc);
	SW(TEMP_1, PERM_REG_1, offGPR(31));

	LI32(MIPSREG_A1, branchPC);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);

	end_block = 1;
}

static INLINE void iJump(u32 branchPC)
{
	branch = 1;
	//psxRegs.code = *(u32*)(psxMemRLUT[pc>>16] + (pc&0xffff));
	psxRegs.code = *(u32 *)((char *)PSXM(pc));
	DISASM_PSX
	pc+=4;

	recBSC[psxRegs.code>>26]();

	branch = 0;
	
	if(ibranch > 0)
	{
		regClearJump();
		LI32(MIPSREG_A1, branchPC);
		LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
		CALLFunc((u32)psxBranchTest_rec);
		end_block = 1;
		return;
	}

	ibranch++;
	blockcycles += ((pc-oldpc)/4);
	oldpc = pc = branchPC;
	recClear(branchPC, 1);
}

#if 1
static void recBLTZ()
{
// Branch if Rs < 0
	u32 bpc = _Imm_ * 4 + pc;
	u32 nbpc = pc + 4;
//	iFlushRegs();

	if (bpc == nbpc && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if(!(_Rs_))
	{
		SetBranch(); return;
	}

	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	SetBranch();
	u32 *backpatch = (u32*)recMem;
	write32(0x04010000 | (br1 << 21)); /* bgez */
	write32(0); /* nop */

	regClearBranch();
	LI32(MIPSREG_A1, bpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);
	rec_recompile_end();

	*backpatch |= mips_relative_offset(backpatch, (u32)recMem, 4);
	regBranchUnlock(br1);
}

static void recBGTZ()
{
// Branch if Rs > 0
	u32 bpc = _Imm_ * 4 + pc;
	u32 nbpc = pc + 4;
//	iFlushRegs();

	if (bpc == nbpc && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if(!(_Rs_))
	{
		SetBranch(); return;
	}

	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	SetBranch();
	u32 *backpatch = (u32*)recMem;
	write32(0x18000000 | (br1 << 21)); /* blez */
	write32(0); /* nop */

	regClearBranch();
	LI32(MIPSREG_A1, bpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);
	rec_recompile_end();

	*backpatch |= mips_relative_offset(backpatch, (u32)recMem, 4);
	regBranchUnlock(br1);
}

static void recBLTZAL()
{
// Branch if Rs < 0
	u32 bpc = _Imm_ * 4 + pc;
	u32 nbpc = pc + 4;
//	iFlushRegs();

	if (bpc == nbpc && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if(!(_Rs_))
	{
		SetBranch(); return;
	}

	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	SetBranch();
	u32 *backpatch = (u32*)recMem;
	write32(0x04010000 | (br1 << 21)); /* bgez */
	write32(0); /* nop */

	regClearBranch();
	LI32(TEMP_1, nbpc);
	SW(TEMP_1, PERM_REG_1, offGPR(31));

	LI32(MIPSREG_A1, bpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);
	rec_recompile_end();

	*backpatch |= mips_relative_offset(backpatch, (u32)recMem, 4);
	regBranchUnlock(br1);
}

static void recBGEZAL()
{
// Branch if Rs >= 0
	u32 bpc = _Imm_ * 4 + pc;
	u32 nbpc = pc + 4;
//	iFlushRegs();

	if (bpc == nbpc && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if(!(_Rs_))
	{
		iJumpAL(bpc, (pc + 4)); return;
	}

	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	SetBranch();
	u32 *backpatch = (u32*)recMem;
	write32(0x04000000 | (br1 << 21)); /* bltz */
	write32(0); /* nop */

	regClearBranch();
	LI32(TEMP_1, nbpc);
	SW(TEMP_1, PERM_REG_1, offGPR(31));

	LI32(MIPSREG_A1, bpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);
	rec_recompile_end();

	*backpatch |= mips_relative_offset(backpatch, (u32)recMem, 4);
	regBranchUnlock(br1);
}
#endif

static void recJ()
{
// j target

	iJumpNormal(_Target_ * 4 + (pc & 0xf0000000));
}

static void recJAL()
{
// jal target

	iJumpAL(_Target_ * 4 + (pc & 0xf0000000), (pc + 4));
}


static void recJR()
{
// jr Rs
	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	SetBranch();

	MOV(MIPSREG_A1, br1);
	regBranchUnlock(br1);
	regClearJump();
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);

	end_block = 1;
}

static void recJALR()
{
// jalr Rs
	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	u32 rd = regMipsToArm(_Rd_, REG_FIND, REG_REGISTER);
	LI32(rd, pc + 4);
	regMipsChanged(_Rd_);

	SetBranch();
	MOV(MIPSREG_A1, br1);
	regBranchUnlock(br1);
	regClearJump();
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);

	end_block = 1;
}

static void recBEQ()
{
// Branch if Rs == Rt
	u32 bpc = _Imm_ * 4 + pc;
	u32 nbpc = pc + 4;
//	iFlushRegs();

	if (bpc == nbpc && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if (_Rs_ == _Rt_)
	{
		iJumpNormal(bpc); return;
	}

	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	u32 br2 = regMipsToArm(_Rt_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	SetBranch();
	u32 *backpatch = (u32*)recMem;
	write32(0x14000000 | (br1 << 21) | (br2 << 16)); /* bne */
	write32(0); /* nop */

	regClearBranch();
	LI32(MIPSREG_A1, bpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);
	rec_recompile_end();

	*backpatch |= mips_relative_offset(backpatch, (u32)recMem, 4);
	regBranchUnlock(br1);
	regBranchUnlock(br2);
}

static void recBNE()
{
// Branch if Rs != Rt
	u32 bpc = _Imm_ * 4 + pc;
	u32 nbpc = pc + 4;
//	iFlushRegs();

	if (bpc == nbpc && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if(!(_Rs_) && !(_Rt_))
	{
		SetBranch(); return;
	}

	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	u32 br2 = regMipsToArm(_Rt_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	//DEBUGG("emitting beq %d(%d), %d(%d) (code 0x%x)\n", br1, _Rs_, br2, _Rt_, psxRegs.code);
	SetBranch();
	u32* backpatch = (u32*)recMem;
	//DEBUGG("encore br1 %d br2 %d\n", br1, br2);
	write32(0x10000000 | (br1 << 21) | (br2 << 16)); /* beq */
	write32(0); /* nop */

	regClearBranch();
	LI32(MIPSREG_A1, bpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);
	rec_recompile_end();

	//DEBUGG("backpatching %p rel to %p -> 0x%x\n", backpatch, recMem, mips_relative_offset(backpatch, (u32)recMem, 4));
	*backpatch |= mips_relative_offset(backpatch, (u32)recMem, 4);
	regBranchUnlock(br1);
	regBranchUnlock(br2);
}

#if 1
static void recBLEZ()
{
// Branch if Rs <= 0
	u32 bpc = _Imm_ * 4 + pc;
	u32 nbpc = pc + 4;
//	iFlushRegs();

	if (bpc == nbpc && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if(!(_Rs_))
	{
		iJumpNormal(bpc); return;
	}

	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	SetBranch();
	u32 *backpatch = (u32*)recMem;
	write32(0x1c000000 | (br1 << 21)); /* bgtz */
	write32(0); /* nop */

	regClearBranch();
	LI32(MIPSREG_A1, bpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);
	rec_recompile_end();

	*backpatch |= mips_relative_offset(backpatch, (u32)recMem, 4);
	regBranchUnlock(br1);
}

static void recBGEZ()
{
// Branch if Rs >= 0
	u32 bpc = _Imm_ * 4 + pc;
	u32 nbpc = pc + 4;
//	iFlushRegs();

	if (bpc == nbpc && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if(!(_Rs_))
	{
		iJumpNormal(bpc); return;
	}

	u32 br1 = regMipsToArm(_Rs_, REG_LOADBRANCH, REG_REGISTERBRANCH);
	SetBranch();
	u32 *backpatch = (u32*)recMem;
	write32(0x04000000 | (br1 << 21)); /* bltz */
	write32(0); /* nop */

	regClearBranch();
	LI32(MIPSREG_A1, bpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);
	rec_recompile_end();

	*backpatch |= mips_relative_offset(backpatch, (u32)recMem, 4);
	regBranchUnlock(br1);
}
#endif

static void recBREAK() { }

static void recHLE() 
{
	regClearJump();

	LI32(TEMP_1, pc);
	SW(TEMP_1, PERM_REG_1, offpc);

	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxHLEt[psxRegs.code & 0xffff]);

	LW(MIPSREG_A1, PERM_REG_1, offpc);
	LI32(MIPSREG_A0, ((blockcycles+((pc-oldpc)/4)))*BIAS);
	CALLFunc((u32)psxBranchTest_rec);

	end_block = 1;
}
