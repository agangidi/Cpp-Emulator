Cpp-Emulator
============

C++  Emulator for M8 Processor


// m8.cpp: implementation of the m8 class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#pragma warning( disable:4786)  // Disable vector debug info truncation warning
#pragma warning( disable:4503)  // Disable vector debug info truncation warning

#include "m8.h"
#include "flash.h"
#include "sram.h"
#include "data_bus.h"
#include "clock.h"
#include "emu_core.h"
#include "emu_global.h"
#include "ice.h"
#include "int_ctl.h"
#include "test_controller.h"
#include "dump_waves.h"
#include "assert.h"
#include "dig_top.h"

#include <iostream>
#include <fstream>

extern char *pEmuTraceFilename;
extern int SimState;
extern int MaxSimClocks;
extern int VerboseLevel;
extern int TraceOn;
extern int CycleCounterActive;
extern int CycleCounterPCStart;
extern int CycleCounterPCEnd;
extern int StackTrackerActive;
extern int StackTrackerPCStart;
extern int StackTrackerPCEnd;
extern int StackMin;
extern int StackMax;
extern bool IsStackMinActive;
extern int OCDMode;
extern int VersatestMode;
extern int DstestMode;
extern int WaveDumpActive;

extern flash flash0;
extern sram sram0;
extern data_bus data_bus0;
extern emu_clock clock0;
extern test_controller test_controller0, test_controller0_next;
extern int_ctl int_ctl0, int_ctl0_next;
extern dump_waves dump_waves0;
extern dig_top dig_top0_next;

extern ice ice0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

m8::m8()
{

}


m8::~m8()
{
}

void m8::Initialize(void) {

  dump_waves::DumpStruct stDumpTemp;
	dump_waves::DumpStruct stTemp;

	// no need to reset EmuTrace array

    Xres();

	// ...

	// set up wave nets

	CPUCLKNet.Initialize();

	std::ostringstream temp4;
	temp4 << "cpuclk";
	CPUCLKNet.m_NetName = temp4.str();
	CPUCLKNet.m_ModuleName = "M8";
	if(WaveDumpActive == 1) {
		//VCD file generation is active
		stTemp.cInitVal = 0;	 //Initialize all to 0
		stTemp.cVarType = VARISWIRE; //Non register nets
		stTemp.iVarWidth = 1;		 //No busses
		dump_waves0.vInitDumpFileVariable("M8", temp4.str(), stTemp);	//Initializes pin for VCD dump	
	}

	SOINet.Initialize();

	std::ostringstream temp1;
	temp1 << "soi";
	SOINet.m_NetName = temp1.str();
	SOINet.m_ModuleName = "M8";
	if(WaveDumpActive == 1) {
		//VCD file generation is active
		stTemp.cInitVal = 0;	 //Initialize all to 0
		stTemp.cVarType = VARISWIRE; //Non register nets
		stTemp.iVarWidth = 1;		 //No busses
		dump_waves0.vInitDumpFileVariable("M8", temp1.str(), stTemp);	//Initializes pin for VCD dump	
	}

	FIRSTNet.Initialize();

	std::ostringstream temp2;
	temp2 << "first";
	FIRSTNet.m_NetName = temp2.str();
	FIRSTNet.m_ModuleName = "M8";
	if(WaveDumpActive == 1) {
		//VCD file generation is active
		stTemp.cInitVal = 0;	 //Initialize all to 0
		stTemp.cVarType = VARISWIRE; //Non register nets
		stTemp.iVarWidth = 1;		 //No busses
		dump_waves0.vInitDumpFileVariable("M8", temp2.str(), stTemp);	//Initializes pin for VCD dump	
	}

	EXECNet.Initialize();

	std::ostringstream temp3;
	temp3 << "exec";
	EXECNet.m_NetName = temp3.str();
	EXECNet.m_ModuleName = "M8";
	if(WaveDumpActive == 1) {
		//VCD file generation is active
		stTemp.cInitVal = 1;	 //Initialize all to 1
		stTemp.cVarType = VARISWIRE; //Non register nets
		stTemp.iVarWidth = 1;		 //No busses
		dump_waves0.vInitDumpFileVariable("M8", temp3.str(), stTemp);	//Initializes pin for VCD dump	
	}

	DBDRVNet.Initialize();

	std::ostringstream temp400;
	temp400 << "dbdrv";
	DBDRVNet.m_NetName = temp400.str();
	DBDRVNet.m_ModuleName = "M8";
	if(WaveDumpActive == 1) {
		//VCD file generation is active
		stTemp.cInitVal = 1;	 //Initialize all to 1
		stTemp.cVarType = VARISWIRE; //Non register nets
		stTemp.iVarWidth = 1;		 //No busses
		dump_waves0.vInitDumpFileVariable("M8", temp400.str(), stTemp);	//Initializes pin for VCD dump	
	}

	//...

	DoStackTrack = 0;
	PCCycleCount = 0;
	MaxStack = 0;

	// set up wave nets
	if(WaveDumpActive == 1) {
	
		stDumpTemp.iVarWidth = 32;
		stDumpTemp.cInitVal = X_BIT;
		stDumpTemp.cVarType = VARISREG;
		dump_waves0.vInitDumpFileVariable("M8", "TotalCPUClocks[31:0]", stDumpTemp);

		stDumpTemp.iVarWidth = 16;
		stDumpTemp.cInitVal = X_BIT;
		stDumpTemp.cVarType = VARISREG;
		dump_waves0.vInitDumpFileVariable("M8", "PC[15:0]", stDumpTemp);

		stDumpTemp.iVarWidth = 8;
		stDumpTemp.cInitVal = X_BIT;
		stDumpTemp.cVarType = VARISREG;
		dump_waves0.vInitDumpFileVariable("M8", "IR[7:0]", stDumpTemp);

		stDumpTemp.iVarWidth = 8;
		stDumpTemp.cInitVal = X_BIT;
		stDumpTemp.cVarType = VARISREG;
		dump_waves0.vInitDumpFileVariable("M8", "A[7:0]", stDumpTemp);

		stDumpTemp.iVarWidth = 8;
		stDumpTemp.cInitVal = X_BIT;
		stDumpTemp.cVarType = VARISREG;
		dump_waves0.vInitDumpFileVariable("M8", "SP[7:0]", stDumpTemp);

		stDumpTemp.iVarWidth = 8;
		stDumpTemp.cInitVal = X_BIT;
		stDumpTemp.cVarType = VARISREG;
		dump_waves0.vInitDumpFileVariable("M8", "X[7:0]", stDumpTemp);

		stDumpTemp.iVarWidth = 8;
		stDumpTemp.cInitVal = X_BIT;
		stDumpTemp.cVarType = VARISREG;
		dump_waves0.vInitDumpFileVariable("M8", "FL[7:0]", stDumpTemp);

		stDumpTemp.iVarWidth = 32;
		stDumpTemp.cInitVal = X_BIT;
		stDumpTemp.cVarType = VARISREG;
		dump_waves0.vInitDumpFileVariable("M8", "BRQStall[31:0]", stDumpTemp);

	}

	fpOp[0x00] = &m8::Op_SSC;			// 0x00
	fpOp[0x01] = &m8::Op_Add_A_k;		// 0x01
	fpOp[0x02] = &m8::Op_Add_A_Mk;		// 0x02
	fpOp[0x03] = &m8::Op_Add_A_MXpk;	// 0x03
	fpOp[0x04] = &m8::Op_Add_Mk_A;		// 0x04
	fpOp[0x05] = &m8::Op_Add_MXpk_A;	// 0x05
	fpOp[0x06] = &m8::Op_Add_Mk_i;		// 0x06
	fpOp[0x07] = &m8::Op_Add_MXpk_i;	// 0x07
	fpOp[0x08] = &m8::Op_Push_A;		// 0x08
	fpOp[0x09] = &m8::Op_Adc_A_k;		// 0x09
	fpOp[0x0a] = &m8::Op_Adc_A_Mk;		// 0x0a
	fpOp[0x0b] = &m8::Op_Adc_A_MXpk;	// 0x0b
	fpOp[0x0c] = &m8::Op_Adc_Mk_A;		// 0x0c
	fpOp[0x0d] = &m8::Op_Adc_MXpk_A;	// 0x0d
	fpOp[0x0e] = &m8::Op_Adc_Mk_i;		// 0x0e
	fpOp[0x0f] = &m8::Op_Adc_MXpk_i;	// 0x0f
	fpOp[0x10] = &m8::Op_Push_X;		// 0x10
	fpOp[0x11] = &m8::Op_Sub_A_k;		// 0x11
	fpOp[0x12] = &m8::Op_Sub_A_Mk;		// 0x12
	fpOp[0x13] = &m8::Op_Sub_A_MXpk;	// 0x13
	fpOp[0x14] = &m8::Op_Sub_Mk_A;		// 0x14
	fpOp[0x15] = &m8::Op_Sub_MXpk_A;	// 0x15
	fpOp[0x16] = &m8::Op_Sub_Mk_i;		// 0x16
	fpOp[0x17] = &m8::Op_Sub_MXpk_i;	// 0x17
	fpOp[0x18] = &m8::Op_Pop_A;		// 0x18
	fpOp[0x19] = &m8::Op_Sbb_A_k;		// 0x19
	fpOp[0x1a] = &m8::Op_Sbb_A_Mk;		// 0x1a
	fpOp[0x1b] = &m8::Op_Sbb_A_MXpk;	// 0x1b
	fpOp[0x1c] = &m8::Op_Sbb_Mk_A;		// 0x1c
	fpOp[0x1d] = &m8::Op_Sbb_MXpk_A;	// 0x1d
	fpOp[0x1e] = &m8::Op_Sbb_Mk_i;		// 0x1e
	fpOp[0x1f] = &m8::Op_Sbb_MXpk_i;	// 0x1f
	fpOp[0x20] = &m8::Op_Pop_X;		// 0x20
	fpOp[0x21] = &m8::Op_And_A_k;		// 0x21
	fpOp[0x22] = &m8::Op_And_A_Mk;		// 0x22
	fpOp[0x23] = &m8::Op_And_A_MXpk;	// 0x23
	fpOp[0x24] = &m8::Op_And_Mk_A;		// 0x24
	fpOp[0x25] = &m8::Op_And_MXpk_A;	// 0x25
	fpOp[0x26] = &m8::Op_And_Mk_i;		// 0x26
	fpOp[0x27] = &m8::Op_And_MXpk_i;	// 0x27
	fpOp[0x28] = &m8::Op_Romx;			// 0x28
	fpOp[0x29] = &m8::Op_Or_A_k;		// 0x29
	fpOp[0x2a] = &m8::Op_Or_A_Mk;		// 0x2a
	fpOp[0x2b] = &m8::Op_Or_A_MXpk;	// 0x2b
	fpOp[0x2c] = &m8::Op_Or_Mk_A;		// 0x2c
	fpOp[0x2d] = &m8::Op_Or_MXpk_A;	// 0x2d
	fpOp[0x2e] = &m8::Op_Or_Mk_i;		// 0x2e
	fpOp[0x2f] = &m8::Op_Or_MXpk_i;	// 0x2f
	fpOp[0x30] = &m8::Op_Halt;			// 0x30
	fpOp[0x31] = &m8::Op_Xor_A_k;		// 0x31
	fpOp[0x32] = &m8::Op_Xor_A_Mk;		// 0x32
	fpOp[0x33] = &m8::Op_Xor_A_MXpk;	// 0x33
	fpOp[0x34] = &m8::Op_Xor_Mk_A;		// 0x34
	fpOp[0x35] = &m8::Op_Xor_MXpk_A;	// 0x35
	fpOp[0x36] = &m8::Op_Xor_Mk_i;		// 0x36
	fpOp[0x37] = &m8::Op_Xor_MXpk_i;	// 0x37
	fpOp[0x38] = &m8::Op_Add_SP_k;		// 0x38
	fpOp[0x39] = &m8::Op_Cmp_A_k;		// 0x39
	fpOp[0x3a] = &m8::Op_Cmp_A_Mk;		// 0x3a
	fpOp[0x3b] = &m8::Op_Cmp_A_MXpk;	// 0x3b
	fpOp[0x3c] = &m8::Op_Cmp_Mk_i;		// 0x3c
	fpOp[0x3d] = &m8::Op_Cmp_MXpk_i;	// 0x3d
	fpOp[0x3e] = &m8::Op_Mvi_A_MMkpp;	// 0x3e
	fpOp[0x3f] = &m8::Op_Mvi_MMkpp_A;	// 0x3f
	fpOp[0x40] = &m8::Op_Nop;			// 0x40
	fpOp[0x41] = &m8::Op_And_IOk_i;	// 0x41
	fpOp[0x42] = &m8::Op_And_IOXpk_i;	// 0x42
	fpOp[0x43] = &m8::Op_Or_IOk_i;		// 0x43
	fpOp[0x44] = &m8::Op_Or_IOXpk_i;	// 0x44
	fpOp[0x45] = &m8::Op_Xor_IOk_i;	// 0x45
	fpOp[0x46] = &m8::Op_Xor_IOXpk_i;	// 0x46
	fpOp[0x47] = &m8::Op_Tst_Mk_i;		// 0x47
	fpOp[0x48] = &m8::Op_Tst_MXpk_i;	// 0x48
	fpOp[0x49] = &m8::Op_Tst_IOk_i;	// 0x49
	fpOp[0x4a] = &m8::Op_Tst_IOXpk_i;	// 0x4a
	fpOp[0x4b] = &m8::Op_Swap_A_X;		// 0x4b
	fpOp[0x4c] = &m8::Op_Swap_A_Mk;	// 0x4c
	fpOp[0x4d] = &m8::Op_Swap_X_Mk;	// 0x4d
	fpOp[0x4e] = &m8::Op_Swap_A_SP;	// 0x4e
	fpOp[0x4f] = &m8::Op_Mov_X_SP;		// 0x4f
	fpOp[0x50] = &m8::Op_Mov_A_k;		// 0x50
	fpOp[0x51] = &m8::Op_Mov_A_Mk;		// 0x51
	fpOp[0x52] = &m8::Op_Mov_A_MXpk;	// 0x52
	fpOp[0x53] = &m8::Op_Mov_Mk_A;		// 0x53
	fpOp[0x54] = &m8::Op_Mov_MXpk_A;	// 0x54
	fpOp[0x55] = &m8::Op_Mov_Mk_i;		// 0x55
	fpOp[0x56] = &m8::Op_Mov_MXpk_i;	// 0x56
	fpOp[0x57] = &m8::Op_Mov_X_k;		// 0x57
	fpOp[0x58] = &m8::Op_Mov_X_Mk;		// 0x58
	fpOp[0x59] = &m8::Op_Mov_X_MXpk;	// 0x59
	fpOp[0x5a] = &m8::Op_Mov_Mk_X;		// 0x5a
	fpOp[0x5b] = &m8::Op_Mov_A_X;		// 0x5b
	fpOp[0x5c] = &m8::Op_Mov_X_A;		// 0x5c
	fpOp[0x5d] = &m8::Op_Mov_A_IOk;	// 0x5d
	fpOp[0x5e] = &m8::Op_Mov_A_IOXpk;	// 0x5e
	fpOp[0x5f] = &m8::Op_Mov_Mi_Mk;	// 0x5f
	fpOp[0x60] = &m8::Op_Mov_IOk_A;	// 0x60
	fpOp[0x61] = &m8::Op_Mov_IOXpk_A;	// 0x61
	fpOp[0x62] = &m8::Op_Mov_IOk_i;	// 0x62
	fpOp[0x63] = &m8::Op_Mov_IOXpk_i;	// 0x63
	fpOp[0x64] = &m8::Op_Asl_A;		// 0x64
	fpOp[0x65] = &m8::Op_Asl_Mk;		// 0x65
	fpOp[0x66] = &m8::Op_Asl_MXpk;		// 0x66
	fpOp[0x67] = &m8::Op_Asr_A;		// 0x67
	fpOp[0x68] = &m8::Op_Asr_Mk;		// 0x68
	fpOp[0x69] = &m8::Op_Asr_MXpk;		// 0x69
	fpOp[0x6a] = &m8::Op_Rlc_A;		// 0x6a
	fpOp[0x6b] = &m8::Op_Rlc_Mk;		// 0x6b
	fpOp[0x6c] = &m8::Op_Rlc_MXpk;		// 0x6c
	fpOp[0x6d] = &m8::Op_Rrc_A;		// 0x6d
	fpOp[0x6e] = &m8::Op_Rrc_Mk;		// 0x6e
	fpOp[0x6f] = &m8::Op_Rrc_MXpk;		// 0x6f
	fpOp[0x70] = &m8::Op_And_F_k;		// 0x70
	fpOp[0x71] = &m8::Op_Or_F_k;		// 0x71
	fpOp[0x72] = &m8::Op_Xor_F_k;		// 0x72
	fpOp[0x73] = &m8::Op_Cpl_A;		// 0x73
	fpOp[0x74] = &m8::Op_Inc_A;		// 0x74
	fpOp[0x75] = &m8::Op_Inc_X;		// 0x75
	fpOp[0x76] = &m8::Op_Inc_Mk;		// 0x76
	fpOp[0x77] = &m8::Op_Inc_MXpk;		// 0x77
	fpOp[0x78] = &m8::Op_Dec_A;		// 0x78
	fpOp[0x79] = &m8::Op_Dec_X;		// 0x79
	fpOp[0x7a] = &m8::Op_Dec_Mk;		// 0x7a
	fpOp[0x7b] = &m8::Op_Dec_MXpk;		// 0x7b
	fpOp[0x7c] = &m8::Op_Lcall_k_i;	// 0x7c
	fpOp[0x7d] = &m8::Op_Ljmp_k_i;		// 0x7d
	fpOp[0x7e] = &m8::Op_Reti;			// 0x7e
	fpOp[0x7f] = &m8::Op_Ret;			// 0x7f
	fpOp[0x80] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x81] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x82] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x83] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x84] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x85] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x86] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x87] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x88] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x89] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x8a] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x8b] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x8c] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x8d] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x8e] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x8f] = &m8::Op_Jmp_k;		// 0x80-8f
	fpOp[0x90] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x91] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x92] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x93] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x94] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x95] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x96] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x97] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x98] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x99] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x9a] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x9b] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x9c] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x9d] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x9e] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0x9f] = &m8::Op_Call_k;		// 0x90-9f
	fpOp[0xa0] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa1] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa2] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa3] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa4] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa5] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa6] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa7] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa8] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xa9] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xaa] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xab] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xac] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xad] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xae] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xaf] = &m8::Op_Jz_k;			// 0xa0-af
	fpOp[0xb0] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb1] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb2] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb3] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb4] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb5] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb6] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb7] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb8] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xb9] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xba] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xbb] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xbc] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xbd] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xbe] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xbf] = &m8::Op_Jnz_k;		// 0xb0-bf
	fpOp[0xc0] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc1] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc2] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc3] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc4] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc5] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc6] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc7] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc8] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xc9] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xca] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xcb] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xcc] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xcd] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xce] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xcf] = &m8::Op_Jc_k;			// 0xc0-cf
	fpOp[0xd0] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd1] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd2] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd3] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd4] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd5] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd6] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd7] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd8] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xd9] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xda] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xdb] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xdc] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xdd] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xde] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xdf] = &m8::Op_Jnc_k;		// 0xd0-df
	fpOp[0xe0] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe1] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe2] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe3] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe4] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe5] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe6] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe7] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe8] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xe9] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xea] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xeb] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xec] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xed] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xee] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xef] = &m8::Op_Jacc_k;		// 0xe0-ef
	fpOp[0xf0] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf1] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf2] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf3] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf4] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf5] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf6] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf7] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf8] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xf9] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xfa] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xfb] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xfc] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xfd] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xfe] = &m8::Op_Index_k;		// 0xf0-ff
	fpOp[0xff] = &m8::Op_Index_k;		// 0xf0-ff

	fpOp[0x100] = &m8::Op_Interrupt;		

	TotalCPUClocks = 2;
}

void m8::Xres( void)
{
	eCurrentState = SOI;	

	A = 0;					
	SP = 0;					
	X = 0;
	T1 = 0;
	T2 = 0;
	ResetFlags();
	if (OCDMode) {
		IR = 0x40;				
		PC = 0xffff;			
        bIsResetNop = true;
	}
    else if (VersatestMode || DstestMode) {
        // Is this correct initialization for a tester run?
		IR = 0x40;				
		PC = 0xffff;			
        bIsResetNop = true;
    }
	else {
		// normal mode...

		// FIXME for real boot skipping... need command line variable...

//		IR = flash0.GetFlashByte(PC);
//		PC = 0x0000;			
		IR = 0x40;				
		PC = 0xffff;			
        bIsResetNop = true;
	}

	k = 0;
	i = 0;
	op_result = 0;
	big_result = 0;
	Saved_k = 0;
	Saved_i = 0;

	TracePtr = 0;

	//....

	IntPending = 0;
	PageCrossed = 0;
	CycleCount = 0;
	ReadOnce = false;
    BRQStallCount = 0;

	PCCycleCount = 0;
	StartStack = 0;
	MaxStack = 0;
	EndStack = 0;
	DoStackTrack = 0;
	
	ResetActive = false;
	ResetDelayClocks = 0;
}

void m8::AssertM8Reset(void) {

  Xres();

  ResetActive = true;
  ResetDelayClocks = 9;

}

void m8::ReleaseM8Reset (void) {

  // emulate the reset sequence
  ResetFlags();
  SP = 0;
  data_bus0.PostMemWrite(0x00, 0x00);
  IR = 0x40;

  TotalCPUClocks += 4;

  ResetActive = false;

}

bool m8::DoClockTick(ETickType eTick, bool *fDoUpdate) {

	bool bOpRet;
	bool tmpsoi, tmpfirst, tmpexec;
	int this_op;

	eCurrentTick = eTick;

	if (ResetDelayClocks == 0) {
	  if (eCurrentTick == POS_CPU) {
	    
	    *fDoUpdate = true;	// WARNING - this will bust if we ever add negedges to an op
	    
	    if (test_controller0.GetSLEEP() == 0) {
	      TotalCPUClocks++;
	    }
	    
	    if (TotalCPUClocks == MaxSimClocks) {
	      emu_log0.printf( "EmuCore:  ending simulation due to MaxSimClocks of %d\n", MaxSimClocks);
	      SimState = SIM_HALT;
	      return 0;
	    }
	    
	    if (PCCycleCount > 0) {
	      // if the count is greater than zero, we're counting - increment
	      PCCycleCount++;
	    }
	    
	  }
	  
	  if (eCurrentTick == NEG_CPU) {
	    // finish bus cycles, if any pending
	    data_bus0.CycleTerminateFall();
	  }
	  if (eCurrentTick == POS_CPU) {
	    // finish bus cycles, if any pending
	    data_bus0.CycleTerminateRise();
	  }
	  
	  // We can't move the opcode calling into POS_CPU only, because we have to generate dbdrv on NEG_CPU.  Bah.
	  // - craign
	  
	  // call current opcode
	  this_op = ((int) IR) | (IntPending << 8);
	  if (this_op > 0x100) {
	    this_op = this_op & 0xff;
	  }
	  bOpRet = (this->*fpOp[this_op]) ();
	  if (bOpRet) {
	    emu_log0.error( "Failure in opcode %x... aborting...\n", IR);
	    SimState = SIMERR_M8_OPFAIL;
	    return 1;
	  }
	  
	}
	else if (ResetActive == false) {
	  ResetDelayClocks--;
	}
	else {
	  // do nothing - wait for reset to go away
	}

	// HACK OPTIMIZE
	// we may be able to move the wave shit under POS_CPU
	// (be very, very careful!!!!!!!)
	// - craign

	if (WaveDumpActive) {
		// only screw with this if we want waves... otherwise don't waste the net processing time...
		if (eCurrentState == SOI) {
			tmpsoi = true;
			tmpfirst = false;
			tmpexec = false;
		}
		else if (eCurrentState == FIRST) {
			tmpsoi = false;
			tmpfirst = true;
			tmpexec = false;
		}
		else {
			// exec
			tmpsoi = false;
			tmpfirst = false;
			tmpexec = true;
		}
		SOINet.ApplyDigDrive(0, tmpsoi);
		FIRSTNet.ApplyDigDrive(0, tmpfirst);
		EXECNet.ApplyDigDrive(0, tmpexec);

		if (eCurrentTick == POS_CPU) {
    		CPUCLKNet.ApplyDigDrive(0, true);
		}
		else if (eCurrentTick == NEG_CPU) {
    		CPUCLKNet.ApplyDigDrive(0, false);
		}

		DumpM8Waves();
	}

	return 0;

}

void m8::CheckIntStatus(void) {

	if (GetGIE()) {
		if (int_ctl0.QueryIntStatus()) {
			// only ask if we really care - apparently the int controller only
			// wants to be asked if the m8 cares...
			IntPending = 1;
		}
		else {
			IntPending = 0;
		}
	}
	else {
		IntPending = 0;
	}

}

void m8::LogEmuTrace(void) {

	int loop;
	int byte;

	if (TraceOn) {

		std::ofstream traceout(pEmuTraceFilename, std::ios::app);

		TracePtr =0;

		if (!traceout) {
			emu_log0.printf( "EmuCore:  error dumping emu trace file %s\n", pEmuTraceFilename);
			return;
		}
	
//	traceout.seekp(0, ios::end);

		EmuTrace[TracePtr++] = ((TotalCPUClocks & 0xff000000)>>24);	
		EmuTrace[TracePtr++] = ((TotalCPUClocks & 0x00ff0000)>>16);	
		EmuTrace[TracePtr++] = ((TotalCPUClocks & 0x0000ff00)>>8);	
		EmuTrace[TracePtr++] = ((TotalCPUClocks & 0x000000ff));	
		EmuTrace[TracePtr++] = ((PC & 0xff00)>>8);
		EmuTrace[TracePtr++] = ((PC & 0x00ff));
		EmuTrace[TracePtr++] = IR;
		EmuTrace[TracePtr++] = A;
		EmuTrace[TracePtr++] = SP;
		EmuTrace[TracePtr++] = X;
		EmuTrace[TracePtr++] = FL;

		byte = 0;

		for (loop = 0; loop < 11; loop++) {
			unsigned int tmp = EmuTrace[loop];
			traceout.width(2);
			traceout.fill('0');
			traceout << std::hex << tmp;
			traceout << " ";
			byte++;
			if (byte == 11) {
				traceout << "\n";
				byte = 0;
			}
		}

		traceout.close();

		return;
	}
}

void m8::DumpM8Waves(void) {

	if (WaveDumpActive) {
		dump_waves0.vPostVariableChange("M8", "TotalCPUClocks[31:0]", TotalCPUClocks);	
		dump_waves0.vPostVariableChange("M8", "PC[15:0]", PC);	
		int iA = A;
		dump_waves0.vPostVariableChange("M8", "A[7:0]", iA);
		int iSP = SP;
		dump_waves0.vPostVariableChange("M8", "SP[7:0]", iSP);	
		int iX = X;
		dump_waves0.vPostVariableChange("M8", "X[7:0]", iX);	
		int iFL = FL;
		dump_waves0.vPostVariableChange("M8", "FL[7:0]", iFL);	
		int iIR = IR;
		dump_waves0.vPostVariableChange("M8", "IR[7:0]", iIR);	
	}
}

bool m8::IOReadSniffer(unsigned char addr, unsigned char *data, int *ack) {

	*ack = 0;	// default is no response


	if (OCDMode == 1 || VersatestMode != 0 || DstestMode != 0) {
		// only allow access in ocd mode or tester runs.
		switch (addr) {
		case 0xf0:
			*data = A;
			*ack = 1;
			break;
		case 0xf1:
			*data = T1;
			*ack = 1;
			break;
		case 0xf2:
			*data = T2;
			*ack = 1;
			break;
		case 0xf3:
			*data = X;
			*ack = 1;
			break;
		case 0xf4:
			*data = GetPCL();
			*ack = 1;
			break;
		case 0xf5:
			*data = GetPCH();
			*ack = 1;
			break;
		case 0xf6:
			*data = SP;
			*ack = 1;
			break;
		case 0xf7:
			*data = FL;	// HACK - this won't work for all flags
			*ack = 1;
			break;
		}
	}
	else {
		switch (addr) {
		case 0xf0:
		case 0xf1:
		case 0xf2:
		case 0xf3:
		case 0xf4:
		case 0xf5:
		case 0xf6:
		case 0xf7:
		  *data = 0;
		  *ack = 1;
		  break;
		}
	}

	return 0;
}

bool m8::IOWriteSniffer(unsigned char addr, unsigned char data, int *ack) {

	*ack = 0;	// default is no response

	if (OCDMode == 1 || VersatestMode != 0 || DstestMode != 0) {
		// only allow access in ocd mode or tester runs.

		if (GetIOX() == 0) {
			// IOX is off
			switch (addr) {
			case 0xf0:
				A = data;
				*ack = 1;
				break;
			case 0xf1:
				T1 = data;
				*ack = 1;
				break;
			case 0xf2:
				T2 = data;
				*ack = 1;
				break;
			case 0xf3:
				X = data;
				*ack = 1;
				break;
			case 0xf4:
				PC = PC & 0xff00;
				PC = PC | data;
				*ack = 1;
				break;
			case 0xf5:
				PC = PC & 0x00ff;
				PC = PC | ((short int) data << 8);
				*ack = 1;
				break;
			case 0xf6:
				SP = data;
				*ack = 1;
				break;			
			case 0xf7:
				FL = data;	// HACK - this won't work for all flags
				*ack = 1;
				break;
				
			}
		}
		else {
			// IOX is on
			switch (addr) {
			case 0xf0:
				A = data;
				*ack = 1;
				break;
			case 0xf1:
				T1 = data;
				*ack = 1;
				break;
			case 0xf2:
				T2 = data;
				*ack = 1;
				break;
			case 0xf3:
				X = data;
				*ack = 1;
				break;
			case 0xf4:
				PC = PC & 0xff00;
				PC = PC | data;
				*ack = 1;
				break;
			case 0xf5:
				PC = PC & 0x00ff;
				PC = PC | ((short int) data << 8);
				*ack = 1;
				break;
			case 0xf6:
				SP = data;
				*ack = 1;
				break;			
			case 0xf7:
				FL = data;	// HACK - this won't work for all flags
				*ack = 1;
				break;				
			}
		}
	}

	return 0;
}

bool m8::IsIntPending(void) {

  return IntPending;
}

bool m8::IsHalted( void) const 
{
    bool bHalted = false;

    if ((eCurrentState == SOI) && test_controller0.GetStop())
    {
        bHalted = true;
    }

    return bHalted;
}


void m8::ResetFlags(void) {

	FL = 2;	// HACK - this is the reset state

}

bool m8::ThisOpIsStack(void) {

	switch (IR) {
	case 0x00:
		if (eCurrentState != EXEC) {
			return 1;
		}
		else {
			return 0;	// don't return stack for flash setup operation
		}
	case 0x08:
		return 1;
	case 0x10:
		return 1;
	case 0x18:
		return 1;
	case 0x20:
		return 1;
	case 0x7c:
		return 1;
	case 0x7e:
		if (eCurrentState != EXEC) {
			return 1;
		}
		else {
			return 0;	// don't return stack for flash setup/fixup operations
		}
	case 0x7f:
		return 1;
	case 0x90:
		return 1;
	case 0x91:
		return 1;
	case 0x92:
		return 1;
	case 0x93:
		return 1;
	case 0x94:
		return 1;
	case 0x95:
		return 1;
	case 0x96:
		return 1;
	case 0x97:
		return 1;
	case 0x98:
		return 1;
	case 0x99:
		return 1;
	case 0x9a:
		return 1;
	case 0x9b:
		return 1;
	case 0x9c:
		return 1;
	case 0x9d:
		return 1;
	case 0x9e:
		return 1;
	case 0x9f:
		return 1;
	default:
		return 0;
	}

}

bool m8::ThisOpIsIndex(void) {

	// These are opcodes that access memory via the X register.

	switch (IR) {
	case 0x03:
		return 1;
	case 0x05:
		return 1;
	case 0x07:
		return 1;
	case 0x0b:
		return 1;
	case 0x0d:
		return 1;
	case 0x0f:
		return 1;
	case 0x13:
		return 1;
	case 0x15:
		return 1;
	case 0x17:
		return 1;
	case 0x1b:
		return 1;
	case 0x1d:
		return 1;
	case 0x1f:
		return 1;
	case 0x23:
		return 1;
	case 0x25:
		return 1;
	case 0x27:
		return 1;
	case 0x2b:
		return 1;
	case 0x2d:
		return 1;
	case 0x2f:
		return 1;
	case 0x33:
		return 1;
	case 0x35:
		return 1;
	case 0x37:
		return 1;
	case 0x3b:
		return 1;
	case 0x3d:
		return 1;
	case 0x48:
		return 1;
	case 0x52:
		return 1;
	case 0x54:
		return 1;
	case 0x56:
		return 1;
	case 0x59:
		return 1;
	case 0x66:
		return 1;
	case 0x69:
		return 1;
	case 0x6c:
		return 1;
	case 0x6f:
		return 1;
	case 0x77:
		return 1;
	case 0x7b:
		return 1;
	default:
		return 0;
	}
}

bool m8::IsMVIOp(void) {

	if ((IR == 0x3e) || (IR == 0x3f)) {
		return 1;
	}
	else {
		return 0;
	}
}

bool m8::IsMVIToAccum(void) {

	if (IR == 0x3e) {
		return 1;
	}
	else {
		return 0;
	}
}

void m8::DoPCCounter(void) {

	if (CycleCounterActive) {
		// pc cycle count command line option is on... check for point crossings...
		// look for start/endpoints
		if (((PC == CycleCounterPCStart) && (GetSup() == 0) && (PCCycleCount == 0)) || 
			((PC == (CycleCounterPCStart & 0x3fff)) && (GetSup() == 1) && (PCCycleCount == 0) && (CycleCounterPCStart > 0x7fff))) {
			// set to count 1 - found start point
			PCCycleCount = 1;
			emu_log0.printf( "EmuCore:  information - found PC startpoint at time PosCPU = 0x%x\n", TotalCPUClocks);
		}
		else if (((PC == CycleCounterPCEnd) && (GetSup() == 0) && (PCCycleCount > 1)) ||
			     ((PC == (CycleCounterPCEnd & 0x3fff)) && (GetSup() == 1) && (PCCycleCount > 1) && (CycleCounterPCEnd > 0x7fff))) {
			emu_log0.printf( "EmuCore:  information - found PC endpoint at time PosCPU = 0x%x\n", TotalCPUClocks);
			emu_log0.printf( "EmuCore:  information - total CPU cycles between points = %d (decimal)\n", (PCCycleCount-1));
			PCCycleCount = 0;
		}
	}
}

void m8::DoStackCounter(void) {

	if (StackTrackerActive) {
		if (((PC == StackTrackerPCStart) && (GetSup() == 0) && (DoStackTrack == 0)) ||
			((PC == (StackTrackerPCStart & 0x3fff)) && (GetSup() == 1) && (DoStackTrack == 0) && (StackTrackerPCStart > 0x7fff))) {
			DoStackTrack = 1;
			StartStack = SP;
			MaxStack = SP;
			emu_log0.printf( "EmuCore:  information - found stack startpoint at time PosCPU = 0x%x\n", TotalCPUClocks);
		}
		else if (((PC == StackTrackerPCEnd) && (GetSup() == 0) && (DoStackTrack == 1)) ||
			     ((PC == (StackTrackerPCEnd & 0x3fff)) && (GetSup() == 1) && (DoStackTrack == 1) && (StackTrackerPCEnd > 0x7fff))) {
			EndStack = SP;
			DoStackTrack = 0;
			emu_log0.printf( "EmuCore:  information - found stack endpoint at time PosCPU = 0x%x\n", TotalCPUClocks);
			emu_log0.printf( "EmuCore:  information - start stack pointer was 0x%x\n", StartStack);
			emu_log0.printf( "EmuCore:  information - end   stack pointer was 0x%x\n", EndStack);
			emu_log0.printf( "EmuCore:  information - max   stack pointer was 0x%x\n", MaxStack);
		}
		else if (DoStackTrack) {
			if (SP > MaxStack) {
				MaxStack = SP;
			}
		}
	}
}

bool m8::ProcessSOI(void) {
    
    if (test_controller0.GetSLEEP() == false) {
        
        
        if (CycleCount == 0) {
            if (test_controller0.GetBRQ() == true) {
                BRQStallCount = 1; 
                dump_waves0.vPostVariableChange("M8", "BRQStall[31:0]", BRQStallCount);	
                // For a total of 3 CPU clock delays after test controller releases BRQ.
            }
            else if (BRQStallCount > 0) {
                --BRQStallCount;
                dump_waves0.vPostVariableChange("M8", "BRQStall[31:0]", BRQStallCount);	
            }
            else {
                LogEmuTrace();
                if (VerboseLevel) {
                    emu_log0.printf( "IR=0x%02x A=0x%02x X=0x%02x SP=0x%02x FL=0x%02x PC=0x%04x PosCPU=%x\n",
                        IR, A, X, SP, FL, PC, TotalCPUClocks);
                }
                unsigned char Data = data_bus0.SniffEvil();
                ice0.LogTraceEntry( A, X, SP, FL, PC, TotalCPUClocks, Data);
                if ((IR == 0x40) && (PC == 0x0000) && (!test_controller0.GetTMODE())) {
                    FL |= SUP_FLAG;	// force sup - really odd bit of hardware... *sigh*
                }
                if (bIsResetNop) {
                    bIsResetNop = false;
                }
                else {
                    flash0.LogProfileEntry( PC);
                }
                
                DoPCCounter();
                DoStackCounter();
                if (SP > StackMax) {
                    emu_log0.error( "SP=%02x hit stack max limit %02x", SP, StackMax);
                    SimState = SIMERR_SP_LIMIT;
                }
                if (SP != 0) {
                    IsStackMinActive = true;
                }
                if (IsStackMinActive && SP < StackMin) {
                    emu_log0.error( "SP=%02x passed stack min limit %02x", SP, StackMin);
                    SimState = SIMERR_SP_LIMIT;
                }
                
                CycleCount++;
            }
            ReadOnce = false;
        }
        else if (CycleCount == 1) {
            if (PCL_FF()) {
                CycleCount++;			
                if (VerboseLevel) {
                    emu_log0.printf( "page crossing in soi...\n");
                }
            }
            else {
                CycleCount = 0;
                eCurrentState = FIRST;
            }
            PC++;
        }
        else {	// CycleCount == 2
            CycleCount = 0;
            eCurrentState = FIRST;
        }
    }
    
    return 0;
}

void m8::DBDRVBitesMyAss(void) {

	// make it blip on the waves
	DBDRVNet.ApplyDigDrive(0, 0);
	DBDRVNet.ApplyDigDrive(0, 1);

	// tell the digblock about it...
	dig_top0_next.IJustLoveGettingDBDRV();
}

bool m8::PCL_FF(void) {

	if ((PC & 0x00ff) == 0x00ff) {
		return 1;
	}
	else {
		return 0;
	}
}

bool m8::PCL_0(void) {

	if ((PC & 0x00ff) == 0) {
		return 1;
	}
	else {
		return 0;
	}
}

unsigned int m8::ConcatPC(unsigned char upper, unsigned char lower) {

	return (((unsigned int) upper << 8) | ((unsigned int) lower));

}

void m8::SSCSetA(unsigned char data) {

	A = data;
}

void m8::ICESetIR( unsigned char data)
{
    IR = data;
}

unsigned char m8::GetA(void) {

	return A;
}

void m8::SSCSetX(unsigned char data) {

	X = data;
}

unsigned char m8::GetSP(void) {

	return SP;
}

unsigned char m8::GetPCL(void) {

	return ((unsigned char ) PC & 0x00ff);
}

unsigned char m8::GetPCH(void) {

	return ((unsigned char) ((PC >> 8) & 0x00ff));

}

unsigned short m8::GetPC( void)
{
    return PC;
}
    
unsigned char m8::GetFL( void)
{
    return FL;
}

bool m8::GetPMODE(void) {

	unsigned char tmp;

	tmp = FL & PMODE_FLAG;

	if (tmp == 0) {
		return 0;
	}
	else {
		return 1;
	}

}

bool m8::GetXPPMODE(void) {

	unsigned char tmp;

	tmp = FL & XPPMODE_FLAG;

	if (tmp == 0) {
		return 0;
	}
	else {
		return 1;
	}

}

unsigned char m8::GetGIE(void) {

	unsigned char tmp;

	tmp = FL & IE_FLAG;

	if (tmp == 0) {
		return 0;
	}
	else {
		return 1;
	}

}

unsigned char m8::GetSup(void) {

	unsigned char tmp;

	tmp = FL & SUP_FLAG;

	if (tmp == 0) {
		return 0;
	}
	else {
		return 1;
	}

}

unsigned char m8::GetCarry(void) {

	unsigned char tmp;

	// Convert the carry flag into a zero or one for math ops.

	tmp = FL & CARRY_FLAG;

	if (tmp == 0) {
		return 0;
	}
	else {
		return 1;
	}

}

unsigned char m8::GetZero(void) {

	unsigned char tmp;

	// Convert the carry flag into a zero or one for math ops.

	tmp = FL & ZERO_FLAG;

	if (tmp == 0) {
		return 0;
	}
	else {
		return 1;
	}

}

unsigned char m8::GetIOX(void) {

	unsigned char tmp;

	// Convert the carry flag into a zero or one for math ops.

	tmp = FL & IOX_FLAG;

	if (tmp == 0) {
		return 0;
	}
	else {
		return 1;
	}

}

void m8::SetCarryIfNegOne(unsigned char res) {

	if (res == 0xff) {
		FL |= CARRY_FLAG;
	}
	else {
		FL &= ~CARRY_FLAG;
	}

}

void m8::SetCarryIfMSB(unsigned char res) {

	unsigned char tmp;

	tmp = res & 0x80;

	if (tmp == 0x80) {
		FL |= CARRY_FLAG;
	}
	else {
		FL &= ~CARRY_FLAG;
	}

}

void m8::SetCarryIfLSB(unsigned char res) {

	unsigned char tmp;

	tmp = res & 0x01;

	if (tmp == 0x01) {
		FL |= CARRY_FLAG;
	}
	else {
		FL &= ~CARRY_FLAG;
	}

}

void m8::SetCarryIfNeg(int bigres) {

	if (bigres < 0) {
		FL |= CARRY_FLAG;
	}
	else {
		FL &= ~CARRY_FLAG;
	}

}

void m8::SetCarryIfOver255(int bigres) {

	if (bigres > 0xff) {
		FL |= CARRY_FLAG;
	}
	else {
		FL &= ~CARRY_FLAG;
	}

}

void m8::SetCarryIfZero(unsigned char res) {

	if (res == 0x00) {
		FL |= CARRY_FLAG;
	}
	else {
		FL &= ~CARRY_FLAG;
	}

}

void m8::SetZeroIfZero(unsigned char res) {

	if (res == 0x00) {
		FL |= ZERO_FLAG;
	}
	else {
		FL &= ~ZERO_FLAG;
	}
}

unsigned int m8::ComputeTwelveBitTarg(unsigned char upper, unsigned char lower) {

	unsigned int res;

	res = (unsigned int) upper;
	res = res << 8;
	res = res & 0x0f00;
	res = res | ((unsigned int) lower);
	if ((res & 0x0800) != 0) {
		// sign extend if negative
		res = res | 0xf000;
	}
	res = res + PC;
	res = res - 1;	// twos comp

	return res;
}

unsigned int m8::ComputeTwelveBitTargNoSub(unsigned char upper, unsigned char lower) {

	unsigned int res;

	res = (unsigned int) upper;
	res = res << 8;
	res = res & 0x0f00;
	res = res | ((unsigned int) lower);
	if ((res & 0x0800) != 0) {
		// sign extend if negative
		res = res | 0xf000;
	}
	res = res + PC;

	return res;
}

unsigned int m8::ComputeTwelveBitTargNoSubIndex(unsigned char upper, unsigned char lower, unsigned char extra) {

	unsigned int res;

	res = (unsigned int) upper;
	res = res << 8;
	res = res & 0x0f00;
	res = res | ((unsigned int) lower);
	if ((res & 0x0800) != 0) {
		// sign extend if negative
		res = res | 0xf000;
	}
	res = res + PC + extra;

	return res;
}

bool m8::BumpPC(void) {

	// return 1 if a break is needed to force an extra cycle

	if (PCL_FF()) {
		if (PageCrossed) {
			emu_log0.printf( "EmuCore:  page hairball!\n");
			PC++;
			Saved_k = k;
			Saved_i = i;
			PageCrossed = 1;
			return 1;
		}
		else {
			if (VerboseLevel) {
				emu_log0.printf( "page crossing NOT in soi...\n");
			}
			PC++;
			Saved_k = k;
			Saved_i = i;
			PageCrossed = 1;
			return 1;
		}
	}
	else if (PCL_0()) {
		if (PageCrossed) {
			PageCrossed = 0;
			k = Saved_k;
			i = Saved_i;
			return 0;
		}
		else {
			PC++;
			PageCrossed = 0;
			return 0;
		}
	}
	else {
		// normal bump
		PC++;
		PageCrossed = 0;
		return 0;
	}

}

void m8::EndOp(void) {

	CycleCount = 0;	// reset - end of instruction, start new count

	// Tell the SRAM to let the autopops take effect.
	sram0.CycleTerminateAPOP();

	if (IntPending) {
		// handle int case
		eCurrentState = FIRST;
		IR = 0;	// int pending will set correct opcode in do_clock_tick
		int_ctl0_next.SetIRA();
	}
	else {

        if (ice0.IsBreakpoint( PC)) {
            test_controller0_next.SetStop();
        }

		// normal transition
		eCurrentState = SOI;
		IR = flash0.GetFlashByte(PC);
	}

}

bool m8::Op_Interrupt(void) {

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		// note - interrupts don't have an soi phase...
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), GetPCH())) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), GetPCL())) {
					return 1;
				}
			}
			else if (CycleCount == 6) {
				// end of seventh FIRST
				PC = int_ctl0_next.QueryIntVector();	// int pending flag will keep us here for a bit...
				int_ctl0_next.SetIVR();
				CycleCount++;
			}
			else if (CycleCount == 7) {
				// end of eigth FIRST
				CycleCount++;
				int_ctl0_next.ClearIVR();
			}
			else if (CycleCount == 8) {
				// end of ninth FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 9) {
				// end of tenth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), (FL & 0xf7))) {	// don't push sup bit... :)
					return 1;
				}
			}
			else if (CycleCount == 10) {
				// end of eleventh FIRST
				CycleCount++;
			}
			else if (CycleCount == 11) {
				// end of last FIRST
				int_ctl0_next.ClearIRA();
				CycleCount = 0;
				eCurrentState = EXEC;
			}
			break;
		case EXEC:

			// set flags
			FL = 0;
			FL = FL & ~IE_FLAG;

			IntPending = 0;
			CycleCount = 0;
			eCurrentState = SOI;
			IR = flash0.GetFlashByte(PC);

			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;



}

bool m8::Op_SSC(void) {


	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
            if (eCurrentState == FIRST) {
                // SUP_FLAG goes high in SSC at beginning of first FIRST.
                FL |= SUP_FLAG;
		        int iFL = FL;
		        dump_waves0.vPostVariableChange("M8", "FL[7:0]", iFL);
                test_controller0_next.SetSup( true);
            }
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				CycleCount++;
				FL = FL | SUP_FLAG;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), GetPCH())) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), GetPCL())) {
					return 1;
				}
			}
			else if (CycleCount == 6) {
				// end of seventh FIRST
				CycleCount++;
			}
			else if (CycleCount == 7) {
				// end of eigth FIRST
				CycleCount++;
				PC = 0;
			}
			else if (CycleCount == 8) {
				// end of ninth FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 9) {
				// end of tenth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), (FL & 0xf7))) {	// don't push sup bit... :)
					return 1;
				}
			}
			else if (CycleCount == 10) {
				// end of eleventh FIRST
				CycleCount++;
			}
			else if (CycleCount == 11) {
				// end of last FIRST
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
			}
			break;
		case EXEC:

			// set flags
			FL = 0;
			FL = FL | SUP_FLAG;

			// tell flash to store information about the pending supervisory op...
			flash0.StartSupOp();

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;


}

bool m8::Op_Add_A_k(void){		// 0x01

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			op_result = A + k;
			big_result = ((int) A) + ((int) k);
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();
			break;

		case EXEC:
			
			// set flags
			SetCarryIfOver255(big_result);
			SetZeroIfZero(op_result);

			A = op_result;	// store result
			
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Add_A_Mk(void){		// 0x02

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead(k, &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) A);
				op_result = op_result + A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetCarryIfOver255(big_result);
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Add_A_MXpk(void){	// 0x03

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) A);
				op_result = op_result + A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetCarryIfOver255(big_result);
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Add_Mk_A(void){		// 0x04

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) A);
				op_result = op_result + A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfOver255(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Add_MXpk_A(void){	// 0x05

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) A);
				op_result = op_result + A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfOver255(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Add_Mk_i(void){		// 0x06

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) i);
				op_result = op_result + i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfOver255(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Add_MXpk_i(void){	// 0x07

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) i);
				op_result = op_result + i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfOver255(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Push_A(void){		// 0x08

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = A;
			SP++;				// SP is updated here in the chip, but the post happens with the old address...
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			if (data_bus0.PostMemWrite((SP - 1), op_result)) { // ... hence the -1 here.
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Adc_A_k(void){		// 0x09

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			op_result = A + k + GetCarry();
			big_result = ((int) A) + ((int) k) + ((int) GetCarry());
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetCarryIfOver255(big_result);
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Adc_A_Mk(void){		// 0x0a

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead(k, &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) A) + ((int) GetCarry());
				op_result = op_result + A + GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetCarryIfOver255(big_result);
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Adc_A_MXpk(void){	// 0x0b

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) A) + ((int) GetCarry());
				op_result = op_result + A + GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
	
			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetCarryIfOver255(big_result);
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Adc_Mk_A(void){		// 0x0c

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) A) + ((int) GetCarry());
				op_result = op_result + A + GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle

				CycleCount++;

				// set flags
				SetCarryIfOver255(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Adc_MXpk_A(void){	// 0x0d

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) A) + ((int) GetCarry());
				op_result = op_result + A + GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfOver255(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Adc_Mk_i(void){		// 0x0e

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) i) + ((int) GetCarry());
				op_result = op_result + i + GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfOver255(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Adc_MXpk_i(void){	// 0x0f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				big_result = ((int) op_result) + ((int) i) + ((int) GetCarry());
				op_result = op_result + i + GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfOver255(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Push_X(void){		// 0x10
	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = X;
			SP++;				// SP is updated here in the chip, but the post happens with the old address...
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			if (data_bus0.PostMemWrite((SP - 1), op_result)) { // ... hence the -1 here.
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Sub_A_k(void){		// 0x11

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			op_result = A - k;
			big_result = ((int) A) - ((int) k);
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Sub_A_Mk(void){		// 0x12

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) A) - ((int) op_result);
				op_result = A - op_result;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Sub_A_MXpk(void){	// 0x13

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) A) - ((int) op_result);
				op_result = A - op_result;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Sub_Mk_A(void){		// 0x14

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) A);
				op_result = op_result - A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:
			
			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfNeg(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Sub_MXpk_A(void){	// 0x15

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) A);
				op_result = op_result - A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfNeg(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Sub_Mk_i(void){		// 0x16

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) i);
				op_result = op_result - i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfNeg(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Sub_MXpk_i(void){	// 0x17

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) i);
				op_result = op_result - i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
	
			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfNeg(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Pop_A(void){		// 0x18

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				CycleCount++;
				SP--;	
			}
			else {
				// end of second FIRST
				if (data_bus0.PostMemRead((SP), &op_result)) {
					return 1;										
				}
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			SetZeroIfZero(op_result);
			A = op_result;
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Sbb_A_k(void){		// 0x19

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			big_result = ((int) A) - ((int) k) - ((int) GetCarry());
			op_result = A - k - GetCarry();
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);

			A = op_result;	// store result
			
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Sbb_A_Mk(void){		// 0x1a

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) A) - ((int) op_result) - ((int) GetCarry());
				op_result = A - op_result - GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Sbb_A_MXpk(void){	// 0x1b

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) A) - ((int) op_result)  - ((int) GetCarry());
				op_result = A - op_result - GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Sbb_Mk_A(void){		// 0x1c

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) A) - ((int) GetCarry());
				op_result = op_result - A - GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle

				CycleCount++;

				// set flags
				SetCarryIfNeg(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Sbb_MXpk_A(void){	// 0x1d

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) A) - ((int) GetCarry());
				op_result = op_result - A - GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle

				CycleCount++;

				// set flags
				SetCarryIfNeg(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Sbb_Mk_i(void){		// 0x1e

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) i) - ((int) GetCarry());
				op_result = op_result - i - GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle

				CycleCount++;

				// set flags
				SetCarryIfNeg(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Sbb_MXpk_i(void){	// 0x1f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) i) - ((int) GetCarry());
				op_result = op_result - i - GetCarry();
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle

				CycleCount++;

				// set flags
				SetCarryIfNeg(big_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Pop_X(void){		// 0x20

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				CycleCount++;
				SP--;	
			}
			else {
				// end of second FIRST
				if (data_bus0.PostMemRead((SP), &op_result)) {
					return 1;
				}
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
			}
			break;
		case EXEC:

			X = op_result;
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_And_A_k(void){		// 0x21

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			op_result = A & k;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_And_A_Mk(void){		// 0x22

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead(k, &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = op_result & A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_And_A_MXpk(void){	// 0x23

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				op_result = op_result & A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_And_Mk_A(void){		// 0x24

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = op_result & A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_And_MXpk_A(void){	// 0x25

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				op_result = op_result & A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_And_Mk_i(void){		// 0x26

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = op_result & i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_And_MXpk_i(void){	// 0x27

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = op_result & i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
	
			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Romx(void){			// 0x28

	int romx_addr;

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
				romx_addr =  (((int) A) << 8) | ((int) X) ;
				op_result = flash0.GetFlashByte(romx_addr);
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
			}
			else if (CycleCount == 6) {
				// end of seventh FIRST
				CycleCount++;
			}
			else {
				// end of eighth FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;
			SetZeroIfZero(op_result);
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Or_A_k(void){		// 0x29

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			op_result = A | k;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Or_A_Mk(void){		// 0x2a

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead(k, &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = op_result | A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Or_A_MXpk(void){	// 0x2b

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				op_result = op_result | A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Or_Mk_A(void){		// 0x2c

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = op_result | A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Or_MXpk_A(void){	// 0x2d

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				op_result = op_result | A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Or_Mk_i(void){		// 0x2e

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = op_result | i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Or_MXpk_i(void){	// 0x2f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = op_result | i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
	
			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Halt(void){			// 0x30

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
            CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();
			break;
		case EXEC:
			if (CycleCount == 0) {
				// end of first EXEC
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second EXEC
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third EXEC
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth EXEC
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth EXEC
				CycleCount++;
			}
			else {
				// end of sixth EXEC cycle... go to SOI
    	        test_controller0_next.SetStop();
    			EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

    return 0;
}


bool m8::Op_Xor_A_k(void){		// 0x31

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			op_result = A ^ k;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();

			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Xor_A_Mk(void){		// 0x32

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead(k, &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = op_result ^ A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Xor_A_MXpk(void){	// 0x33

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				op_result = op_result ^ A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Xor_Mk_A(void){		// 0x34

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = op_result ^ A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Xor_MXpk_A(void){	// 0x35

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				op_result = op_result ^ A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Xor_Mk_i(void){		// 0x36

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = op_result ^ i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Xor_MXpk_i(void){	// 0x37

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = op_result ^ i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Add_SP_k(void){		// 0x38

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of third FIRST cycle... go to EXEC
				CycleCount = 0;
				op_result = SP + k;
				SP = op_result;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Cmp_A_k(void){		// 0x39

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
				k = flash0.GetFlashByte(PC);
				if (BumpPC()) break;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
				break;
		case EXEC:

		  if (CycleCount == 0) {
		    // first exec
		    op_result = A - k;
		    big_result = ((int) A) - ((int) k);
		    CycleCount++;
		  }
		  else {
		    // second exec

		    // set flags
		    SetCarryIfNeg(big_result);
		    SetZeroIfZero(op_result);

		    EndOp();
		  }
		  break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Cmp_A_Mk(void){		// 0x3a

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				big_result = ((int) A) - ((int) op_result);
				op_result = A - op_result;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:
		  if (CycleCount == 0) {
			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);
			CycleCount++;
		  }
		  else {
			EndOp();
		  }
		  break;

		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;


}

bool m8::Op_Cmp_A_MXpk(void){	// 0x3b

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				big_result = ((int) A) - ((int) op_result);
				op_result = A - op_result;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:
		  if (CycleCount == 0) {
			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);
			CycleCount++;
		  }
		  else {
			EndOp();
		  }
		  break;

		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Cmp_Mk_i(void){		// 0x3c

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) i);
				op_result = op_result - i;
				CycleCount = 0;
				eCurrentState = EXEC;
	
				CheckIntStatus();

			}
			break;
		case EXEC:

			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Cmp_MXpk_i(void){	// 0x3d

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				big_result = ((int) op_result) - ((int) i);
				op_result = op_result - i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			// set flags
			SetCarryIfNeg(big_result);
			SetZeroIfZero(op_result);

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mvi_A_MMkpp(void){	// 0x3e

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((k), (op_result + 1))) {
					return 1;
				}
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
			}
			else {
				// end of seventh FIRST cycle... go to EXEC
				if (data_bus0.PostMemRead((op_result), &op_result)) {
					return 1;
				}
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mvi_MMkpp_A(void){	// 0x3f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((k), (op_result + 1))) {
					return 1;
				}
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
			}
			else {
				// end of seventh FIRST cycle... go to EXEC
				if (data_bus0.PostMemWrite((op_result), A)) {
					return 1;
				}
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Nop(void){			// 0x40

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_And_IOk_i(void){	// 0x41

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				data_bus0.PostIOReadPrecharge(k);
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				if (ReadOnce == false) {
					if (data_bus0.PostIORead((k), &op_result)) {
						return 1;
					}
				}
				ReadOnce = true;
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = op_result & i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostIOWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if (CycleCount == 2) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}

	return 0;
}

bool m8::Op_And_IOXpk_i(void){	// 0x42

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				data_bus0.PostIOReadPrecharge(X+k);
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				if (data_bus0.PostIORead((X+k), &op_result)) {
					return 1;
				}
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = op_result & i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostIOWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if (CycleCount == 3) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Or_IOk_i(void){		// 0x43

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				data_bus0.PostIOReadPrecharge(k);
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				if (ReadOnce == false) {
					if (data_bus0.PostIORead((k), &op_result)) {
						return 1;
					}
				}
				ReadOnce = true;
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = op_result | i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostIOWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if (CycleCount == 2) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Or_IOXpk_i(void){	// 0x44

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				data_bus0.PostIOReadPrecharge(X+k);
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				if (data_bus0.PostIORead((X+k), &op_result)) {
					return 1;
				}
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = op_result | i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostIOWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if (CycleCount == 3) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Xor_IOk_i(void){	// 0x45

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				data_bus0.PostIOReadPrecharge(k);
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				if (ReadOnce == false) {
					if (data_bus0.PostIORead((k), &op_result)) {
						return 1;
					}
				}
				ReadOnce = true;
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = op_result ^ i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostIOWrite((k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if (CycleCount == 2) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Xor_IOXpk_i(void){	// 0x46

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				data_bus0.PostIOReadPrecharge(X+k);
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
				if (data_bus0.PostIORead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = op_result ^ i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostIOWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if (CycleCount == 3) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Tst_Mk_i(void){		// 0x47

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = op_result & i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			// set flags
			SetZeroIfZero(op_result);

			EndOp();

			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Tst_MXpk_i(void){	// 0x48

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = op_result & i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Tst_IOk_i(void){	// 0x49

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				data_bus0.PostIOReadPrecharge(k);
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				if (ReadOnce == false) {
					if (data_bus0.PostIORead((k), &op_result)) {
						return 1;
					}
				}
				ReadOnce = true;
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = op_result & i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if (CycleCount == 2) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Tst_IOXpk_i(void){	// 0x4a

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				data_bus0.PostIOReadPrecharge(X+k);
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				if (data_bus0.PostIORead((X+k), &op_result)) {
					return 1;
				}
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = op_result & i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if (CycleCount == 3) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Swap_A_X(void){		// 0x4b

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			// end of first FIRST
			CycleCount = 0;
			eCurrentState = EXEC;
			op_result = A;
			CheckIntStatus();

			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				A = X;

				// set flags
				SetZeroIfZero(A);
			}
			else {
				// second EXEC cycle

				X = op_result;	// weird one... op_result used as temp storage, but end A determines flags
				
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Swap_A_Mk(void){	// 0x4c

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				if (data_bus0.PostMemWrite((k), A)) {
					return 1;
				}

			}
			else {
				// second EXEC cycle

				A = op_result;

				// set flags
				SetZeroIfZero(op_result);

				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Swap_X_Mk(void){	// 0x4d

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				if (data_bus0.PostMemWrite((k), X)) {
					return 1;
				}

			}
			else {
				// second EXEC cycle

				X = op_result;

				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Swap_A_SP(void){	// 0x4e

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			// end of first FIRST
			CycleCount = 0;
			eCurrentState = EXEC;
			op_result = A;
			CheckIntStatus();

			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				A = SP;

				// set flags
				SetZeroIfZero(A);
			}
			else {
				// second EXEC cycle

				SP = op_result;	// flags based on dest A, op_result is temp storage
				
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mov_X_SP(void){		// 0x4f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = SP;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			X = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_A_k(void){		// 0x50

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			op_result = k;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_A_Mk(void){		// 0x51

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of second FIRST
				if (data_bus0.PostMemRead(k, &op_result)) {
					return 1;
				}
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_A_MXpk(void){	// 0x52

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else {
				// end of third FIRST
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			A = op_result;

			// set flags
			SetZeroIfZero(op_result);

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_Mk_A(void){		// 0x53

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostMemWrite(k, op_result)) {
				return 1;
			}

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;


}

bool m8::Op_Mov_MXpk_A(void){	// 0x54

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else {
				// end of third FIRST
				op_result = A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostMemWrite((X+k), op_result)) {
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mov_Mk_i(void){		// 0x55

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostMemWrite((k), op_result)) {
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mov_MXpk_i(void){	// 0x56

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostMemWrite((X+k), op_result)) {
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mov_X_k(void){		// 0x57

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			op_result = k;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			X = op_result;	// store result
			
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_X_Mk(void){		// 0x58

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead(k, &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			X = op_result;

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_X_MXpk(void){	// 0x59

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			X = op_result;

			EndOp();
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_Mk_X(void){		// 0x5a	

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = X;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
	
			}
			break;
		case EXEC:

			if (data_bus0.PostMemWrite(k, op_result)) {
				return 1;
			}

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_A_X(void){		// 0x5b

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = X;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetZeroIfZero(op_result);

			A = op_result;	// store result
			
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_X_A(void){		// 0x5c

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = A;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			X = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_A_IOk(void){	// 0x5d

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of second FIRST
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;
				data_bus0.PostIOReadPrecharge(k);
			}
			else {
				// second EXEC cycle

				if (data_bus0.PostIORead((k), &op_result)) {
					return 1;
				}

				A = op_result;

				// set flags
				SetZeroIfZero(op_result);

				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if ((CycleCount == 0) && (eCurrentState == EXEC)) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mov_A_IOXpk(void){	// 0x5e

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;
				data_bus0.PostIOReadPrecharge(X+k);
			}
			else {
				// second EXEC cycle

				if (data_bus0.PostIORead((X+k), &op_result)) {
					return 1;
				}

				A = op_result;

				// set flags
				SetZeroIfZero(op_result);

				EndOp();
			}
			break;
		}

	}
	else if (eCurrentTick == NEG_CPU) {
		if ((CycleCount == 0) && (eCurrentState == EXEC)) {
			// this will happen a half clock before the io read
			DBDRVBitesMyAss();
		}
	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mov_Mi_Mk(void){	// 0x5f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				i = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				k = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
			}
			else {
				// end of seventh FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostMemWrite((i), op_result)) {
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mov_IOk_A(void){	// 0x60

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostIOWrite(k, op_result)) {
				return 1;
			}

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;


}

bool m8::Op_Mov_IOXpk_A(void){	// 0x61

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else {
				// end of third FIRST
				op_result = A;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostIOWrite((X+k), op_result)) {
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Mov_IOk_i(void){	// 0x62

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else {
				// end of fifth FIRST cycle... go to EXEC
				op_result = i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostIOWrite((k), op_result)) {
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Mov_IOXpk_i(void){	// 0x63

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load third byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else {
				// end of sixth FIRST cycle... go to EXEC
				op_result = i;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (data_bus0.PostIOWrite((X+k), op_result)) {
				return 1;
			}
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Asl_A(void){		// 0x64

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			op_result = A << 1;
			op_result = op_result & 0xfe;	// make sure low bit is zero - viva paranoia
			
			// set flags
			SetCarryIfMSB(A);		// carry based on high bit of original A value
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Asl_Mk(void){		// 0x65

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				SetCarryIfMSB(op_result);	// carry based on msb of original read

				op_result = op_result << 1;
				op_result = op_result & 0xfe;	// make sure low bit is zero - viva paranoia

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Asl_MXpk(void){		// 0x66

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				SetCarryIfMSB(op_result);	// carry based on msb of original read

				op_result = op_result << 1;
				op_result = op_result & 0xfe;	// make sure low bit is zero - viva paranoia

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Asr_A(void){		// 0x67

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			op_result = A >> 1;
			if ((op_result & 0x40) == 0) {	// determine upper bit of result
				op_result = op_result & 0x7f;
			}
			else {
				op_result = op_result | 0x80;
			}
			
			// set flags
			SetCarryIfLSB(A);		// carry based on low bit of original A value
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Asr_Mk(void){		// 0x68

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				SetCarryIfLSB(op_result);	// carry based on msb of original read

				op_result = op_result >> 1;
				if ((op_result & 0x40) == 0) {	// determine upper bit of result
					op_result = op_result & 0x7f;
				}
				else {
					op_result = op_result | 0x80;
				}

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Asr_MXpk(void){		// 0x69

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				SetCarryIfLSB(op_result);	// carry based on msb of original read

				op_result = op_result >> 1;
				if ((op_result & 0x40) == 0) {	// determine upper bit of result
					op_result = op_result & 0x7f;
				}
				else {
					op_result = op_result | 0x80;
				}

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Rlc_A(void){		// 0x6a

	unsigned char tmp;

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			tmp = GetCarry();

			op_result = A << 1;
			op_result = op_result & 0xfe;	// make sure low bit is zero
			op_result = op_result | tmp;	// low bit is last carry
			
			// set flags
			SetCarryIfMSB(A);		// carry based on high bit of original A value
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Rlc_Mk(void){		// 0x6b

	unsigned char tmp;

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				tmp = GetCarry();

				SetCarryIfMSB(op_result);	// carry based on msb of original read

				op_result = op_result << 1;
				op_result = op_result & 0xfe;	// make sure low bit is zero
				op_result = op_result | tmp;	// low bit is carry

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Rlc_MXpk(void){		// 0x6c

	unsigned char tmp;

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				tmp = GetCarry();

				SetCarryIfMSB(op_result);	// carry based on msb of original read

				op_result = op_result << 1;
				op_result = op_result & 0xfe;	// make sure low bit is zero - viva paranoia
				op_result = op_result | tmp;	// low bit is carry

				// set flags
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Rrc_A(void){		// 0x6d

	unsigned char tmp;

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			tmp = A;

			op_result = A >> 1;
			if ((GetCarry()) == 0) {	// determine upper bit of result
				op_result = op_result & 0x7f;
			}
			else {
				op_result = op_result | 0x80;
			}
			
			// set flags
			SetCarryIfLSB(tmp);		// carry based on low bit of original A value
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Rrc_Mk(void){		// 0x6e

	unsigned char tmp;

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				tmp = op_result;

				op_result = op_result >> 1;
				if ((GetCarry()) == 0) {	// determine upper bit of result
					op_result = op_result & 0x7f;
				}
				else {
					op_result = op_result | 0x80;
				}

				// set flags
				SetCarryIfLSB(tmp);	// carry based on lsb of original read
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Rrc_MXpk(void){		// 0x6f

	unsigned char tmp;

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				tmp = op_result;

				op_result = op_result >> 1;
				if ((GetCarry()) == 0) {	// determine upper bit of result
					op_result = op_result & 0x7f;
				}
				else {
					op_result = op_result | 0x80;
				}

				// set flags
				SetCarryIfLSB(tmp);	// carry based on lsb of original read
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_And_F_k(void){		// 0x70

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;

			if (GetSup()) {
				// sup is on - preserve it
				op_result = FL & k;
				op_result = op_result | SUP_FLAG;
			}
			else {
				// sup is off - preserve it
				op_result = FL & k;
				op_result = op_result & ~SUP_FLAG;
			}

			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			FL = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Or_F_k(void){		// 0x71

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			if (GetSup()) {
				// sup is on - preserve it
				op_result = FL | k;
				op_result = op_result | SUP_FLAG;
			}
			else {
				// sup is off - preserve it
				op_result = FL | k;
				op_result = op_result & ~SUP_FLAG;
			}
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			FL = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Xor_F_k(void){		// 0x72

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			k = flash0.GetFlashByte(PC);
			if (BumpPC()) break;
			if (GetSup()) {
				// sup is on - preserve it
				op_result = FL ^ k;
				op_result = op_result | SUP_FLAG;
			}
			else {
				// sup is off - preserve it
				op_result = FL ^ k;
				op_result = op_result & ~SUP_FLAG;
			}
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			FL = op_result;	// store result

			EndOp();
			
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Cpl_A(void){		// 0x73

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			op_result = A ^ 0xff;

			// set flags
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Inc_A(void){		// 0x74

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = A + 1;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetCarryIfZero(op_result);
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Inc_X(void){		// 0x75

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = X + 1;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetCarryIfZero(op_result);
			SetZeroIfZero(op_result);

			X = op_result;

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Inc_Mk(void){		// 0x76

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = op_result + 1;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfZero(op_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Inc_MXpk(void){		// 0x77

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				op_result = op_result + 1;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfZero(op_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Dec_A(void){		// 0x78

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = A - 1;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetCarryIfNegOne(op_result);
			SetZeroIfZero(op_result);

			A = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Dec_X(void){		// 0x79

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			op_result = X - 1;
			CycleCount = 0;
			eCurrentState = EXEC;
			CheckIntStatus();

			break;
		case EXEC:

			// set flags
			SetCarryIfNegOne(op_result);
			SetZeroIfZero(op_result);

			X = op_result;	// store result

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Dec_Mk(void){		// 0x7a

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of third FIRST cycle... go to EXEC
				op_result = op_result - 1;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;
				// set flags
				SetCarryIfNegOne(op_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite(k, op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;
}

bool m8::Op_Dec_MXpk(void){		// 0x7b

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((X+k), &op_result)) {
					return 1;
				}
			}
			else {
				// end of fourth FIRST cycle... go to EXEC
				op_result = op_result - 1;
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			if (CycleCount == 0) {
				// first EXEC cycle
				CycleCount++;

				// set flags
				SetCarryIfNegOne(op_result);
				SetZeroIfZero(op_result);
			}
			else {
				// second EXEC cycle
				if (data_bus0.PostMemWrite((X+k), op_result)) {
					return 1;
				}
				EndOp();
			}
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Lcall_k_i(void){	// 0x7c

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), GetPCH())) {
					return 1;
				}
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
			}
			else if (CycleCount == 6) {
				// end of seventh FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 7) {
				// end of eigth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), GetPCL())) {
					return 1;
				}
			}
			else if (CycleCount == 8) {
				// end of ninth FIRST
				CycleCount++;
			}
			else if (CycleCount == 9) {
				// end of tenth FIRST
				CycleCount++;
				PC = ConcatPC(k,i);
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Ljmp_k_i(void){		// 0x7d

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				i = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else {
				// end of fourth FIRST

				PC = ConcatPC(k, i);
				
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Reti(void){			// 0x7e

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
            if (eCurrentState == FIRST) {
                // SUP_FLAG goes low in RETI at beginning of first FIRST.
                FL &= ~SUP_FLAG;
		        int iFL = FL;
		        dump_waves0.vPostVariableChange("M8", "FL[7:0]", iFL);	
                test_controller0_next.SetSup( false);
            }
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				CycleCount++;
				SP--;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((SP), &FL)) {
					return 1;
				}
				SP--;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((SP), &k)) {
					return 1;
				}
				SP--;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((SP), &i)) {
					return 1;
				}
			}
			else if (CycleCount == 6) {
				// end of seventh FIRST
				CycleCount++;

				PC = ConcatPC(i, k);

				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();
			}
			break;
		case EXEC:
			// tell flash to fix up anything it needs to fix up... the supervisory operation is over...
			flash0.FinishSupOp();
			// clear boot
			test_controller0_next.ClearBoot(); // so it sticks on clock
			test_controller0.ClearBoot();      // so the flash doesn't get the wrong fucking byte in boot mode
			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Ret(void){			// 0x7f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				CycleCount++;
				SP--;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((SP), &k)) {
					return 1;
				}
				SP--;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
				if (data_bus0.PostMemRead((SP), &i)) {
					return 1;
				}
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;

				PC = ConcatPC(i, k);

				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Jmp_k(void){		// 0x80-8f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
//				if (BumpPC()) break;
				BumpPC();
				CycleCount++;
			}
			else {
				// end of second FIRST

				PC = ComputeTwelveBitTarg(IR, k);
				
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Call_k(void){		// 0x90-9f

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), GetPCH())) {
					return 1;
				}
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
				SP++;
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
				if (data_bus0.PostMemWrite((SP - 1), GetPCL())) {
					return 1;
				}
			}
			else if (CycleCount == 6) {
				// end of seventh FIRST
				CycleCount++;
			}
			else if (CycleCount == 7) {
				// end of eighth FIRST
				CycleCount++;
				PC = ComputeTwelveBitTargNoSub(IR, k);
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Jz_k(void){			// 0xa0-af

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
//				if (BumpPC()) break;
				BumpPC();
				CycleCount++;
			}
			else {
				// end of second FIRST

				if (GetZero()) {
					PC = ComputeTwelveBitTarg(IR, k);
				}
				
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Jnz_k(void){		// 0xb0-bf

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
//				if (BumpPC()) break;
				BumpPC();
				CycleCount++;
			}
			else {
				// end of second FIRST

				if (GetZero() == 0) {
					PC = ComputeTwelveBitTarg(IR, k);
				}
				
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Jc_k(void){			// 0xc0-cf

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
//				if (BumpPC()) break;
				BumpPC();
				CycleCount++;
			}
			else {
				// end of second FIRST

				if (GetCarry()) {
					PC = ComputeTwelveBitTarg(IR, k);
				}
				
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Jnc_k(void){		// 0xd0-df

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
//				if (BumpPC()) break;
				BumpPC();
				CycleCount++;
			}
			else {
				// end of second FIRST

				if (GetCarry() == 0) {
					PC = ComputeTwelveBitTarg(IR, k);
				}
				
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Jacc_k(void){		// 0xe0-ef

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
//				if (BumpPC()) break;
				BumpPC();
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
			}
			else {
				// end of fourth FIRST

				PC = ComputeTwelveBitTarg(IR, k);
				PC = PC + A;

				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}

bool m8::Op_Index_k(void){		// 0xf0-ff

	unsigned int tmp;

	if (eCurrentTick == POS_CPU) {

		switch (eCurrentState) {
		case SOI:
			ProcessSOI();
			break;
		case FIRST:
			if (CycleCount == 0) {
				// end of first FIRST
				k = flash0.GetFlashByte(PC);	// load second byte of op
				if (BumpPC()) break;
				CycleCount++;
			}
			else if (CycleCount == 1) {
				// end of second FIRST
				CycleCount++;
			}
			else if (CycleCount == 2) {
				// end of third FIRST
				CycleCount++;
			}
			else if (CycleCount == 3) {
				// end of fourth FIRST
				CycleCount++;
			}
			else if (CycleCount == 4) {
				// end of fifth FIRST
				CycleCount++;
			}
			else if (CycleCount == 5) {
				// end of sixth FIRST
				CycleCount++;
			}
			else if (CycleCount == 6) {
				// end of seventh FIRST
				CycleCount++;
				tmp = ComputeTwelveBitTargNoSubIndex(IR, k, A);
				if (VerboseLevel) {
					emu_log0.printf( "index to %x from %x : %x + %x\n", tmp, IR, k, A);
				}
				op_result = flash0.GetFlashByte(tmp);
			}
			else if (CycleCount == 7) {
				// end of eigth FIRST
				CycleCount++;
			}
			else if (CycleCount == 8) {
				// end of ninth FIRST
				CycleCount++;
				A = op_result;
				// set flags
				SetZeroIfZero(op_result);
			}
			else if (CycleCount == 9) {
				// end of tenth FIRST
				CycleCount = 0;
				eCurrentState = EXEC;
				CheckIntStatus();

			}
			break;
		case EXEC:

			EndOp();
			break;
		}

	}
	else {
		// do nothing if it ain't a POS_CPU tick
	}

	return 0;

}
