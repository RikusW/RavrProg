// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"

#include <stdio.h>

//-----------------------------------------------------------------------------####

ProgJTAGICEmkIIu::ProgJTAGICEmkIIu()
{
	usb.pid = USB_DEVICE_JTAGICEMKII;

	cmd = cmdj2 = new Jtag2Commands();
	cmdj2->SetProg(this);
	
	cmds5j2 = new STK500J2Commands();
	cmds5j2->SetProg(this);
	cmds5j2->cmdj2 = cmdj2;

	interface = &isp;
	isp.next = &mj;
	mj.next = &pdi;
	pdi.next = &xj;
	xj.next = &aj;

	isp.prog = this; isp.cmd = isp.cmds5 = cmds5j2; isp.cmdj2 = cmdj2;
	mj .prog = this; mj .cmd = mj .cmdj2 = cmdj2;
	pdi.prog = this; pdi.cmd = pdi.cmdj2 = cmdj2;
	xj .prog = this; xj .cmd = xj .cmdj2 = cmdj2;
	aj .prog = this; aj .cmd = aj .cmdj2 = cmdj2;

	pname = "JTAGICE mkII USB";
	pnm = "jtag2";
	isp.iname = "JTAGICE mkII ISP";
	mj .iname = "JTAGICE mkII MEGA JTAG";
	pdi.iname = "JTAGICE mkII PDI xxx";
	xj .iname = "JTAGICE mkII XMEGA JTAG xxx";
	aj .iname = "JTAGICE mkII AVR32 JTAG xxx";
}

ProgJTAGICEmkIIu::~ProgJTAGICEmkIIu()
{
	delete cmds5j2;
}

//---------------------------------------------------------

ProgJTAGICEmkIIs::ProgJTAGICEmkIIs()
{
	port = &serial;
	pname = "JTAGICE mkII Serial";
	pnm = "jtag2s";
	cmdj2->SetProg(this);
	cmds5j2->SetProg(this);

}

//-----------------------------------------------------------------------------####

bool ProgIF_JTAGICEmkII_ISP::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
		if(!cmdj2->SetEmulatorMode(EMULATOR_MODE_SPI)) {
			puts("cmdj2->SetEmulatorMode(EMULATOR_MODE_SPI) FAILED");
	
			//temporarily disable debugwire
			if(!cmdj2->SetEmulatorMode(EMULATOR_MODE_DEBUGWIRE)) {
				puts("cmdj2->SetEmulatorMode(EMULATOR_MODE_DEBUGWIRE) FAILED");
				puts("FAILED to disable dW");
				goto fail;
			}
			cmdj2->Reset(4); //saw this in the avrdude source, and protocol dump
			puts("Disabled dW");
		}
		//Atmel does this, but it seems to work without it... 
		if(!cmdj2->SetEmulatorMode(EMULATOR_MODE_SPI)) {
			puts("cmdj2->SetEmulatorMode(EMULATOR_MODE_SPI) FAILED again");
			goto fail;
		}
		SetParameter(PARAM_RESET_POLARITY,1);
		return true;
	}
fail:
	Disconnect();
	return false;
}

//-----------------------------------------------------------------------------####
//-----------------------------------------------------------------------------####

u8 rmb[300]; //temp XXX
int MJtagPlist[] = { -1, 2000000, 1000000, 500000, 250000, 125000, 50000, 0 };

ProgIF_JTAGICEmkII_MEGA_JTAG::ProgIF_JTAGICEmkII_MEGA_JTAG()
{
	inm = "mj";
	PClockList = MJtagPlist;
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkII_MEGA_JTAG::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
		if(!cmdj2->SetEmulatorMode(EMULATOR_MODE_JTAG)) {
			puts("cmdj2->SetEmulatorMode(EMULATOR_MODE_JTAG) FAILED");
		}
		return true;
	}
	return false;
}

//---------------------------------------------------------

void ProgIF_JTAGICEmkII_MEGA_JTAG::SetDevice(Device *d)
{
	cmdj2->SetDescriptor(d);
}

//---------------------------------------------------------

int ProgIF_JTAGICEmkII_MEGA_JTAG::SetPClock(int f)
{
	Hz2P(f);
	int r = P2Hz();
	printf("ProgIF_JTAGICEmkII_MEGA_JTAG::SetPClock %i => %i (jclk = %02x)\n",f,r,jclk);
	return r;
}

//---------------------------------------------------------

int ProgIF_JTAGICEmkII_MEGA_JTAG::ApplyPClock()
{
	int r;
	if(bDefaultP) {
		Hz2P(-1);
		r = P2Hz();
		printf("Default Programming clock %i (jclk = %02x)\n",r,jclk);
	}else{
		r = P2Hz();
		printf("Applying Programming clock %i (jclk = %02x) ",r,jclk);
		bool b = cmdj2->SetParameter(PAR_OCD_JTAG_CLK,jclk);
		printf("%s\n",b ? "OK" : "FAILED");
	}
	return r;
}

//---------------------------------------------------------

#include <math.h>

void ProgIF_JTAGICEmkII_MEGA_JTAG::Hz2P(int f)
{
	bDefaultP = false;
	if(f == -1) {
		if(bConnected) {
			cmdj2->GetParameter(PAR_OCD_JTAG_CLK,jclk);
		}else{
			jclk = 10; //500kHz
		}
		bDefaultP = true;
	}else{
		if(f >= 6400000) {
			jclk = 0;
		}else
		if(f >= 5800000) {
			jclk = 1;
		}else
		if(f <= 20915) {
			jclk = 0xFF;
		}else{
			jclk = (u8)(ceil((5333333.333333 / f) - 0.91666666667));
		}
	}
}

//---------------------------------------------------------

int ProgIF_JTAGICEmkII_MEGA_JTAG::P2Hz()
{
	int r;
	switch(jclk) {
	case 0: r = 6400000; break;
	case 1: r = 5800000; break;
	default: r= (int)(5333333.333333 / jclk); break;
	}
	return r;
}

//---------------------------------------------------------

u8 ProgIF_JTAGICEmkII_MEGA_JTAG::ReadSig(u8 a)
{
	cmdj2->ReadMem(rmb,MEMP_SIGNATURE,1,a);
	return *rmb;
}

u32 ProgIF_JTAGICEmkII_MEGA_JTAG::ReadSignature()
{
	u32 r;
	r  = ReadSig(0); r <<= 8;
	r |= ReadSig(1); r <<= 8;
	r |= ReadSig(2);
	return r;
}

//---------------------------------------------------------

u8 ProgIF_JTAGICEmkII_MEGA_JTAG::ReadCal(u8 a)
{
	cmdj2->ReadMem(rmb,MEMP_OSCCAL,1,a);
	return *rmb;
}

//---------------------------------------------------------

u32 ProgIF_JTAGICEmkII_MEGA_JTAG::ReadJtagID()
{
	u32 r = 0;
	cmdj2->GetParameter4(PAR_JTAGID,r);
	return r;
}

//---------------------------------------------------------

u8 ProgIF_JTAGICEmkII_MEGA_JTAG::ReadFuses(u8 b[32])
{
	b[0] = ReadFuse(0);
	b[1] = ReadFuse(1);
	b[2] = ReadFuse(2);
	b[3] = ReadLock();
	return 4;
}

u8 ProgIF_JTAGICEmkII_MEGA_JTAG::ReadFuse(u8 a)
{
	cmdj2->ReadMem(rmb,MEMP_FUSES,1,a);
	return *rmb;
}

u8 ProgIF_JTAGICEmkII_MEGA_JTAG::ReadLock()
{
	cmdj2->ReadMem(rmb,MEMP_LOCK,1,0);
	return *rmb;
}

//---------------------------------------------------------

void ProgIF_JTAGICEmkII_MEGA_JTAG::WriteFuses(u8 b[32])
{
	if(b[0] != ReadFuse(0)) {
		WriteFuse(0,b[3]);
	}
	if(b[1] != ReadFuse(1)) {
		WriteFuse(1,b[3]);
	}
	if(b[2] != ReadFuse(2)) {
		WriteFuse(2,b[3]);
	}
	if(b[3] != ReadLock()) {
		WriteLock(b[3]);
	}
}

void ProgIF_JTAGICEmkII_MEGA_JTAG::WriteFuse(u8 a, u8 v)
{
	*rmb = v;
	cmdj2->WriteMem(rmb,MEMP_FUSES,1,a);
}

void ProgIF_JTAGICEmkII_MEGA_JTAG::WriteLock(u8 v)
{
	*rmb = v;
	cmdj2->WriteMem(rmb,MEMP_LOCK,1,0);
}

//-----------------------------------------------------------------------------

void ProgIF_JTAGICEmkII_MEGA_JTAG::ChipErase()
{
	cmdj2->ChipErase();
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkII_MEGA_JTAG::ReadFlashBlock(u8 *p, u32 size, u32 addr)
{
	return cmdj2->ReadMem(p,MEMP_FLASH,size,addr);
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkII_MEGA_JTAG::WriteFlashBlock(u8 *p, u32 size, u32 addr)
{
	return cmdj2->WriteMem(p,MEMP_FLASH,size,addr);
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkII_MEGA_JTAG::ReadEepromBlock(u8 *p, u32 size, u32 addr)
{
	return cmdj2->ReadMem(p,MEMP_EEPROM,size,addr);
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkII_MEGA_JTAG::WriteEepromBlock(u8 *p, u32 size, u32 addr)
{
	return cmdj2->WriteMem(p,MEMP_EEPROM,size,addr);
}

//-----------------------------------------------------------------------------####
//-----------------------------------------------------------------------------####
// Jtag2Commands

bool Jtag2Commands::Connect(const char *c)
{
	if(Commands::Connect(c)) {
		port->SetBaud(19200);
		return true;
	}
	return false;
}

//---------------------------------------------------------

bool Jtag2Commands::SignOn()
{
	StartFrame(&msg);
	*msg++ = CMND_GET_SIGN_ON;
	EndFrame(&msg);
	
	Jtag2SignOn *p = (Jtag2SignOn*)&msg[1];

	printf("\
Comm Version: %02x\n\
Master: 0x%02x%02x Boot=0x%02x HW=0x%02x\n\
Slave : 0x%02x%02x Boot=0x%02x HW=0x%02x\n\
Serial: %02x %02x %02x %02x %02x %02x\n\
Name  : %s\n",
	p->comm_id,
	p->m_fw_maj, p->m_fw_min, p->m_bl, p->m_hw,
	p->s_fw_maj, p->s_fw_min, p->s_bl, p->s_hw,
	p->serial[0], p->serial[1], p->serial[2],
	p->serial[3], p->serial[4], p->serial[5],
	p->device_id);

	return msg[0] == RSP_SIGN_ON;
}

//---------------------------------------------------------

bool Jtag2Commands::SignOff()
{
	StartFrame(&msg);
	*msg++ = CMND_SIGN_OFF;
	EndFrame(&msg);

	return msg[0] == RSP_OK;
}

//---------------------------------------------------------

bool Jtag2Commands::GetSync()
{
	StartFrame(&msg);
	*msg++ = CMND_GET_SYNC;
	EndFrame(&msg); //XXX timeout ???

	return msg[0] == RSP_OK;
}

//---------------------------------------------------------

bool Jtag2Commands::Enter()
{
	StartFrame(&msg);
	*msg++ = CMND_ENTER_PROGMODE;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::Leave()
{
	StartFrame(&msg);
	*msg++ = CMND_LEAVE_PROGMODE;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::ChipErase()
{
	StartFrame(&msg);
	*msg++ = CMND_CHIP_ERASE;
	EndFrame(&msg);
	
	if(*msg != RSP_OK) {
		printf("Jtag2Commands::ChipErase EndFrame FAILED\n");
		return false;
	}
	return true;
}

//---------------------------------------------------------

//u8 rmb[300];

bool Jtag2Commands::ReadMem(u8* p, u8 type, u32 count, u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND_READ_MEMORY;
	*msg++ = type;
	*msg++ = (u8)(count >>  0); //LSB
	*msg++ = (u8)(count >>  8);
	*msg++ = (u8)(count >> 16);
	*msg++ = (u8)(count >> 24); //MSB
	*msg++ = (u8)(address >>  0); //LSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >> 16);
	*msg++ = (u8)(address >> 24); //MSB
	EndFrame(&msg);

	if(*msg != RSP_MEMORY) {
		printf("Jtag2Commands::ReadMem EndFrame FAILED\n");
		return false;
	}
	msg++;
	for(u32 i = 0; i < count; i++) {
		*p++ = msg[i];
	}
	return true;
}

//---------------------------------------------------------

bool Jtag2Commands::WriteMem(u8* p, u8 type, u32 count, u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND_WRITE_MEMORY;
	*msg++ = type;
	*msg++ = (u8)(count >>  0); //LSB
	*msg++ = (u8)(count >>  8);
	*msg++ = (u8)(count >> 16);
	*msg++ = (u8)(count >> 24); //MSB
	*msg++ = (u8)(address >>  0); //LSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >> 16);
	*msg++ = (u8)(address >> 24); //MSB
	for(u32 i = 0; i < count; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg);

	if(*msg != RSP_OK) {
		printf("Jtag2Commands::WriteMem EndFrame FAILED\n");
		return false;
	}
	return true;
}

//---------------------------------------------------------===
//parameter sizes

u8 psizes[] = {
//  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F
 0, 2, 4, 1,  2, 1, 2, 1,  1, 1, 1, 2,  2, 1, 4, 1, //0
 1, 1, 1, 1,  2, 1, 0, 2,  2, 1, 1, 4,  4, 2, 4, 4, //1
 0, 0, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, //2
 0, 4, 4, 4,  0, 0, 0, 1,  1, 0, 0, 0,  0, 0, 0, 0, //3
 4, 4, 4, 4,  4, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, //4
};

//---------------------------------------------------------

bool Jtag2Commands::GetParameter(u8 p, u8 &v)
{
	if(psizes[p] != 1) {
		printf("Wrong size GetParameter %i\n", psizes[p]);
	}

	StartFrame(&msg);
	*msg++ = CMND_GET_PARAMETER;
	*msg++ = p;
	EndFrame(&msg);

	if(msg[0] != RSP_PARAMETER) {
		v = 0;
		return false;
	}

	v = msg[1];
	return true;
}

//---------------------------------------------------------

bool Jtag2Commands::GetParameter2(u8 p, u16 &v)
{
	if(psizes[p] != 2) {
		printf("Wrong size GetParameter2 %i\n", psizes[p]);
	}

	StartFrame(&msg);
	*msg++ = CMND_GET_PARAMETER;
	*msg++ = p;
	EndFrame(&msg);

	if(msg[0] != RSP_PARAMETER) {
		v = 0;
		return false;
	}

	v  = msg[2]; v <<= 8;
	v |= msg[1];
	return true;
}

//---------------------------------------------------------

bool Jtag2Commands::GetParameter4(u8 p, u32 &v)
{
	if(psizes[p] != 4) {
		printf("Wrong size GetParameter4 %i\n", psizes[p]);
	}

	StartFrame(&msg);
	*msg++ = CMND_GET_PARAMETER;
	*msg++ = p;
	EndFrame(&msg);

	if(msg[0] != RSP_PARAMETER) {
		v = 0;
		return false;
	}

	v  = msg[4]; v <<= 8;
	v |= msg[3]; v <<= 8;
	v |= msg[2]; v <<= 8;
	v |= msg[1];
	return true;
}

//---------------------------------------------------------===

bool Jtag2Commands::SetParameter (u8 p, u8 v)
{
	if(psizes[p] != 1) {
		printf("Wrong size SetParameter %i\n", psizes[p]);
	}

	StartFrame(&msg);
	*msg++ = CMND_SET_PARAMETER;
	*msg++ = p;
	*msg++ = v;
	EndFrame(&msg);

	return msg[0] == RSP_OK;
}

//---------------------------------------------------------

bool Jtag2Commands::SetParameter2(u8 p, u16 v)
{
	if(psizes[p] != 2) {
		printf("Wrong size SetParameter2 %i\n", psizes[p]);
	}

	StartFrame(&msg);
	*msg++ = CMND_SET_PARAMETER;
	*msg++ = p;
	*msg++ = v >>  0; //LSB
	*msg++ = v >>  8;
	EndFrame(&msg);

	return msg[0] == RSP_OK;
}

//---------------------------------------------------------

bool Jtag2Commands::SetParameter4(u8 p, u32 v)
{
	if(psizes[p] != 4) {
		printf("Wrong size SetParameter4 %i\n", psizes[p]);
	}

	StartFrame(&msg);
	*msg++ = CMND_SET_PARAMETER;
	*msg++ = p;
	*msg++ = (u8)(v >>  0); //LSB
	*msg++ = (u8)(v >>  8);
	*msg++ = (u8)(v >> 16);
	*msg++ = (u8)(v >> 24); //MSB

	EndFrame(&msg);

	return msg[0] == RSP_OK;
}

//-----------------------------------------------------------------------------+++++

bool Jtag2Commands::GetDaisyChain(u8 &ub, u8 &ua, u8 &bb, u8 &ba)
{
	StartFrame(&msg);
	*msg++ = CMND_GET_PARAMETER;
	*msg++ = PAR_DAISY_CHAIN_INFO;
	EndFrame(&msg);

	if(msg[0] == RSP_PARAMETER) {
		return false;
	}

	ub = msg[1];
	ua = msg[2];
	bb = msg[3];
	ba = msg[4];
	return msg[0] == RSP_OK;
}

//---------------------------------------------------------

bool Jtag2Commands::SetDaisyChain(u8 ub, u8 ua, u8 bb, u8 ba)
{
	StartFrame(&msg);
	*msg++ = CMND_SET_PARAMETER;
	*msg++ = PAR_DAISY_CHAIN_INFO;
	*msg++ = ub;
	*msg++ = ua;
	*msg++ = bb;
	*msg++ = ba;
	EndFrame(&msg);

	return msg[0] == RSP_OK;
}

//-----------------------------------------------------------------------------+++++
//Debug

u32 Jtag2Commands::GetPC()
{
	StartFrame(&msg);
	*msg++ = CMND_READ_PC;
	EndFrame(&msg);

	if(*msg != RSP_PC) {
		printf("Jtag2Commands::GetPC EndFrame FAILED\n");
		return 0;
	}

	u32 r;
	r  = msg[4]; r <<= 8;
	r |= msg[3]; r <<= 8;
	r |= msg[2]; r <<= 8;
	r |= msg[1];
	return r;
}

//---------------------------------------------------------

bool Jtag2Commands::SetPC(u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND_WRITE_PC;
	*msg++ = (u8)(address >>  0); //LSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >> 16);
	*msg++ = (u8)(address >> 24); //MSB
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::Go()
{
	StartFrame(&msg);
	*msg++ = CMND_GO;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::Stop(u8 hl)
{
	StartFrame(&msg);
	*msg++ = CMND_FORCED_STOP;
	*msg++ = hl;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::Reset(u8 hl)
{
	StartFrame(&msg);
	*msg++ = CMND_RESET;
	*msg++ = hl;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::Step(u8 hl, u8 type)
{
	StartFrame(&msg);
	*msg++ = CMND_SINGLE_STEP;
	*msg++ = hl;
	*msg++ = type;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::RunTo(u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND_RUN_TO_ADDR;
	*msg++ = (u8)(address >>  0); //LSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >> 16);
	*msg++ = (u8)(address >> 24); //MSB
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::GetBreakPoint(u8 number, Jtag2GetBreak *b)
{
	StartFrame(&msg);
	*msg++ = CMND_GET_BREAK;
	*msg++ = number;
	EndFrame(&msg);

	if(*msg != RSP_GET_BREAK) {
		printf("Jtag2Commands::GetBreak EndFrame FAILED\n");
		return false;
	}

	u32 r;
	b->type = msg[1];
	b->mode = msg[6];
	r  = msg[5]; r <<= 8;
	r |= msg[4]; r <<= 8;
	r |= msg[3]; r <<= 8;
	r |= msg[2];
	b->address = r;
	return true;


}
//---------------------------------------------------------

bool Jtag2Commands::SetBreakPoint(u8 type, u8 number, u8 mode, u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND_SET_BREAK;
	*msg++ = type;
	*msg++ = number;
	*msg++ = (u8)(address >>  0); //LSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >> 16);
	*msg++ = (u8)(address >> 24); //MSB
	*msg++ = mode;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::ClearBreakPoint(u8 number, u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND_CLR_BREAK;
	*msg++ = number;
	*msg++ = (u8)(address >>  0); //LSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >> 16);
	*msg++ = (u8)(address >> 24); //MSB
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::ClearEvents()
{
	StartFrame(&msg);
	*msg++ = CMND_CLEAR_EVENTS;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::RestoreTarget()
{
	StartFrame(&msg);
	*msg++ = CMND_RESTORE_TARGET;
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//---------------------------------------------------------

bool Jtag2Commands::ErasePageSPM(u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND_ERASEPAGE_SPM;
	*msg++ = (u8)(address >>  0); //LSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >> 16);
	*msg++ = (u8)(address >> 24); //MSB
	EndFrame(&msg);

	return (*msg == RSP_OK);
}

//-----------------------------------------------------------------------------#####
//JTAG ICE mkII framing

const unsigned short crc_table[256] =
{
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

//---------------------------------------------------------

u16 Checksum(u8 *message, u32 length) //excluding crc
{
	u16 crc = 0xFFFF;
	for(u32 i = 0; i < length; i++) {
		crc = (crc >> 8) ^ crc_table[(crc ^ message[i]) & 0xFF];
	}
	return crc;
}

//---------------------------------------------------------

bool IsChecksumOK(u8 *message, u32 length) //including crc
{
	if(!length) {
		return false;
	}
	u16 crc = Checksum(message,length-2);
	return message[length-2] == (crc & 0xFF) && message[length-1] == (crc >> 8);
}

//-----------------------------------------------------------------------------

void Jtag2Commands::StartFrame(u8** m)
{
	*m = &buf[8];
}

//---------------------------------------------------------

u16 seq16 = 0;

u32 Jtag2Commands::EndFrame(u8** m)
{
	u16 crc,sz;

	sz = *m - &buf[8];

	buf[0] = MESSAGE_START;
	buf[1] = (u8)(seq16);
	buf[2] = (u8)(seq16 >> 8);
	buf[3] = (u8)(sz);
	buf[4] = (u8)(sz >> 8);
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = TOKEN;
	sz += 8;

	crc = Checksum(buf,sz);
	buf[sz++] = (u8)(crc);
	buf[sz++] = (u8)(crc >> 8);

	port->Write(buf,sz);
	sz = (u8)port->Read(buf,1100);

	if(!IsChecksumOK(buf,sz)) {
		puts("JTAGICEmkII checksum FAILED");
	}

	seq16++;
	*m = &buf[8];
	return sz;
}

//-----------------------------------------------------------------------------####

bool STK500J2Commands::SignOn() { return cmdj2->SignOn(); };
bool STK500J2Commands::SignOff() { return cmdj2->SignOff(); };

//---------------------------------------------------------

void STK500J2Commands::StartFrame(u8** m)
{
	cmdj2->StartFrame(m);
	*(*m)++ = CMND_ISP_PACKET;
	count  = *m;
	*m += 2;
}

//---------------------------------------------------------
//Why on earth do CMND_ISP_PACKET want the return size ?!
//And its not even documented in AVR067 ?! grrrr

s8 retsizes[] = {
//  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F
 0, 0, 2, 3,  0, 2, 2, 0,  0, 0, 0, 0,  0, 0, 0, 0, //0x00
 2, 2, 2, 2, -1, 2,-1, 3,  4, 3, 4, 4,  4,-2, 0, 0, //0x10
 2, 2, 2, 2, -1, 2,-1, 2,  3, 2, 3, 3,  3, 2, 0, 0, //0x20
 2, 2, 2, 2, -1, 2,-1, 2,  3, 2, 3, 3,  3, 0, 0, 0, //0x30
};

//---------------------------------------------------------

u32 STK500J2Commands::EndFrame(u8** m)
{
	u16 u;
	u8 *command = count + 2;
	if(*command > 0x3F || retsizes[*command] == 0) {
		printf("EndFrameSTK Unknown STK500 command !!! %02x\n",*command);
		return 0;
	}
	if(retsizes[*command] > 0) {
		*count++ = retsizes[*command];
		*count++ = 0;
	}else
	if(retsizes[*command] == -1) { //Reading FLASH/EEPROM
		u = *++command;
		u <<= 8;
		u |= *++command;
		u += 3;
		*count++ = (u8)(u);
		*count++ = (u8)(u >> 8);
	}else
	if(retsizes[*command] == -2) { //CMD_SPI_MULTI
		u = *(command+2) + 3;
		*count++ = (u8)(u);
		*count++ = (u8)(u >> 8);
	}else{
		puts("EndFrameSTK unknown size !!!");
		return 0;
	}

	u = (u8)cmdj2->EndFrame(m);
	if(**m != RSP_SPI_DATA) {
		puts("EndFrameSTK expected RSP_SPI_DATA...");			
	}
	(*m)++; //XXX
	return u-1;
}

//-----------------------------------------------------------------------------####

/*u8 descx[] = { //temp m162 for debug
0xE7,0x6F,0xFF,0xFF,0xFE,0xFF,0xFF,0xEF,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x10,
0xC3,0x26,0xB6,0xFD,0xFE,0xFF,0xFF,0xEA,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x10,
0x02,0x18,0x00,0x30,0xF3,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x18,0x00,0x20,0xF3,0x0F,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

0x04,0x57,0x00,
0x80,0x00,
0x04,
0x80,0x1F,0x00,0x00,
0xBB,0x00,
0x00,0x40,0x00,0x00, fsz

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,

0x3E,0x3D,  -- SP

0x80,0x00, fpsz
0x00,
0x00,

0x01,0x00,0x00,
0x01, page
0x00, cache
0x00,0x01, sram
0x00, rst

0x00,0x00,0x00,

0x1C,0x00 };EECR */

typedef struct
{
	u8 ucRead[8]; //bit0 byte0 == IO 0x00 --- bit7 byte7 == IO 0x3F
	u8 ucReadShadow[8];
	u8 ucWrite[8];
	u8 ucWriteShadow[8];
	u8 ucExtRead[52];
	u8 ucExtReadShadow[52];
	u8 ucExtWrite[52];
	u8 ucExtWriteShadow[52];

	u8 ucIDRAddress;   //IO
	u8 ucSPMCRAddress; //RAM
	u8 ucRAMPZAddress; //IO

	u8 uiFlashPageSize[2]; //bytes, LE
	u8 ucEepromPageSize;   //bytes

	u8 ulBootAddress[4];   //smallest, word, LE
	u8 uiUpperExtIOLoc[2]; //RAM

	u8 ulFlashSize[4]; //bytes
	u8 ucEepromInst[20]; //dW
	u8 ucFlashInst[3]; //dW
	u8 ucSPHaddr; //IO
	u8 ucSPLaddr; //IO
 
	u8 uiFlashPages[2]; // number of pages in flash
	u8 ucDWDRAddress;
	u8 ucDWBasePC; //? m48 m88 m325p m3250p m329p m3290p m645 m6490
 
 	//True for ATmega16 16A 162 32 32A 64 64A 128 128A
	u8 ucAllowFullPageBitstream; //FALSE on ALL new parts, different algorithm on IR=6
	u8 uiStartSmallestBootLoaderSection[2]; //?? dW ??

	u8 EnablePageProgramming; // For JTAG parts only, default TRUE
	u8 ucCacheType; //default 0, 1 for AT90CANxxx
	u8 uiSramStartAddr[2];
	u8 ucResetType; // default 0
                            
	u8 ucPCMaskExtended;
	u8 ucPCMaskHigh;
	u8 ucEindAddress; //???
 
	u8 EECRAddress[2];
} jtag2_dd;


#include "Rtk-base/RConfig.h"

bool Jtag2Commands::SetDescriptor(Device *dev)
{
	u32 i;
	jtag2_dd dd;
	u8 *ddp = (u8*)&dd;

	dev->Load();//XXX debug

	RConfigNode *t,*cfg = dev->config;

	for(i = 0; i < sizeof(dd); i++) {
		*ddp++ = 0;
	}

	t = cfg->GetNode("OCD");
	t->GetBin("ucRead", dd.ucRead, 8);
	t->GetBin("ucWrite", dd.ucWrite, 8);
	t->GetBin("ucExtRead", dd.ucExtRead, 52);
	t->GetBin("ucExtWrite", dd.ucExtWrite, 52);
	t->GetBin("ucEepromInst", dd.ucEepromInst, 20);
	t->GetBin("ucFlashInst", dd.ucFlashInst, 3);

	dd.ucIDRAddress    = (u8)t->GetValue("OCDR");
	dd.ucDWDRAddress = dd.ucIDRAddress; //XXX ?!!
	dd.ucDWBasePC = (u8)t->GetValue("BASE_PC");
	i = t->GetValue("ucUpperExtIOLoc");
	dd.uiUpperExtIOLoc[0] = (u8)(i >> 0);
	dd.uiUpperExtIOLoc[1] = (u8)(i >> 8);

	t = cfg->GetNode("IO_PORTS");
	if((dd.ucRAMPZAddress = (u8)t->GetValue("RAMPZ"))) {
		dd.ucRAMPZAddress -= 0x20; //convert to IO
	}
	if(!(dd.ucSPMCRAddress = (u8)t->GetValue("SPMCSR"))) {
		 dd.ucSPMCRAddress = (u8)t->GetValue("SPMCR"); // a few old parts m32 (m8 m8515 m161 m163)
	}
	if((dd.ucSPHaddr = (u8)t->GetValue("SPH"))) {
		dd.ucSPHaddr -= 0x20;
	}
	if((dd.ucSPLaddr = (u8)t->GetValue("SPL"))) {
		dd.ucSPLaddr -= 0x20;
	}
	if((i = t->GetValue("EECR"))) {
		i -= 0x20;
		dd.EECRAddress[0] = (u8)(i >> 0);
		dd.EECRAddress[1] = (u8)(i >> 8);
	}

	t = cfg->GetNode("MEMORY");
	i = t->GetValue2("FLASH");
	dd.uiFlashPageSize[0] = (u8)(i >> 0);
	dd.uiFlashPageSize[1] = (u8)(i >> 8);
	dd.ucEepromPageSize = (u8)t->GetValue2("EEPROM");

	u32 fpsz = i;
	i = t->GetValue("FLASH");
	dd.ulFlashSize[0] = (u8)(i >>  0);
	dd.ulFlashSize[1] = (u8)(i >>  8);
	dd.ulFlashSize[2] = (u8)(i >> 16);
	dd.ulFlashSize[3] = (u8)(i >> 24);

	//dd.ucPCMaskExtended; ??
	//dd.ucPCMaskHigh; ??

	i /= fpsz;
	dd.uiFlashPages[0] = (u8)(i >> 0);
	dd.uiFlashPages[1] = (u8)(i >> 8);

	i = t->GetValue2("IRAM");
	dd.uiSramStartAddr[0] = (u8)(i >> 0);
	dd.uiSramStartAddr[1] = (u8)(i >> 8);

	i = cfg->GetValue("BOOT/BOOTSIZES");
	dd.ulBootAddress[0] = (u8)(i >>  0);
	dd.ulBootAddress[1] = (u8)(i >>  8);
	dd.ulBootAddress[2] = (u8)(i >> 16);
	dd.ulBootAddress[3] = (u8)(i >> 24);
	dd.uiStartSmallestBootLoaderSection[0] = dd.ulBootAddress[0]; //???
	dd.uiStartSmallestBootLoaderSection[1] = dd.ulBootAddress[1]; //???

	dd.ucAllowFullPageBitstream = (u8)t->GetValue("ucAllowFullPageBitstream",0); //default 0
	dd.EnablePageProgramming = (u8)t->GetValue("EnablePageProgramming",1); //default 1, false on xmega
	dd.ucCacheType = (u8)t->GetValue("ucCacheType",0); //default 0

//-------------------------------------

	u8 *p = (u8*)&dd;
	printf("Sizeof jtag2_dd 298 = %i\n",(int)sizeof(dd));
	StartFrame(&msg);
	*msg++ = CMND_SET_DEVICE_DESCRIPTOR;
	{	for(u32 i = 0; i < sizeof(dd); i++) { //data
		*msg++ = *p++;
	}}//XXX
	EndFrame(&msg);

	if(*msg != RSP_OK) {
		printf("SetDescriptor failed\n");
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------####
//-----------------------------------------------------------------------------####


