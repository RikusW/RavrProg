// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"

#include <stdio.h>

//-----------------------------------------------------------------------------

ProgAVRISPmkII::ProgAVRISPmkII()
{
	usb.pid = USB_DEVICE_AVRISPMKII;

	cmd = cmdi2 = new AVRISP2Commands();
	cmd->SetProg(this);

	interface = &isp;
	isp.next = &tpi;
	tpi.next = &pdi;

	isp.SetCmd(this,cmdi2);
	tpi.prog = this; tpi.cmd = tpi.cmdi2 = cmdi2;
	pdi.prog = this;

	pname = "AVRISP mkII";
	pnm = "avrisp2";
	isp.iname = "AVRISP mkII ISP";
	tpi.iname = "AVRISPmkII TPI xxx";
	pdi.iname = "AVRISPmkII PDI xxx";
}

//---------------------------------------------------------

ProgSTK600::ProgSTK600()
{
	cmd = cmdi2 = cmds6 = new STK600Commands();
	cmd->SetProg(this);

	cmdj2s6 = new Jtag2S6Commands();
	cmdj2s6->SetProg(this);
	cmdj2s6->cmds6 = cmds6;

	usb.pid = USB_DEVICE_STK600;
	usb.ep_read = 0x83;

	interface = &isp;
	isp.next = &hvpp;
	hvpp.next = &hvsp;
	hvsp.next = &mj;
	mj.next = &tpi;
	tpi.next = &pdi;
	pdi.next = &xj;
	xj.next = &aj;

	isp.SetCmd(this,cmds6);
	hvpp.SetCmd(this,cmds6);
	hvsp.SetCmd(this,cmds6);

	mj.prog = this; mj.cmd = mj.cmdj2 = cmdj2s6;

/*	mj.prog = this;
	tpi.prog = this;
	pdi.prog = this;
	xj.prog = this;
	aj.prog = this;*/

	pname = "STK600";
	pnm = "stk600";
	isp.iname = "STK600 ISP";
	hvpp.iname= "STK600 HVPP";
	hvsp.iname= "STK600 HVSP";
	mj .iname = "STK600 MEGA JTAG xxx";
	tpi.iname = "STK600 TPI xxx";
	pdi.iname = "STK600 PDI xxx";
	xj .iname = "STK600 XMEGA JTAG xxx";
	aj .iname = "STK600 AVR32 JTAG xxx";
}

ProgSTK600::~ProgSTK600()
{
	delete cmdj2s6;
}

//-----------------------------------------------------------------------------
//ISP

int AVRISPmkII_Plist[] = { -1,
8000000, 4000000, 2000000, 1000000, 500000, 250000, 125000, //HW
64000, 32654, 16097, 8239, 4209, 2152, 1000, 511, 100, 0 }; //SW

ProgIF_AVRISPmkII_ISP::ProgIF_AVRISPmkII_ISP()
{
	sck = 6;
	bDefaultP = false;
	PClockList = AVRISPmkII_Plist;
}

//---------------------------------------------------------

int ProgIF_AVRISPmkII_ISP::SetPClock(int f)
{
	Hz2P(f);
	int r = P2Hz();
	printf("ProgIF_AVRISPmkII_ISP::SetPClock %i => %i (sck = %02x)\n",f,r,sck);
	return r;
}

//---------------------------------------------------------

int ProgIF_AVRISPmkII_ISP::ApplyPClock()
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
//frequencies for AVRISP mkII ISP programming
int avrispmkIIfreqs[] = {
	8000000, 4000000, 2000000, 1000000, 500000, 250000, 125000,
	96386, 89888, 84211, 79208, 74767, 70797, 67227, 64000,
	61069, 58395, 55945, 51613, 49690, 47905, 46243, 43244,
	41885, 39409, 38278, 36200, 34335, 32654, 31129, 29740,
	28470, 27304, 25724, 24768, 23461, 22285, 21221, 20254,
	19371, 18562, 17583, 16914, 16097, 15356, 14520, 13914,
	13224, 12599, 12031, 11511, 10944, 10431, 9963, 9468,
	9081, 8612, 8239, 7851, 7498, 7137, 6809, 6478, 6178,
	5879, 5607, 5359, 5093, 4870, 4633, 4418, 4209, 4019,
	3823, 3645, 3474, 3310, 3161, 3011, 2869, 2734, 2611,
	2484, 2369, 2257, 2152, 2052, 1956, 1866, 1779, 1695,
	1615, 1539, 1468, 1398, 1333, 1271, 1212, 1155, 1101,
	1049, 1000, 953, 909, 866, 826, 787, 750, 715, 682,
	650, 619, 590, 563, 536, 511, 487, 465, 443, 422,
	402, 384, 366, 349, 332, 317, 302, 288, 274, 261,
	249, 238, 226, 216, 206, 196, 187, 178, 170, 162,
	154, 147, 140, 134, 128, 122, 116, 111, 105, 100,
	96, 91, 87, 83, 79, 75, 72, 69,	65, 62, 59, 57, 54, 52
};

//---------------------------------------------------------

void ProgIF_AVRISPmkII_ISP::Hz2P(int f)
{
	bDefaultP = false;
	if(f == -1) {
		if(bConnected) {
			cmds5->GetParameter(PARAM_SCK_DURATION,sck);
		}else{
			sck = 6; //125kHz
		}
		bDefaultP = true;
	}else{
		sck = 6; //Default to 125000Hz
		for(int i = 0; i < (int)(sizeof(avrispmkIIfreqs)/4); i++) {
			if(avrispmkIIfreqs[i] <= f) {
				sck = i;
				break;
		    }
		}
	}
}

//---------------------------------------------------------

int ProgIF_AVRISPmkII_ISP::P2Hz()
{
	if(sck >= (u8)(sizeof(avrispmkIIfreqs)/4)) {
		return avrispmkIIfreqs[(sizeof(avrispmkIIfreqs)/4) - 1];
	}
	return avrispmkIIfreqs[sck];
}

//-----------------------------------------------------------------------------

int STK600_Plist[] = { -1,
8000000, 4000000, 2000000, 1000000, 500000, 250000, 125000, //HW
64000, 32000, 16000, 8000, 4000, 2000, 0 }; //SW

int STK600_Elist[] = { -1,
20000000, 16000000, 12000000, 8000000, 4000000, 2000000, 1000000, 500000, 250000, 125000,
64000, 32000, 16000, 8000, 4000, 2000, 0 };

ProgIF_STK600_ISP::ProgIF_STK600_ISP()
{
	stkxml = "STK600";
	PClockList = STK600_Plist;
	EClockList = STK600_Elist;
}

//---------------------------------------------------------

int ProgIF_STK600_ISP::SetPClock(int f)
{
	Hz2P(f);
	int r = P2Hz();
	printf("ProgIF_STK600_ISP::SetPClock %i => %i (sck = %02x)\n",f,r,sck);
	return r;
}

//---------------------------------------------------------

int ProgIF_STK600_ISP::ApplyPClock()
{
	int r;
	if(bDefaultP) {
		Hz2P(-1);
		r = P2Hz();
		printf("Default Programming clock %i (sck = %02x)\n",r,sck);
	}else{
		r = P2Hz();
		printf("Applying Programming clock %i (sck = %04x) ",r,sck);
//		bool b = SetParameter2(PARAM2_SCK_DURATION,sck);
//		printf("%s\n",b ? "OK" : "FAILED");
	}
	return r;
}

//---------------------------------------------------------

#include <math.h>
void ProgIF_STK600_ISP::Hz2P(int f)
{
	bDefaultP = false;
	if(f == -1) {
		if(bConnected) {
		//	GetParameter2(PARAM2_SCK_DURATION,sck);
		}else{
			sck = 15; //XXX ???
		}
		bDefaultP = true;
	}else{
		if(f > 8000000) {
			f = 8000000;
		}
		sck = (u16)ceil((8e6 / f) - 1);
		if(sck >= 4096) {
			sck = 4095; //1953 Hz
		}
	}
}

//---------------------------------------------------------

int ProgIF_STK600_ISP::P2Hz()
{
	return (8000000 / (sck + 1));
}

//-----------------------------------------------------------------------------

int ProgIF_STK600_ISP::SetEClock(int u)
{
	Hz2E(u);
	int r = E2Hz();
	u16 oct,dac;
	oct = (eclk >> 12) & 0xF;
	dac = (eclk >> 2) & 0x3FF;
	printf("ProgIF_STK600_ISP::SetEClock %i => %i (oct = %u, dac = %u)\n",(int)u,r,oct,dac);
	return r;
}

//---------------------------------------------------------

int ProgIF_STK600_ISP::ApplyEClock()
{
	int r;
	if(bDefaultE) {
		Hz2E(-1);
		r = E2Hz();
		printf("Default External clock %i (eclk = %04x)\n",r,eclk);
	}else{
		r = E2Hz();
		printf("Applying External clock %i (eclk = %04x)\n",r,eclk);
//		bool b = prog->SetParameter2(PARAM2_CLOCK_CONF,eclk);
//		printf("%s\n",b ? "OK" : "FAILED");
	}
	return r;
}

//---------------------------------------------------------

void ProgIF_STK600_ISP::Hz2E(int i)
{
	bDefaultE = false;
	if(i == -1) {
		if(bConnected) {
		//	GetParameter2(PARAM2_CLOCK_CONF,eclk);
		}else{
			eclk = 0xCEF8; //8MHz
		}
		bDefaultE = true;
	}else
	if(i == 0) {
		eclk = 0; //XXX off ???
		return;
	}else{
		u16 oct,dac;
		oct = (u16)(3.322 * log10(((double)i)/1039.0));
		dac = (u16)(2048 - ((2078.0 * pow(2, 10 + oct)) / (double)i));
		eclk = ((oct << 12) & 0xF000) | ((dac << 2) & 0xFFC);
	}
}

//---------------------------------------------------------

int ProgIF_STK600_ISP::E2Hz()
{
	if(!eclk) {
		return 0; //XXX ???
	}
	u16 oct,dac;
	oct = (eclk >> 12) & 0xF;
	dac = (eclk >> 2) & 0x3FF;
	return (int)(pow(2,oct) * (2078.0 / (2.0 - (((double)dac) / 1024.0))));
}

//ISP
//-----------------------------------------------------------------------------
//TPI

bool ProgIF_AVRISPmkII_TPI::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
		SetParameter(PARAM_RESET_POLARITY,1);
		SetParameter(PARAM_DISCHARGEDELAY,44);
		return cmdi2->XpSetMode(XPRG_PROTOCOL_TPI);
	}
	return false;
}

//---------------------------------------------------------

bool ProgIF_AVRISPmkII_TPI::Enter()
{
	if(!cmdi2->XpEnter()) {
		return false;
	}

	cmdi2->XpSetParameter1(XPRG_PARAM_NVMCMD_REG,0x33); //t10
	cmdi2->XpSetParameter1(XPRG_PARAM_NVMCSR_REG,0x32); //t10
	
	return true;
}

//---------------------------------------------------------

bool ProgIF_AVRISPmkII_TPI::Leave()
{
	return cmdi2->XpLeave();
}

//---------------------------------------------------------

u8 ProgIF_AVRISPmkII_TPI::ReadSig(u8 u)
{
	u8 r;
	cmdi2->XpRead(&r,XPRG_MEM_TYPE_APPL,1,0x3FC0 + u);
	return r;
}

//---------------------------------------------------------

u8 ProgIF_AVRISPmkII_TPI::ReadCal(u8 u)
{
	u8 r;
	cmdi2->XpRead(&r,XPRG_MEM_TYPE_FACTORY_CALIBRATION,1,0x3F80 + u);
	return r;
}

//---------------------------------------------------------

//XP XR mt aa aa aa aa cc cc
//50 05 01 00 00 3F C0 00 01 -> 50 05 00 1E
//50 05 01 00 00 3F C1 00 01 -> 50 05 00 90
//50 05 01 00 00 3F C2 00 01 -> 50 05 00 03
/*
u32 ProgIF_AVRISPmkII_TPI::ReadSignature()
{
	u32 r;
	u8 buf[5];	
	cmdi2->XpRead(buf,XPRG_MEM_TYPE_APPL,3,0x3FC0); //for tiny10....
	r  = buf[0]; r <<= 8;
	r |= buf[1]; r <<= 8;
	r |= buf[2];
	return r;
}
*/
//---------------------------------------------------------

void ProgIF_AVRISPmkII_TPI::SetDevice(Device *)
{
	frblock = fwblock = fpagesize;
}

//---------------------------------------------------------

u8 ProgIF_AVRISPmkII_TPI::ReadFuse(u8 a, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return 0;
	}

	u8 r;
	switch(a) {
	case 0: cmdi2->XpRead(&r,XPRG_MEM_TYPE_FUSE,1,0x3F40); break; //for tiny10....
	case 3: cmdi2->XpRead(&r,XPRG_MEM_TYPE_LOCKBITS,1,0x3F00); break; //for tiny10....
	//move 3 to 1 sometime ???
	default: return 0xFF;
	}
	printf("0x%02X %s\n",r,  "OK" );// : "FAILED");
	return r;
}

//---------------------------------------------------------

void ProgIF_AVRISPmkII_TPI::WriteFuse(u8 a, u8 v, u8 ov, const char *name)
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

	switch(a) {
	case 0: cmdi2->XpErase(XPRG_ERASE_FUSE, 0x3F41);
	        cmdi2->XpWrite(&v,XPRG_MEM_TYPE_FUSE,1,0x3F40); break; //for tiny10....
	case 3: cmdi2->XpWrite(&v,XPRG_MEM_TYPE_LOCKBITS,1,0x3F00); break; //for tiny10....
	//move 3 to 1 sometime ???
	default: puts("fuse XXX"); return;
	}

	printf("0x%02X %s\n",v, "OK" );// : "FAILED");
}

//---------------------------------------------------------

void ProgIF_AVRISPmkII_TPI::ChipErase()
{
	cmdi2->XpErase(XPRG_ERASE_CHIP, 0x4001); //?? 0 or 0x4000 ??
}

//---------------------------------------------------------

bool ProgIF_AVRISPmkII_TPI::ReadFlashBlock(u8 *p, u32 size, u32 addr)
{
	return cmdi2->XpRead(p,XPRG_MEM_TYPE_APPL, (u16)size, 0x4000 + addr);
}

//---------------------------------------------------------

bool ProgIF_AVRISPmkII_TPI::WriteFlashBlock(u8 *p, u32 size, u32 addr)
{
	return cmdi2->XpWrite(p,XPRG_MEM_TYPE_APPL, (u16)size, 0x4000 + addr);
}

//TPI
//-----------------------------------------------------------------------------
//PDI

bool ProgIF_AVRISPmkII_PDI::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
		SetParameter(PARAM_RESET_POLARITY,1);
		SetParameter(PARAM_DISCHARGEDELAY,44);
		return cmdi2->XpSetMode(XPRG_PROTOCOL_PDI);
	}
	return false;
}

//PDI
//-----------------------------------------------------------------------------
//XJ

bool ProgIF_STK600_XMEGA_JTAG::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
		return cmdi2->XpSetMode(XPRG_PROTOCOL_JTAG);
	}
	return false;
}

//XJ
//-----------------------------------------------------------------------------

bool AVRISP2Commands::XpSetMode(u8 u)
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG_SETMODE;
	*msg++ = u;
	EndFrame(&msg);

	if(msg[1] != XPRG_ERR_OK) {
		printf("XpSetMode failed\n");
		return false;
	}
	return true;
}

//-----------------------------------------------

bool AVRISP2Commands::XpEnter()
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_ENTER_PROGMODE;
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpEnter failed\n");
		return false;
	}
	return true;
}

//-----------------------------------------------

bool AVRISP2Commands::XpLeave()
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_LEAVE_PROGMODE;
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpLeave failed\n");
		return false;
	}
	return true;
}

//-----------------------------------------------

void AVRISP2Commands::XpSetParameter1(u8 p, u8 v)
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_SET_PARAM;
	*msg++ = p;
	*msg++ = v;
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpLeave failed\n");
	}
}

//-----------------------------------------------

void AVRISP2Commands::XpSetParameter2(u8 p, u16 v)
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_SET_PARAM;
	*msg++ = p;
	*msg++ = (u8)(v >> 8);
	*msg++ = (u8)v;
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpLeave failed\n");
	}
}

//-----------------------------------------------

void AVRISP2Commands::XpSetParameter4(u8 p, u32 v)
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_SET_PARAM;
	*msg++ = p;
	*msg++ = (u8)(v >> 24);
	*msg++ = (u8)(v >> 16);
	*msg++ = (u8)(v >> 8);
	*msg++ = (u8)(v);
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpLeave failed\n");
	}
}

//-----------------------------------------------

void AVRISP2Commands::XpErase(u8 m, u32 a)
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_ERASE;
	*msg++ = m;
	*msg++ = (u8)(a >> 24);
	*msg++ = (u8)(a >> 16);
	*msg++ = (u8)(a >> 8);
	*msg++ = (u8)(a);
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpErase failed\n");
	}
}

//-----------------------------------------------

bool AVRISP2Commands::XpWrite(u8 *p, u8 t, u16 c, u32 a, u8 pm)
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_WRITE_MEM;
	*msg++ = t;
	*msg++ = pm;
	*msg++ = (u8)(a >> 24);
	*msg++ = (u8)(a >> 16);
	*msg++ = (u8)(a >> 8);
	*msg++ = (u8)(a);
	*msg++ = (u8)(c >> 8);
	*msg++ = (u8)(c);
	for(u32 i = 0; i < c; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpWrite failed\n");
		return false;
	}
	return true;

}

//-----------------------------------------------

bool AVRISP2Commands::XpRead(u8 *p, u8 t, u16 c, u32 a)
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_READ_MEM;
	*msg++ = t;
	*msg++ = (u8)(a >> 24);
	*msg++ = (u8)(a >> 16);
	*msg++ = (u8)(a >> 8);
	*msg++ = (u8)(a);
	*msg++ = (u8)(c >> 8);
	*msg++ = (u8)(c);
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpRead failed\n");
		return false;
	}
	msg += 3;
	for(u32 i = 0; i < c; i++) { //data
		*p++ = *msg++;
	}
	return true;
}

//-----------------------------------------------

u32 AVRISP2Commands::XpCrc(u8 t)
{
	StartFrame(&msg);
	*msg++ = CMD_XPROG;
	*msg++ = XPRG_CMD_CRC;
	*msg++ = t;
	EndFrame(&msg);

	if(msg[2] != XPRG_ERR_OK) {
		printf("XpCrc failed\n");
		return 0;
	}

	u32 r;
	r  = msg[3] << 16;
	r |= msg[4] << 8;
	r |= msg[5];
	return r;
}

//---------------------------------------------------------

void AVRISP2Commands::StartFrame(u8** msg)
{
	*msg = buf;
}

u32 AVRISP2Commands::EndFrame(u8** msg)
{
	u32 sz = *msg - buf;

	port->Write(buf,sz);

	sz = port->Read(buf,1100); //max size

	*msg = buf;
	return sz;
}

//-----------------------------------------------------------------------------

bool Jtag2S6Commands::SignOn() { return cmds6->SignOn(); };
bool Jtag2S6Commands::SignOff() { return cmds6->SignOff(); };

//---------------------------------------------------------

void Jtag2S6Commands::StartFrame(u8** m)
{
	cmds6->StartFrame(m);
	*(*m)++ = CMD_JTAG_AVR; //jtag wrapper command
}

//---------------------------------------------------------

u32 Jtag2S6Commands::EndFrame(u8** m)
{
	u32 r = cmds6->EndFrame(m);
	if(*(*m)++ != CMD_JTAG_AVR) {
		printf("CMD_JTAG_AVR expected !!!\n");
	}
	return r;
}

//-----------------------------------------------------------------------------
//22118400, 18432000, 14745600, 11059200, 7372800, 3686400, 1843200,
