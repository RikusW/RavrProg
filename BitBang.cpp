// vim:ts=4 sts=0 sw=4

//BitBang protocol handler
//Copyright Rikus Wessels 2013
//rikusw gmail

#include "RavrProg.h"
#include "Rtk-base/RConfig.h"
#include "BitBang.h"

#include <stdio.h>


//-----------------------------------------------------------------------------
/*
extern int AVRISPmkI_Plist[];// = { -1, 921600, 230400, 57600, 28800, 10000, 4000, 2000, 1300, 0 };

ProgAVRISPmkI::ProgAVRISPmkI()
{
	STK500Commands *cmds5;
	cmd = cmds5 = new AVRISP1Commands();
	cmd->SetProg(this);

	interface = &isp;
	isp.SetCmd(this,cmds5);

	Clock = 3686400;
	isp.PClockList = AVRISPmkI_Plist;

	pname = "AVRISP mkI";
	pnm = "avrisp1";

	isp.iname = "AVRISP mkI ISP";
}
*/
//---------------------------------------------------------
//---------------------------------------------------------
/*
bool ProgIF_STK500::SetAddress(u32 a)
{
	address = a;
	return cmds5->SetAddress(a);
}
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Programming clock ISP

int ProgIF_BitBang_ISP::SetPClock(int f)
{
	return 0;
}

//---------------------------------------------------------

int ProgIF_BitBang_ISP::ApplyPClock()
{
	return 0;
}

//---------------------------------------------------------
//convert Hz to parameters

void ProgIF_BitBang_ISP::Hz2P(int u)
{
}

//---------------------------------------------------------
//convert parameters to Hz

int ProgIF_BitBang_ISP::P2Hz()
{
	return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//ProgIF_BitBang_ISP

bool ProgIF_BitBang_ISP::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
//		cmds5->SetParameter(PARAM_RESET_POLARITY,1);
		return true;
	}
	return false;
}

//---------------------------------------------------------

/*
PulseSCK:
	//SPI off
	out		SPCR,rnull

	//pulse sck
	ldi		r16,3 //ms
	rcall	Delay
	sbi		SPIPORT,SCKBIT
	rcall	Delay
	cbi		SPIPORT,SCKBIT
	rcall	Delay

	//SPI on
	rcall	SetupSpiClk
	ret

SetResetActive:
	lds		r16,extrst //should be (1=AVR rst0) or (0=AT89 rst1)
	tst		r16
	breq	SRAz
	cbi		RSTPORT,RSTBIT //AVR
	rjmp	SRAr
SRAz:
	sbi		RSTPORT,RSTBIT //AT89
SRAr:
	sbi		RSTDDR,RSTBIT
	ret


*/

bool ProgIF_BitBang_ISP::Enter()
{
	const u8 timeout=200,stabdelay=100,cmdexedelay=25,synchloops=32;

	//SetResetActive();
	//DelayMs(stabdelay);
	//SetupSpiPort();
	//DelayMs(cmdexedelay);
	//Disable_dW();
	//SetupSpiClk();
	//DelayMs(stabdelay);
	//SetResetActive();

	for(int i = 0; i < 32; i++) {
		StartFrame(&msg);
		*msg++ = 0xAC; // cmd1 
		*msg++ = 0x53; // cmd2 
		*msg++ = 0x00; // cmd3 
		*msg++ = 0x00; // cmd4 
		//ISPGet(&msg); ISPxx
		if(msg[2] == 0x53) {
			return true;
		}
		//PulseSCK();
	}
	return false;
}

//---------------------------------------------------------

bool ProgIF_BitBang_ISP::Leave()
{
	DelayMs(1);
	//SpiOff();
	//ResetLo_HiZ();
	DelayMs(1);
	return true; //XXX
}

//---------------------------------------------------------=====

u8 ProgIF_BitBang_ISP::ReadSig(u8 u)
{
	StartFrame(&msg);
	*msg++ = 0x30;
	*msg++ = 0;
	*msg++ = u;
	*msg++ = 0;
//	ISPGet(&msg);

	return msg[3];
}

//---------------------------------------------------------

u8 ProgIF_BitBang_ISP::ReadCal(u8 u)
{
	StartFrame(&msg);
	*msg++ = 0x38;
	*msg++ = 0;
	*msg++ = u;
	*msg++ = 0;
//	ISPGet(&msg);

	return msg[3];
}

//---------------------------------------------------------=====
/*
#define RF_LFUSE_ISP 0x00 //lf 50 00 00 oo
#define RF_LOCK_ISP  0x80 //lk 58 00 00 oo
#define RF_EFUSE_ISP 0x08 //ef 50 08 00 oo
#define RF_HFUSE_ISP 0x88 //hf 58 08 00 oo*/

u8 ProgIF_BitBang_ISP::ReadFuse(u8 a, const char *name)
{
	printf(name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return 0;
	}

	u8 c[4] = { 0x00, 0x88, 0x08, 0x80 };
	u8 u = c[a & 3];

	StartFrame(&msg);
	*msg++ = 0x50 | ((u>>4)&0x0F);
	*msg++ = u & 0x0F;
	*msg++ = 0;
	*msg++ = 0;
//	ISPGet(&msg);

	return msg[3];
}

//---------------------------------------------------------=====
/*
#define WF_LFUSE_ISP 0xA0 //lf AC A0 00 ii
#define WF_LOCK_ISP  0xE0 //lk AC E0 00 ii
#define WF_EFUSE_ISP 0xA4 //ef AC A4 00 ii
#define WF_HFUSE_ISP 0xA8 //hf AC A8 00 ii*/

void ProgIF_BitBang_ISP::WriteFuse(u8 a, u8 v, u8 ov, const char *name)
{
/*	if(!((1<<n) & XXX)) { //not present on this avr
		return;
	}*/

	printf(name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return;
	}
	if(v == ov) {
		printf("SKIPPING\n");
		return;
	}
	Delay(10);

	u8 c[4] = { 0xA0, 0xA8, 0xA4, 0xE0 };
	u8 u = c[a & 3];

	StartFrame(&msg);
	*msg++ = 0xAC;
	*msg++ = u;
	*msg++ = 0;
	*msg++ = v;
//	ISPSend(&msg); ISPxx

}

//-----------------------------------------------------------------------------

void ProgIF_BitBang_ISP::ChipErase()
{
	StartFrame(&msg);
	*msg++ = 0xAC;
	*msg++ = 0x80;
	*msg++ = 0x00;
	*msg++ = 0x00;
//	ISPSend(&msg); ISPxx

	if(xml.ce.pollMethod == 0) {
		DelayMs(xml.ce.eraseDelay);
		return;
	}
	//RdyBsy();
}

//---------------------------------------------------------

void LoadEA(u32 a)
{
/*	if(!(a & 0x80000000)) {
		return;
	}
//	if(already updated) {
//		return;
//	}

	StartFrame(&msg);
	*msg++ = 0x4D;
	*msg++ = 0x00;
	*msg++ = (u8)((a >> 16) & 0xFF);
	*msg++ = 0x00;
//	ISPSend(&msg); ISPxx*/
}

bool ProgIF_BitBang_ISP::ReadFlashBlock(u8 *p, u32 size, u32 addr)
{
	while(size) {
		StartFrame(&msg);
		*msg++ = 0x20; // read command XXX use xml value ??
		*msg++ = (u8)((address >> 8) & 0xFF);
		*msg++ = (u8)(address & 0xFF);
		*msg++ = 0x00;
	//	ISPGet(&msg); poll 4 ISPxx*/
		*p++ = msg[3];

		StartFrame(&msg);
		*msg++ = 0x28; // read command
		*msg++ = (u8)((address >> 8) & 0xFF);
		*msg++ = (u8)(address & 0xFF);
		*msg++ = 0x00;
	//	ISPGet(&msg); poll 4 ISPxx*/
		*p++ = msg[3];

		//ISPIncAddress(); //XXX LoadEA updating...
		address++;
		size -= 2;
	}
	//save address to global here
	return true;
}

//---------------------------------------------------------

bool ProgIF_BitBang_ISP::WriteFlashBlock(u8 *p, u32 size, u32 addr)
{
	LoadEA(addr); //XXX
	while(size) {
		WriteByte(&xml.pf,*p++); xml.pf.cmd1 |= 0x80;
		WriteByte(&xml.pf,*p++); xml.pf.cmd1 &= 0x7F;
		//ISPIncAddress();
		size -= 2;
	}
	WritePage(&xml.pf);
	//save address to global here
	Delay(50); //???

	return true;
}

//---------------------------------------------------------

bool ProgIF_BitBang_ISP::ReadEepromBlock(u8 *p, u32 size, u32)
{
	while(size) {
		StartFrame(&msg);
		*msg++ = 0xA0; //XXX xml ??
		*msg++ = (u8)((address >> 8) & 0xFF);
		*msg++ = (u8)(address & 0xFF);
		*msg++ = 0;
	//	ISPGet(&msg);
		*p++ = msg[3];

		size--;
		address++;
	}
	return true;
}

//---------------------------------------------------------

bool ProgIF_BitBang_ISP::WriteEepromBlock(u8 *p, u32 size, u32)
{
	while(size) {
		WriteByte(&xml.pe,*p++);
		size--;
		address++;
	}
	WritePage(&xml.pe);

	Delay(50);

	return true;
}

//---------------------------------------------------------

void ProgIF_BitBang_ISP::WriteByte(IspProgram *ip, u8 data)
{
	StartFrame(&msg);
	*msg++ = ip->cmd1;
	*msg++ = (u8)((address >> 8) & 0xFF);
	*msg++ = (u8)(address & 0xFF);
	*msg++ = data;
//	ISPSend(&msg);

	if(ip->mode & 2) { // --timed delay--
		DelayMs(xml.pf.delay);
		return;
	}
	if(ip->mode & 4) { // --value polling--
		PollValue(ip);
		return;
	}
	if(ip->mode & 8) { // --rdy/bsy-- eep only
		RdyBsy(ip);
	}
}

//---------------------------------------------------------

void ProgIF_BitBang_ISP::WritePage(IspProgram *ip)
{
	StartFrame(&msg);
	*msg++ = xml.pf.cmd2;
	*msg++ = (u8)((address >> 8) & 0xFF);
	*msg++ = (u8)(address & 0xFF);
	*msg++ = 0;
//	ISPSend(&msg);

	if(ip->mode & 0x10) { // --timed delay--
		DelayMs(ip->delay);
		return;
	}
	if(ip->mode & 0x20) { // --value polling--
		address--; PollValue(ip); address++; //XXX ??? get data in here....
		return;
	}
	if(ip->mode & 0x40) { // --rdy/bsy-- eep only
		RdyBsy(ip);
	}
}

//---------------------------------------------------------

bool ProgIF_BitBang_ISP::PollValue(IspProgram *ip) //delay/cmd/data
{
	u8 cnt = ip->delay;

	u8 dt = 12;//XXX!!!
	//if(dt == 0xFF) Delay(cnt); XXX

	while(cnt) {
		StartFrame(&msg);
		*msg++ = ip->cmd3; //XXX xml !! pf/pe !! ??
		*msg++ = (u8)((address >> 8) & 0xFF);
		*msg++ = (u8)(address & 0xFF);
		*msg++ = 0;
	//	ISPGet(&msg);

		if(msg[4] == dt) {
			return true;
		}
		cnt--;
		Delay(1);
	}
	return false;
}

//---------------------------------------------------------

bool ProgIF_BitBang_ISP::RdyBsy(IspProgram *ip)
{
	u8 cnt = ip->delay;
	while(cnt) {
		StartFrame(&msg);
		*msg++ = 0xF0; //XXX xml ??
		*msg++ = 0;
		*msg++ = 0;
		*msg++ = 0;
	//	ISPGet(&msg);

		if((msg[4] & 1) == 0) {
			return true;
		}
		cnt--;
		Delay(1);
	}
	return false;
}

//ProgIF_BitBang_ISP
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

































//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
