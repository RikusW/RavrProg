// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"
#include "Rtk-base/RConfig.h"

#include <stdio.h>


//-----------------------------------------------------------------------------

int AVRISPmkI_Plist[] = { -1, 921600, 230400, 57600, 28800, 10000, 4000, 2000, 1300, 0 };

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

//---------------------------------------------------------

int STK500_Plist[] = { -1, 1843200, 460800, 115200, 57600, 28800, 10000, 4000, 2000, 1300, 0 };
int STK500_Elist[] = { -1, 3686400, 1843200, 1228800, 921600, 460800, 230400, 115200, 57600, 28800, 0 };

ProgSTK500::ProgSTK500()
{
	STK500Commands *cmds5;
	cmd = cmds5 = new STK500Commands();
	cmd->SetProg(this);

	interface = &isp;
	isp.next = &hvpp;
	hvpp.next = &hvsp;

	isp.SetCmd(this,cmds5);
	hvpp.SetCmd(this,cmds5);
	hvsp.SetCmd(this,cmds5);

	Clock = 7372800;
	isp.PClockList = STK500_Plist;
	isp.EClockList = STK500_Elist;


	pname = "STK500";
	pnm = "stk500";

	isp .iname = "STK500 ISP";
	hvpp.iname = "STK500 HVPP";
	hvsp.iname = "STK500 HVSP";
}

//---------------------------------------------------------

bool ProgIF_STK500::SetAddress(u32 a)
{
	address = a;
	return cmds5->SetAddress(a);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Programming clock ISP

#include <math.h>
#define B        12.0

int ProgIF_AVRISPmkI_ISP::SetPClock(int f)
{
	Hz2P(f);
	int r = P2Hz();
	printf("ProgIF_AVRISPmkI_ISP::SetPClock %i => %i (sck = %02x)\n",f,r,sck);
	return r;
}

//---------------------------------------------------------

int ProgIF_AVRISPmkI_ISP::ApplyPClock()
{
	int r;
	if(bDefaultP) {
		Hz2P(-1);
		r = P2Hz();
		printf("Default Programming clock %i (sck = %02x)\n",r,sck);
	}else{
		r = P2Hz();
		printf("Applying Programming clock %i (sck = %02x) ",r,sck);
		bool b = cmds5->SetParameter(PARAM_SCK_DURATION,sck);
		printf("%s\n",b ? "OK" : "FAILED");
	}
	return r;
}

//---------------------------------------------------------
//convert Hz to parameters

void ProgIF_AVRISPmkI_ISP::Hz2P(int u)
{
	bDefaultP = false;
	if(u == -1) {
//		printf("ProgIF_AVRISPmkI_ISP::Hz2P Using defaults\n");
		if(bConnected) {
			cmds5->GetParameter(PARAM_SCK_DURATION,sck);
		}else{
			sck = 2; //XXX ???
		}
		bDefaultP = true;
	}else{
		//HW SPI
		for(int i = 1; i < 5; i++) {
			if(u >= (int)PClockList[i]) {
				sck = i - 1;
				return;
//				goto end;
			}
		}

		//SW SPI
		double T = 1 / ((double)prog->Clock);
		sck = (u8)ceil(1/(2 * B * u * T) - 10/B);
		sck = sck > 254 ? 254 : sck;
	}
//end:
//	uint ss = sck;
//	printf("ProgIF_AVRISPmkI_ISP::Hz2P %i <- (sck = %u)\n",P2Hz(),ss);
}

//---------------------------------------------------------
//convert parameters to Hz

int ProgIF_AVRISPmkI_ISP::P2Hz()
{
	if(sck < 4) {
		return PClockList[sck + 1];
	}
//	int ss = sck;
	double s = sck;
	double T = 1 / ((double)prog->Clock);
	int r = (int)(1 / ((s - (10 / B)) * (2 * B * T)));
//	printf("ProgIF_AVRISPmkI_ISP::P2Hz %i -> (sck = %u)\n",r,ss);
	return r;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//ProgIF_AVRISPmkI_ISP

bool ProgIF_AVRISPmkI_ISP::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
		cmds5->SetParameter(PARAM_RESET_POLARITY,1);
		return true;
	}
	return false;
}

//---------------------------------------------------------

void ProgIF_AVRISPmkI_ISP::SetDevice(Device *d)
{
	RConfigNode *t,*n = d->config->GetNode(stkxml);

	fwblock = fpagesize; //ISP write using page sized blocks
	ewblock = epagesize; //??

	t = n->GetNode("IspChipErase");
	xml.ce.eraseDelay = (u8)t->GetValue("eraseDelay", 20);
	xml.ce.pollMethod = (u8)t->GetValue("pollMethod", 0);

	t = n->GetNode("IspProgramFlash");
	xml.pf.mode      = (u8)t->GetValue("mode",       0);
	xml.pf.blockSize = (u8)t->GetValue("blockSize",  0);
	xml.pf.delay     = (u8)t->GetValue("delay",      10);
	xml.pf.cmd1      = (u8)t->GetValue("cmd1",       0x40);
	xml.pf.cmd2      = (u8)t->GetValue("cmd2",       0x4C);
	xml.pf.cmd3      = (u8)t->GetValue("cmd3",       0x20);
	xml.pf.pollVal1  = (u8)t->GetValue("pollVal1",   0xFF);
	xml.pf.pollVal2  = (u8)t->GetValue("pollVal2",   0xFF);

	t = n->GetNode("IspProgramEeprom");
	xml.pe.mode      = (u8)t->GetValue("mode",       0);
	xml.pe.blockSize = (u8)t->GetValue("blockSize",  0);
	xml.pe.delay     = (u8)t->GetValue("delay",      20);
	xml.pe.cmd1      = (u8)t->GetValue("cmd1",       0xC0);
	xml.pe.cmd2      = (u8)t->GetValue("cmd2",       0x00);
	xml.pe.cmd3      = (u8)t->GetValue("cmd3",       0xA0);
	xml.pe.pollVal1  = (u8)t->GetValue("pollVal1",   0xFF);
	xml.pe.pollVal2  = (u8)t->GetValue("pollVal2",   0xFF);
}

//---------------------------------------------------------

bool ProgIF_AVRISPmkI_ISP::Enter()
{
	StartFrame(&msg);
	*msg++ = CMD_ENTER_PROGMODE_ISP; // used the values for ATmega16U2
	*msg++ = 200; // timeout	1 byte  XML: timeout  Command time-out (in ms) 
	*msg++ = 100; // stabDelay	1 byte  XML: stabDelay  Delay (in ms) used for pin stabilization 
	*msg++ =  25; // cmdexeDelay1 byte  XML: cmdexeDelay  Delay (in ms) in connection with the 
	*msg++ =  32; // synchLoops	1 byte  XML: synchLoops  Number of synchronization loops 
	*msg++ =   0; // byteDelay	1 byte  XML: byteDelay  Delay (in ms) between each byte in the
	*msg++ = 0x53; // pollValue	0x53 for AVR, 0x69 for AT89xx 
	*msg++ = 0x03; // pollIndex 0 = no polling, 3 = AVR, 4 = AT89xx 
	*msg++ = 0xAC; // cmd1 
	*msg++ = 0x53; // cmd2 
	*msg++ = 0x00; // cmd3 
	*msg++ = 0x00; // cmd4 
	EndFrame(&msg);

	return msg[1] == STATUS_CMD_OK;
}
//---------------------------------------------------------

bool ProgIF_AVRISPmkI_ISP::Leave()
{
	StartFrame(&msg);
	*msg++ = CMD_LEAVE_PROGMODE_ISP;
	*msg++ = 1;
	*msg++ = 1;
	EndFrame(&msg);

	return true; //XXX
}

//---------------------------------------------------------=====

u8 ProgIF_AVRISPmkI_ISP::ReadSig(u8 u)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_SIGNATURE_ISP;
	*msg++ = 4; //pollIndex
	*msg++ = 0x30;
	*msg++ = 0;
	*msg++ = u;
	*msg++ = 0;
	EndFrame(&msg);

	return msg[2];
}

//---------------------------------------------------------

u8 ProgIF_AVRISPmkI_ISP::ReadCal(u8 u)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_OSCCAL_ISP;
	*msg++ = 4; //pollIndex
	*msg++ = 0x38;
	*msg++ = 0;
	*msg++ = u;
	*msg++ = 0;
	EndFrame(&msg);

	return msg[2];
}

//---------------------------------------------------------=====
/*
#define RF_LFUSE_ISP 0x00 //lf 50 00 00 oo
#define RF_LOCK_ISP  0x80 //lk 58 00 00 oo
#define RF_EFUSE_ISP 0x08 //ef 50 08 00 oo
#define RF_HFUSE_ISP 0x88 //hf 58 08 00 oo*/

u8 ProgIF_AVRISPmkI_ISP::ReadFuse(u8 a, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return 0;
	}

	u8 c[4] = { 0x00, 0x88, 0x08, 0x80 };
	u8 u = c[a & 3];

	StartFrame(&msg);
	*msg++ = a == 3 ? CMD_READ_LOCK_ISP : CMD_READ_FUSE_ISP;
	*msg++ = 4; //pollIndex
	*msg++ = 0x50 | ((u>>4)&0x0F);
	*msg++ = u & 0x0F;
	*msg++ = 0;
	*msg++ = 0;
	EndFrame(&msg);

	printf("0x%02X %s\n", msg[2],(msg[1] == STATUS_CMD_OK && msg[3] == STATUS_CMD_OK) ? "OK" : "FAILED");
	return msg[2];
}

//---------------------------------------------------------=====
/*
#define WF_LFUSE_ISP 0xA0 //lf AC A0 00 ii
#define WF_LOCK_ISP  0xE0 //lk AC E0 00 ii
#define WF_EFUSE_ISP 0xA4 //ef AC A4 00 ii
#define WF_HFUSE_ISP 0xA8 //hf AC A8 00 ii*/

void ProgIF_AVRISPmkI_ISP::WriteFuse(u8 a, u8 v, u8 ov, const char *name)
{
/*	if(!((1<<n) & XXX)) { //not present on this avr
		return;
	}*/

	printf("%s",name);
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
	*msg++ = a == 3 ? CMD_PROGRAM_LOCK_ISP : CMD_PROGRAM_FUSE_ISP;
	*msg++ = 0xAC;
	*msg++ = u;
	*msg++ = 0;
	*msg++ = v;
	EndFrame(&msg);

	printf("0x%02X %s\n",v,(msg[1] == STATUS_CMD_OK && msg[2] == STATUS_CMD_OK) ? "OK" : "FAILED");
}

//-----------------------------------------------------------------------------

void ProgIF_AVRISPmkI_ISP::ChipErase()
{
	StartFrame(&msg);
	*msg++ = CMD_CHIP_ERASE_ISP;
	*msg++ = xml.ce.eraseDelay; //Delay (in ms) to ensure that the erase of the device is finished
	*msg++ = xml.ce.pollMethod; //Poll method, 0 = use delay 1= use RDY/BSY command
	*msg++ = 0xAC;
	*msg++ = 0x80;
	*msg++ = 0x00;
	*msg++ = 0x00;
	EndFrame(&msg);
}

//---------------------------------------------------------=====

bool ProgIF_AVRISPmkI_ISP::ReadFlashBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_FLASH_ISP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	*msg++ = 0x20;
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK || msg[2+size] != STATUS_CMD_OK) {
		return false;
	}
	msg+=2;
	for(u32 i = 0; i < size; i++) {
		*p++ = msg[i];
	}
	return true;
}

//---------------------------------------------------------=====

bool ProgIF_AVRISPmkI_ISP::WriteFlashBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_PROGRAM_FLASH_ISP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	*msg++ = xml.pf.mode | 0x80; //write page
	*msg++ = xml.pf.delay;
	*msg++ = xml.pf.cmd1;
	*msg++ = xml.pf.cmd2;
	*msg++ = xml.pf.cmd3;
	*msg++ = xml.pf.pollVal1;
	*msg++ = xml.pf.pollVal2;
	for(u32 i = 0; i < size; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg);

	Delay(50);

	if(msg[1] != STATUS_CMD_OK) {
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------

bool ProgIF_AVRISPmkI_ISP::ReadEepromBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_EEPROM_ISP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	*msg++ = 0xA0;
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK) {
		return false;
	}
	msg+=2;
	for(u32 i = 0; i < size; i++) {
		*p++ = msg[i];
	}
	return true;
}

//---------------------------------------------------------

bool ProgIF_AVRISPmkI_ISP::WriteEepromBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_PROGRAM_EEPROM_ISP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	*msg++ = xml.pe.mode | 0x80; //write page
	*msg++ = xml.pe.delay;
	*msg++ = xml.pe.cmd1;
	*msg++ = xml.pe.cmd2;
	*msg++ = xml.pe.cmd3;
	*msg++ = xml.pe.pollVal1;
	*msg++ = xml.pe.pollVal2;
	for(u32 i = 0; i < size; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg);

	Delay(50);

	if(msg[1] != STATUS_CMD_OK) {
		return false;
	}
	return true;
}

//ProgIF_AVRISPmkI_ISP
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//ProgIF_STK500_ISP

int ProgIF_STK500_ISP::SetEClock(int f)
{
	Hz2E(f);
	int r = E2Hz();
	printf("ProgIF_STK500_ISP::SetEClock %i => %i (oscp = %02x oscc = %02x)\n",f,r,oscp,oscc);
	return r;
}

//---------------------------------------------------------

int ProgIF_STK500_ISP::ApplyEClock()
{
	int r;
	if(bDefaultE) {
		Hz2E(-1);
		r = E2Hz();
		printf("Default External clock %i (oscp = %02x oscc = %02x)\n",r,oscp,oscc);
	}else{
		r = E2Hz();
		printf("Applying External clock %i (oscp = %02x oscc = %02x) ",r,oscp,oscc);
		bool b = cmds5->SetParameter(PARAM_OSC_PSCALE,oscp) && cmds5->SetParameter(PARAM_OSC_CMATCH,oscc);
		printf("%s\n",b ? "OK" : "FAILED");
	}
	return r;
}

//---------------------------------------------------------
//convert Hz to parameters

void ProgIF_STK500_ISP::Hz2E(int i)
{
	bDefaultE = false;
	if(i == -1) {
		if(bConnected) {
			cmds5->GetParameter(PARAM_OSC_PSCALE,oscp);
			cmds5->GetParameter(PARAM_OSC_CMATCH,oscc);
		}else{
			oscp = oscc = 0; //XXX ???
		}
		bDefaultE = true;
	}else
	if(i == 0) {
		oscp = oscc = 0;
	}else{
		oscp = 1; //no support for other prescalers...
		int j =  (prog->Clock / 2) / i;
		j = j > 255 ? 255 : j;
		j = j == 0 ? 0 : j-1;
		oscc = (u8)j;
	}
}

//---------------------------------------------------------
//convert parameters to Hz

int ProgIF_STK500_ISP::E2Hz()
{
	u32 clk = prog->Clock;
	switch(oscp & 7) {
	case 0: clk  = 0;   break;
	case 1: clk /= 2;   break;
	case 2: clk /= 16;  break;
	case 3: clk /= 64;  break;
	case 4: clk /= 128; break;
	case 5: clk /= 256; break;
	case 6: clk /= 512; break;
	case 7: clk /= 2048;break;
	}

	//uint p = oscp, c = oscc;
	if(clk) {
		int r = clk / (oscc + 1);
		//printf("Prog_STK500_ISP::E2Hz (oscp = %u - oscc = %u) -> %u\n",p,c,(uint)r);
		return (int)r;
	}else{
		//printf("Prog_STK500_ISP::E2Hz (oscp = %u - oscc = %u) -> Off\n",p,c);
		return 0;
	}
}

//ProgIF_STK500_ISP
//-----------------------------------------------------------------------------
//ProgIF_STK500_HVPP

void ProgIF_STK500_HVPP::SetDevice(Device *d)
{
	u8 buf[50];
	RConfigNode *t,*n = d->config->GetNode(stkxml);
	n->GetBin("PPControlStack",buf,32);
	SetControlStack(buf);

	t = n->GetNode("PpEnterProgMode");
	xml.e.stabDelay      = (u8)t->GetValue("stabDelay",   100);
	xml.e.progModeDelay  = (u8)t->GetValue("progModeDelay", 0);
	xml.e.latchCycles    = (u8)t->GetValue("latchCycles",   5);
	xml.e.toggleVtg      = (u8)t->GetValue("toggleVtg",     1);
	xml.e.powerOffDelay  = (u8)t->GetValue("powerOffDelay",15);
	xml.e.resetDelayMs   = (u8)t->GetValue("resetDelayMs",  2);
	xml.e.resetDelayUs   = (u8)t->GetValue("resetDelayUs",  0);

	t = n->GetNode("PpChipErase");
	xml.ce.pulseWidth   = (u8)t->GetValue("pulseWidth",    1);
	xml.ce.pollTimeout  = (u8)t->GetValue("pollTimeout",  50); //??

	t = n->GetNode("PpProgramFlash");
	xml.pf.pollTimeout  = (u8)t->GetValue("pollTimeout",  10);
	xml.pf.mode         = (u8)t->GetValue("mode",          0);
	xml.pf.blockSize    = (u8)t->GetValue("blockSize",   256);

	t = n->GetNode("PpProgramEeprom");
	xml.pe.pollTimeout  = (u8)t->GetValue("pollTimeout",  10);
	xml.pe.mode         = (u8)t->GetValue("mode",          0);
	xml.pe.blockSize    = (u8)t->GetValue("blockSize",   256);

	t = n->GetNode("PpProgramFuse");
	xml.pu.pulseWidth   = (u8)t->GetValue("pulseWidth",    0);
	xml.pu.pollTimeout  = (u8)t->GetValue("pollTimeout",  10);

	t = n->GetNode("PpProgramLock");
	xml.pl.pulseWidth   = (u8)t->GetValue("pulseWidth",    0);
	xml.pl.pollTimeout  = (u8)t->GetValue("pollTimeout",  10);
}

//---------------------------------------------------------

void ProgIF_STK500_HVPP::SetControlStack(u8 *s)
{
	StartFrame(&msg);
	*msg++ = CMD_SET_CONTROL_STACK;
	for(int i = 0; i < 32; i++) {
		*msg++ = *s++;
	}
	EndFrame(&msg);
}

//---------------------------------------------------------

bool ProgIF_STK500_HVPP::Enter()
{
	StartFrame(&msg);
	*msg++ = CMD_ENTER_PROGMODE_HVPP; // used the values for ATmega16U2
	*msg++ = xml.e.stabDelay;
	*msg++ = xml.e.progModeDelay;
	*msg++ = xml.e.latchCycles;
	*msg++ = xml.e.toggleVtg;
	*msg++ = xml.e.powerOffDelay;
	*msg++ = xml.e.resetDelayMs;
	*msg++ = xml.e.resetDelayUs;
	EndFrame(&msg);

	return msg[1] == STATUS_CMD_OK;
}

//---------------------------------------------------------

bool ProgIF_STK500_HVPP::Leave()
{
	StartFrame(&msg);
	*msg++ = CMD_LEAVE_PROGMODE_HVPP;
	*msg++ = 15; //stabDelay
	*msg++ = 15; //resetDelay
	EndFrame(&msg);

	return true; //XXX
}

//---------------------------------------------------------

u8 ProgIF_STK500_HVPP::ReadSig(u8 a)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_SIGNATURE_HVPP;
	*msg++ = a;
	EndFrame(&msg);

	return msg[2];
}

//---------------------------------------------------------

u8 ProgIF_STK500_HVPP::ReadCal(u8 a)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_OSCCAL_HVPP;
	*msg++ = a;
	EndFrame(&msg);

	return msg[2];
}

//---------------------------------------------------------

u8 ProgIF_STK500_HVPP::ReadFuse(u8 a, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return 0;
	}

	StartFrame(&msg);
	if(a < 3) {
		*msg++ = CMD_READ_FUSE_HVPP;
		*msg++ = a;
	}else{
		*msg++ = CMD_READ_LOCK_HVPP;
		*msg++ = 0;
	}
	EndFrame(&msg);

	printf("0x%02X %s\n", msg[2], msg[1] == STATUS_CMD_OK ? "OK" : "FAILED");
	return msg[2];
}

//---------------------------------------------------------

void ProgIF_STK500_HVPP::WriteFuse(u8 a, u8 v, u8 ov, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return;
	}
	if(v == ov) {
		printf("SKIPPING\n");
		return;
	}
	Delay(10);

	StartFrame(&msg);
	if(a < 3) {
		*msg++ = CMD_PROGRAM_FUSE_HVPP;
		*msg++ = a;
		*msg++ = v;
		*msg++ = xml.pu.pulseWidth;
		*msg++ = xml.pu.pollTimeout;
	}else{
		*msg++ = CMD_PROGRAM_LOCK_HVPP;
		*msg++ = 0;
		*msg++ = v;
		*msg++ = xml.pl.pulseWidth;
		*msg++ = xml.pl.pollTimeout;
	}
	EndFrame(&msg);

	printf("0x%02X %s\n",v,msg[1] == STATUS_CMD_OK ? "OK" : "FAILED");
}

//-----------------------------------------------------------------------------

void ProgIF_STK500_HVPP::ChipErase()
{
	StartFrame(&msg);
	*msg++ = CMD_CHIP_ERASE_HVPP;
	*msg++ = xml.ce.pulseWidth;
	*msg++ = xml.ce.pollTimeout;
	EndFrame(&msg);
}

//--------------------------------------------------------=====

bool ProgIF_STK500_HVPP::ReadFlashBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_FLASH_HVPP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK || msg[2+size] != STATUS_CMD_OK) {
		return false;
	}
	msg+=2;
	for(u32 i = 0; i < size; i++) {
		*p++ = msg[i];
	}
	return true;
}

//---------------------------------------------------------

bool ProgIF_STK500_HVPP::WriteFlashBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_PROGRAM_FLASH_HVPP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	*msg++ = xml.pf.mode | 0x80; //write page XXX 0x40 for last page.... XXX
	*msg++ = xml.pf.pollTimeout;
	for(u32 i = 0; i < size; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK) {
		return false;
	}
	return true;
}

//---------------------------------------------------------

bool ProgIF_STK500_HVPP::ReadEepromBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_EEPROM_HVPP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK || msg[2+size] != STATUS_CMD_OK) {
		return false;
	}
	msg+=2;
	for(u32 i = 0; i < size; i++) {
		*p++ = msg[i];
	}
	return true;
}

//---------------------------------------------------------

bool ProgIF_STK500_HVPP::WriteEepromBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_PROGRAM_EEPROM_HVPP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	*msg++ = xml.pe.mode | 0x80; //write page
	*msg++ = xml.pe.pollTimeout;
	for(u32 i = 0; i < size; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK) {
		return false;
	}
	return true;
}

//ProgIF_STK500_HVPP
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//ProgIF_STK500_HVSP

void ProgIF_STK500_HVSP::SetDevice(Device *d)
{
	u8 buf[50];
	RConfigNode *t,*n = d->config->GetNode(stkxml);
	n->GetBin("HvspControlStack",buf,32);
	SetControlStack(buf);
	
	t = n->GetNode("HvspEnterProgMode");
	xml.e.stabDelay     = (u8)t->GetValue("stabDelay", 100);
	xml.e.cmdexeDelay   = (u8)t->GetValue("cmdexeDelay", 0);
	xml.e.synchCycles   = (u8)t->GetValue("synchCycles", 6);
	xml.e.latchCycles   = (u8)t->GetValue("latchCycles", 1);
	xml.e.toggleVtg     = (u8)t->GetValue("toggleVtg",   1);
	xml.e.powoffDelay   = (u8)t->GetValue("powoffDelay",25);
	xml.e.resetDelay1   = (u8)t->GetValue("resetDelay1", 1);
	xml.e.resetDelay2   = (u8)t->GetValue("resetDelay2", 0);

	t = n->GetNode("HvspChipErase");
	xml.ce.pollTimeout  = (u8)t->GetValue("pollTimeout",40);
	xml.ce.eraseTime    = (u8)t->GetValue("eraseTime",   0);

	t = n->GetNode("HvspProgramFlash");
	xml.pf.mode         = (u8)t->GetValue("mode",        0);
	xml.pf.blockSize    = (u8)t->GetValue("blockSize", 256);
	xml.pf.pollTimeout  = (u8)t->GetValue("pollTimeout", 5);

	t = n->GetNode("HvspProgramEeprom");
	xml.pe.mode         = (u8)t->GetValue("mode",        0);
	xml.pe.blockSize    = (u8)t->GetValue("blockSize", 256);
	xml.pe.pollTimeout  = (u8)t->GetValue("pollTimeout", 5);
}

//---------------------------------------------------------

void ProgIF_STK500_HVSP::SetControlStack(u8 *s)
{
	StartFrame(&msg);
	*msg++ = CMD_SET_CONTROL_STACK;
	for(int i = 0; i < 32; i++) {
		*msg++ = *s++;
	}
	EndFrame(&msg);
}

//---------------------------------------------------------

bool ProgIF_STK500_HVSP::Enter() //for t24
{
	StartFrame(&msg);
	*msg++ = CMD_ENTER_PROGMODE_HVSP; // used the values for ATmega16U2
	*msg++ = 100; //stabDelay
	*msg++ = 0;   //cmdexeDelay
	*msg++ = 6;   //synchCycles
	*msg++ = 1;   //latchCycles
	*msg++ = 1;   //toggleVtg
	*msg++ = 25;  //powoffDelay
	*msg++ = 1;   //resetDelayMs
	*msg++ = 0;   //resetDelayUs
	EndFrame(&msg);

	return msg[1] == STATUS_CMD_OK;
}

//---------------------------------------------------------

bool ProgIF_STK500_HVSP::Leave()
{
	StartFrame(&msg);
	*msg++ = CMD_LEAVE_PROGMODE_HVSP;
	*msg++ = 100; //stabDelay
	*msg++ = 25; //resetDelay
	EndFrame(&msg);

	return true; //XXX
}

//---------------------------------------------------------

u8 ProgIF_STK500_HVSP::ReadSig(u8 u)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_SIGNATURE_HVSP;
	*msg++ = u;
	EndFrame(&msg);

	return msg[2];
}

//---------------------------------------------------------

u8 ProgIF_STK500_HVSP::ReadCal(u8 u)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_OSCCAL_HVSP;
	*msg++ = u;
	EndFrame(&msg);

	return msg[2];
}

//---------------------------------------------------------

u8 ProgIF_STK500_HVSP::ReadFuse(u8 a, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return 0;
	}

	StartFrame(&msg);
	if(a < 3) {
		*msg++ = CMD_READ_FUSE_HVSP;
		*msg++ = a;
	}else{
		*msg++ = CMD_READ_LOCK_HVSP;
		*msg++ = 0;
	}
	EndFrame(&msg);

	printf("0x%02X %s\n", msg[2], msg[1] == STATUS_CMD_OK ? "OK" : "FAILED");
	return msg[2];
}

//---------------------------------------------------------

void ProgIF_STK500_HVSP::WriteFuse(u8 a, u8 v, u8 ov, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return;
	}
	if(v == ov) {
		printf("SKIPPING\n");
		return;
	}
	Delay(10);

	StartFrame(&msg);
	if(a < 3) {
		*msg++ = CMD_PROGRAM_FUSE_HVSP;
		*msg++ = a;
		*msg++ = v;
		*msg++ = 25; //pollTimeout
	}else{
		*msg++ = CMD_PROGRAM_LOCK_HVSP;
		*msg++ = 0;
		*msg++ = v;
		*msg++ = 25; //pollTimeout
	}
	EndFrame(&msg);

	printf("0x%02X %s\n",v,msg[1] == STATUS_CMD_OK ? "OK" : "FAILED");
}

//-----------------------------------------------------------------------------

void ProgIF_STK500_HVSP::ChipErase()
{
	StartFrame(&msg);
	*msg++ = CMD_CHIP_ERASE_HVSP;
	*msg++ = 1;   //XXX XML
	*msg++ = 100; //XXX XML
	EndFrame(&msg);
}

//---------------------------------------------------------=====

bool ProgIF_STK500_HVSP::ReadFlashBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_FLASH_HVSP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK || msg[2+size] != STATUS_CMD_OK) {
		return false;
	}
	msg+=2;
	for(u32 i = 0; i < size; i++) {
		*p++ = msg[i];
	}
	return true;
}

//---------------------------------------------------------

bool ProgIF_STK500_HVSP::WriteFlashBlock(u8 *p, u32 size, u32)
{
	return false;

	StartFrame(&msg);
	*msg++ = CMD_PROGRAM_FLASH_HVSP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	*msg++ = 0 | 0x80; //write page; XXX 0x40 for last page.... XXX //mode  // get these from the config file
	*msg++ = 0; //pollTimeout
	for(u32 i = 0; i < size; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK) {
		return false;
	}
	return true;
}

//---------------------------------------------------------

bool ProgIF_STK500_HVSP::ReadEepromBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = CMD_READ_EEPROM_HVSP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK || msg[2+size] != STATUS_CMD_OK) {
		return false;
	}
	msg+=2;
	for(u32 i = 0; i < size; i++) {
		*p++ = msg[i];
	}
	return true;
}

//---------------------------------------------------------

bool ProgIF_STK500_HVSP::WriteEepromBlock(u8 *p, u32 size, u32)
{
	return false;

	StartFrame(&msg);
	*msg++ = CMD_PROGRAM_EEPROM_HVSP;
	*msg++ = (u8)((size >> 8) & 0xFF);
	*msg++ = (u8)(size & 0xFF);
	*msg++ = 0 | 0x80; //write page; //mode  // get these from the config file
	*msg++ = 0; //pollTimeout
	for(u32 i = 0; i < size; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg);

	if(msg[1] != STATUS_CMD_OK) {
		return false;
	}
	return true;
}

//ProgIF_STK500_HVSP
//-----------------------------------------------------------------------------#####
//-----------------------------------------------------------------------------#####
//STK500Commands

bool STK500Commands::Connect(const char *c)
{
	if(Commands::Connect(c)) {
		port->SetBaud(115200);
		return true;
	}
	return false;
}

//---------------------------------------------------------

bool STK500Commands::SetParameter(u8 p,u8 v)
{
	StartFrame(&msg);
	*msg++ = CMD_SET_PARAMETER;
	*msg++ = p;
	*msg++ = v;
	EndFrame(&msg);

	return msg[1] == STATUS_CMD_OK;
}

//---------------------------------------------------------

bool STK500Commands::GetParameter(u8 p,u8 &v)
{
	StartFrame(&msg);
	*msg++ = CMD_GET_PARAMETER;
	*msg++ = p;
	EndFrame(&msg);

	if(msg[1] == STATUS_CMD_OK) {
		v = msg[2];
		return true;
	}else{
		v = 0;
		return false;
	}
}

//---------------------------------------------------------

bool STK500Commands::SetAddress(u32 a)
{
	StartFrame(&msg);
	*msg++ = CMD_LOAD_ADDRESS;
	*msg++ = (u8)(a >> 24); //MSB
	*msg++ = (u8)(a >> 16);
	*msg++ = (u8)(a >>  8);
	*msg++ = (u8)(a >>  0); //LSB
	EndFrame(&msg);

	return msg[1] == STATUS_CMD_OK;
}

//---------------------------------------------------------

bool STK500Commands::Osccal()
{
	StartFrame(&msg);
	*msg++ = CMD_OSCCAL;
	EndFrame(&msg);

	return msg[1] == STATUS_CMD_OK;
}

//---------------------------------------------------------
/*
#include <string.h>

bool STK500Commands::FwUpgrade()
{
	StartFrame(&msg);
	*msg++ = CMD_FIRMWARE_UPGRADE;
	strcpy(msg,"fwupgrade");
	msg += 10;
	EndFrame(&msg);

	return msg[1] == STATUS_CMD_OK;
}*/

//---------------------------------------------------------

#include <string.h>

bool STK500Commands::SignXn(const char *sig)
{
	printf("STK Signon: ");
	StartFrame(&msg);
	*msg++ = CMD_SIGN_ON;
	EndFrame(&msg);

	u8 l = strlen(sig);

	if(msg[1] == STATUS_CMD_OK && msg[2] == l) {
		if(!strncmp((char*)&msg[3],sig,l)) {
			char buf[15];
			strncpy(buf,(char*)&msg[3],l);
			buf[l] = 0;
			puts(buf);
			return true;
		}
	}

	puts("FAILED");
	return false;
}

//---------------------------------------------------------

bool STK500Commands::SignOff()
{
	return true;
}

//-----------------------------------------------------------------------------

void STK500Commands::StartFrame(u8** msg)
{
	*msg = &buf[5];
}

//---------------------------------------------------------

u8 seq = 0; //XXX

void PrintHex(const char *m, u8 *buf, int sz); //dbg

u32 STK500Commands::EndFrame(u8** msg)
{
//------------------------Transmit------------------------

	u8 xorsum;
	u16 sz;
	int i;

	sz = *msg - &buf[5];

	buf[0] = MESSAGE_START;
	buf[1] = seq++;
	buf[2] = (sz >> 8) & 0xFF;
	buf[3] = sz & 0xFF;
	buf[4] = TOKEN;
	sz += 5;

	xorsum = 0;
	for(i=0; i<sz; i++) {
		xorsum ^= buf[i];
	}
	buf[sz++] = xorsum;

//	PrintHex("EndFrame Write",buf,sz);

	port->Write(buf,sz);
	sz = 0;

//------------------------Receive-------------------------

	do {
		port->Read(buf,1);
	}while(buf[0] != MESSAGE_START); // timeout ???

	port->Read(buf+1,4);
	if(buf[4] != TOKEN) {
		return 0; //fail
	}

	//TODO seq check ?

	u32 rxsize = ((buf[2] << 8) | buf[3]) + 1; //+1 for checksum
	u32 left = rxsize;
	u32 idx = 5;
	u32 r;

	//port->Read(&buf[5],rxsize);

	//new 201311 code
	while(left) {
		r = port->Read(&buf[idx],left);
		left -= r;
		idx += r;
	}
	
//	PrintHex("EndFrame Read",buf,rxsize+5);

	//TODO do checksum check here ?

	*msg = &buf[5];
	return rxsize;
}


//STK500Commands
//-----------------------------------------------------------------------------











//-----------------------------------------------------------------------------
/*//Programming clock ISP

u32 ProgSTK500::SetPclk(u32 u)
{
	if(u == -1) {
		printf("Setting Programming clock to: Default => ");
		GetParameter(PARAM_SCK_DURATION,sck);
	}else{
		printf("Setting Programming clock to: %u => ",u);
		if(u >= 1843200) {
			sck = 0;
			return 1843200;
		}else
		if(u >= 460800) {
			sck = 1;
			return 460800;
		}else
		if(u >= 115200) {
			sck = 2;
			return 115200;
		}else
		if(u >= 57600) {
			sck = 3;
			return 57600;
		}else{
			
			sck = ceil(1/(2 * B * u * T_STK500) - 10/B);
			sck = sck > 254 ? 254 : sck;
		}
	}

	u32 ss = sck;
	double s = sck;
	double C = 1 / Clock;
	u32 r = 1 / ((s - (10 / B)) * (2 * B * C));
	printf("%u (sck = %u)\n",r,ss);
	return r;
}

void ProgSTK500::ApplyPclk()
{
	u32 ss = sck;
	double s = sck;
	double C = 1 / Clock;
	u32 r = 1 / ((s - (10 / B)) * (2 * B * C));
	printf("Applying Programming clock: %u (sck = %u)",r,ss);
	if(SetParameter(PARAM_SCK_DURATION,sck)) {
		puts("OK");
	}else{
		puts("FAILED");
	}
}*/

//---------------------------------------------------------
/*//Clock generator

u32 STK500_Elist[] = { -1, 3686400, 1843200, 1228800, 921600, 460800, 230400, 115200, 57600, 28800, 0 };

u32* ProgSTK500::GetEClockList()
{
	return STK500_Elist;
}

u32 ProgSTK500::SetEclk(u32 u)
{
	if(u == -1) {
		//use defaults from programmer
		printf("Setting Clock generator to: Default => ");
		GetParameter(PARAM_OSC_PSCALE,oscp);
		GetParameter(PARAM_OSC_CMATCH,oscc);
	}else
	if(u == 0) {
		printf("Turning off Clock generator\n");
		return 0;
	}else{
		//convert Hz to parameters
		printf("Setting Clock generator to: %u => ",u);
	}
	
	//convert parameters to Hz
	u32 clk = 7372800;
	switch(oscp & 7) {
	case 0: clk  = 0;   break;
	case 1: clk /= 2;   break;
	case 2: clk /= 16;  break;
	case 3: clk /= 64;  break;
	case 4: clk /= 128; break;
	case 5: clk /= 256; break;
	case 6: clk /= 512; break;
	case 7: clk /= 2048;break;
	}

	if(clk) {
		u32 p = oscp, c = oscc;
		u32 r = clk / (oscc + 1);
		printf("%u (oscp = %u - oscc = %u)\n",r,p,c);
		return r;
	}else{
		printf("OFF\n");
		return 0;
	}
}

void ProgSTK500::ApplyEclk()
{
	SetParameter(PARAM_OSC_PSCALE,oscp);
	SetParameter(PARAM_OSC_CMATCH,oscc);
}
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//---------------------------------------------------------

