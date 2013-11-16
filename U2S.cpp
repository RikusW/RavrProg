// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"
#include <stdio.h>

int U2S_Plist[] = { -1, 2000000, 500000, 125000, 62500, 28800, 10000, 4000, 2000, 1310, 0 };
int U2S_Elist[] = { -1, 4000000, 2000000, 1333333, 1000000, 500000, 250000, 125000, 62500, 40000, 20000, 0 };
int JTAGICEmkIu_Plist[] = { -1, 1000000, 500000, 250000, 166000, 125000, 100000, 50000, 0 };

//udev rule for modem-manager
//SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", ENV{ID_MM_DEVICE_IGNORE}="1", ENV{ID_MM_CANDIDATE}="0"

//-----------------------------------------------------------------------------

ProgU2S::ProgU2S()
{
	STK500Commands *cmds5;
	cmd = cmds5 = cmdu2s = new U2SCommands();
	cmd->SetProg(this);

	cmdj1 = new Jtag1Commands();
	cmdj1->SetProg(this);

	interface = &boot;
	boot.next = &isp;
	isp.next = &hvpp;
	hvpp.next = &hvsp;
	hvsp.next = &mj;
	mj.next = &fwup;

	boot.SetCmd(this,cmds5); boot.cmdu2s = cmdu2s;
	fwup.SetCmd(this,cmds5); fwup.cmdu2s = cmdu2s;
	isp.SetCmd(this,cmds5);   isp.cmdu2s = cmdu2s;
	hvpp.SetCmd(this,cmds5); hvpp.cmdu2s = cmdu2s;
	hvsp.SetCmd(this,cmds5); hvsp.cmdu2s = cmdu2s;

	mj.prog = this;
	mj.cmd = mj.cmdj1 = cmdj1;
	mj.cmdu2s = cmdu2s;

	Clock = 8000000;
	isp.PClockList = U2S_Plist;
	isp.EClockList = U2S_Elist;
	mj.PClockList = JTAGICEmkIu_Plist;

	pname = "U2S";
	pnm = "u2s";
	boot.iname= "U2S Bootloader";
	fwup.iname= "U2S Firmware Upgrade";
	isp.iname = "U2S ISP";
	hvpp.iname= "U2S HVPP";
	hvsp.iname= "U2S HVSP";
	mj.iname  = "U2S MEGA JTAG (mkI++)";
}

ProgU2S::~ProgU2S()
{
	delete cmdj1;
}

//-----------------------------------------------------------------------------

bool ProgIF_U2S_BOOT::Connect(const char *c)
{
	return (bConnected = cmdu2s->U2SConnect(c,0x81));
}

//---------------------------------------------------------

bool ProgIF_U2S_FWUP::Connect(const char *c)
{
	bConnected = false;
	if(!cmdu2s->U2SConnect(c,0x81)) {
		return false;
	}
	uint u;
	u = cmdu2s->GetAppSize();
	printf("Unlocking %x\n",u);
	cmdu2s->ModUnlock();
	u = cmdu2s->GetAppSize();
	printf("Unlocked  %x\n",u);
	bConnected = true;
	return true;
}

//---------------------------------------------------------

bool ProgIF_U2S_ISP::Connect(const char *c)
{
	return (bConnected = cmdu2s->U2SConnect(c,0x82));
}

//---------------------------------------------------------

bool ProgIF_U2S_HVPP::Connect(const char *c)
{
	return (bConnected = cmdu2s->U2SConnect(c,0x82));
}

//---------------------------------------------------------

bool ProgIF_U2S_HVSP::Connect(const char *c)
{
	return (bConnected = cmdu2s->U2SConnect(c,0x82));
}

//---------------------------------------------------------

bool ProgIF_U2S_MEGA_JTAG::Connect(const char *c)
{
	bConnected = false;
	if(!cmdu2s->U2SConnect(c,0x83)) {
		return false;
	}
	if(!cmdj1->SignOn()) {
		Disconnect();
		return false;
	}
	bConnected = true;
	return true;

/*
	u8 mode = 0xA3;
	printf("ProgIF_U2S_MEGA_JTAG::Connect -> %s - 0x%02x\n",c,mode);
	if((bConnected = prog->Connect(c))) { //XXX
		if(((ProgU2S*)prog2)->SetMode(mode)) { //XXX shared port !!!! trouble !!!!
			if(prog->SignOn()) {
				return true;
			}
		}
		Disconnect();
	}
	bConnected = false;
	return false;*/
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//U2SCommands
/*
static bool U2SConnect(ProgIF *i, const char *c , u8 mode)
{
	mode |= 0x20; //turn off led flashing
	printf("U2SConnect -> %s - 0x%02x\n",c,mode);
	if((i->bConnected = i->prog->Connect(c))) {
		if(!((ProgU2S*)i->prog)->SetMode(mode) || !i->prog->SignOn()) {
			i->Disconnect();
			i->bConnected = false;
		}
	}
	return i->bConnected;
}*/

//---------------------------------------------------------

bool U2SCommands::U2SConnect(const char *c , u8 mode)
{
	printf("U2SConnect -> %s - 0x%02x\n",c,mode);
	if(!Connect(c)) {
		return false;
	}

//	port->setDtr(); //exit Arduino Sketch

	//Exit dW and jtag modes and go to mode 81
	u8 buf[3] = { 0x4D, 0x4D, 0x4D }; //MMM
	port->Write(buf,3);

/*	if(!SignOn() || !SetMode(mode | 0x20)) { //turn off led flashing
		Disconnect();
		return false;
	}*/

	if(!SignOn()) { //STK500_2
		Disconnect();
		return false;
	}
	if(!SetMode(mode | 0x20)) { //turn off led flashing
		Disconnect();
		return false;
	}
	return true;
}

//---------------------------------------------------------

#define CMD_CUSTOM 0xFF

u8 U2SCommands::GetMode()
{
	StartFrame(&msg);
	*msg++ = CMD_CUSTOM;
	*msg++ = 0x01;
	EndFrame(&msg);

	return msg[2];
}

//---------------------------------------------------------

bool U2SCommands::SetMode(u8 mode)
{
	if((GetMode() & 0x8F) == (mode & 0x8F)) {
		return true;
	}

	StartFrame(&msg);
	*msg++ = CMD_CUSTOM;
	*msg++ = 0x00;
	*msg++ = mode; // sel passed to app
	EndFrame(&msg);
	
	return msg[1] == STATUS_CMD_OK;
}

//---------------------------------------------------------

u8 U2SCommands::GetVersion() //currently 4
{
	StartFrame(&msg);
	*msg++ = CMD_CUSTOM;
	*msg++ = 0x02; //getversion
	EndFrame(&msg);

	return msg[2];
}

//---------------------------------------------------------

u16 U2SCommands::GetAppSize()
{
	u16 u;
	StartFrame(&msg);
	*msg++ = CMD_CUSTOM;
	*msg++ = 0x03; //getsize
	EndFrame(&msg);

	u = msg[2];
	u <<= 8;
	u |=msg[1];

	return u;
}

//---------------------------------------------------------

u8 U2SCommands::ModUnlock()
{
	StartFrame(&msg);
	*msg++ = CMD_CUSTOM;
	*msg++ = 0x04; //unlock
	*msg++ = 0x5C; //magic
	EndFrame(&msg);

	return msg[1];
}

//-----------------------------------------------------------------------------

