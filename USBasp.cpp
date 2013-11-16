// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"

#include <stdio.h>

//-----------------------------------------------------------------------------

ProgUSBasp::ProgUSBasp()
{
	cmd = cmdua = new USBaspCommands();
	cmd->SetProg(this);

	usb.vid = USBASP_VID;
	usb.pid = USBASP_PID;

	interface = &isp;
	isp.next = &tpi;

	isp.prog = this; isp.cmd = isp.cmdua = cmdua;
	tpi.prog = this; tpi.cmd = tpi.cmdua = cmdua;

	pname = "USBasp";
	pnm = "usbasp";
	isp.iname = "USBasp ISP xxx";
	tpi.iname = "USBasp TPI xxx";
}

//-----------------------------------------------------------------------------

u8 usbasp_ret[17] = { 0,   0, 0, 4, -1, 1,   -2, -1, -2, 0, 1,  0, 0, 1, 0, -1,  0 };

//-----------------------------------------------------------------------------

int USBasp_Plist[] = { -1,
1500000, 750000, 375000, 187500, 93750, //HW
32000, 16000, 8000, 4000, 2000, 1000, 500, 0 }; //SW

ProgIF_USBasp_ISP::ProgIF_USBasp_ISP()
{
	sck = 3;
	inm = "isp";
	bDefaultP = false;
	PClockList = USBasp_Plist;
}

//---------------------------------------------------------

bool ProgIF_USBasp_ISP::Enter()
{
	StartFrame(&msg);
	*msg++ = USBASP_FUNC_CONNECT;
	EndFrame(&msg);

	StartFrame(&msg);
	*msg++ = USBASP_FUNC_ENABLEPROG; //1
	EndFrame(&msg);

	return !msg[0]; 
}

//---------------------------------------------------------

bool ProgIF_USBasp_ISP::Leave()
{
	StartFrame(&msg);
	*msg++ = USBASP_FUNC_DISCONNECT;
	EndFrame(&msg);

	return true;
}

//---------------------------------------------------------=====

u8 ProgIF_USBasp_ISP::ReadSig(u8 u)
{
	StartFrame(&msg);
	*msg++ = USBASP_FUNC_TRANSMIT;
	*msg++ = 0x30;
	*msg++ = 0;
	*msg++ = u;
	*msg++ = 0;
	EndFrame(&msg);

	return msg[3];
}

//---------------------------------------------------------

u8 ProgIF_USBasp_ISP::ReadCal(u8 u)
{
	StartFrame(&msg);
	*msg++ = USBASP_FUNC_TRANSMIT;
	*msg++ = 0x38;
	*msg++ = 0;
	*msg++ = u;
	*msg++ = 0;
	EndFrame(&msg);

	return msg[3];
}

//---------------------------------------------------------=====
/*
#define RF_LFUSE_ISP 0x00 //lf 50 00 00 oo
#define RF_LOCK_ISP  0x80 //lk 58 00 00 oo
#define RF_EFUSE_ISP 0x08 //ef 50 08 00 oo
#define RF_HFUSE_ISP 0x88 //hf 58 08 00 oo*/

u8 ProgIF_USBasp_ISP::ReadFuse(u8 a, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return 0;
	}

	u8 c[4] = { 0x00, 0x88, 0x08, 0x80 };
	u8 u = c[a & 3];

	StartFrame(&msg);
	*msg++ = USBASP_FUNC_TRANSMIT;
	*msg++ = 0x50 | ((u>>4)&0x0F);
	*msg++ = u & 0x0F;
	*msg++ = 0;
	*msg++ = 0;
	EndFrame(&msg);

//	printf("0x%02X\n", msg[3]);
	return msg[3];
}

//---------------------------------------------------------=====
/*
#define WF_LFUSE_ISP 0xA0 //lf AC A0 00 ii
#define WF_LOCK_ISP  0xE0 //lk AC E0 00 ii
#define WF_EFUSE_ISP 0xA4 //ef AC A4 00 ii
#define WF_HFUSE_ISP 0xA8 //hf AC A8 00 ii*/

void ProgIF_USBasp_ISP::WriteFuse(u8 a, u8 v, u8 ov, const char *name)
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
//	DELAY(10);

	u8 c[4] = { 0xA0, 0xA8, 0xA4, 0xE0 };
	u8 u = c[a & 3];

	StartFrame(&msg);
	*msg++ = USBASP_FUNC_TRANSMIT;
	*msg++ = 0xAC;
	*msg++ = u;
	*msg++ = 0;
	*msg++ = v;
	EndFrame(&msg);

//	printf("0x%02X %s\n",v,(msg[1] == STATUS_CMD_OK && msg[2] == STATUS_CMD_OK) ? "OK" : "FAILED");
}

//-----------------------------------------------------------------------------

bool ProgIF_USBasp_ISP::SetAddress(u32 a)
{
	StartFrame(&msg);
	*msg++ = USBASP_FUNC_SETLONGADDRESS;
	*msg++ = (u8)(a);
	*msg++ = (u8)(a >> 8);
	*msg++ = (u8)(a >> 16);
	*msg++ = (u8)(a >> 24);
	EndFrame(&msg);

	return true;
}

//---------------------------------------------------------

void ProgIF_USBasp_ISP::ChipErase()
{
	StartFrame(&msg);
	*msg++ = USBASP_FUNC_TRANSMIT;
	*msg++ = 0xAC;
	*msg++ = 0x80;
	*msg++ = 0x00;
	*msg++ = 0x00;
	EndFrame(&msg);
//	DELAY(100);
}

//---------------------------------------------------------
/*
void xxx()
{
	u32 a,c;

	StartFrame(&msg);
	*msg++ = USBASP_FUNC_READFLASH;
	*msg++ = a;
	*msg++ = a >> 8;
	*msg++ = 0;
	*msg++ = 0;
	*msg++ = c;
	*msg++ = c >> 8;
	EndFrame(&msg);


	StartFrame(&msg);
	*msg++ = USBASP_FUNC_WRITEFLASH;
	*msg++ = a;
	*msg++ = a >> 8;
	*msg++ = 0; //pgsz
	*msg++ = 0; //flags
	*msg++ = c;
	*msg++ = c >> 8;
	EndFrame(&msg);


	StartFrame(&msg);
	*msg++ = USBASP_FUNC_READEEPROM;
	*msg++ = a;
	*msg++ = a >> 8;
	*msg++ = 0;
	*msg++ = 0;
	*msg++ = c;
	*msg++ = c >> 8;
	EndFrame(&msg);


	StartFrame(&msg);
	*msg++ = USBASP_FUNC_WRITEEEPROM;
	*msg++ = a;
	*msg++ = a >> 8;
	*msg++ = 0;
	*msg++ = 0;
	*msg++ = c;
	*msg++ = c >> 8;
	EndFrame(&msg);
}*/

//-----------------------------------------------------------------------------

int ProgIF_USBasp_ISP::SetPClock(int f)
{
	Hz2P(f);
	int r = P2Hz();
	printf("ProgIF_USBasp_ISP::SetPClock %i => %i (sck = %02x)\n",f,r,sck);
	return r;
}

//---------------------------------------------------------

int ProgIF_USBasp_ISP::ApplyPClock()
{
	int r;
	if(bDefaultP) {
		Hz2P(-1);
		r = P2Hz();
		printf("Default Programming clock %i (sck = %02x)\n",r,sck);
	}else{
		r = P2Hz();
		printf("Applying Programming clock %i (sck = %02x) ",r,sck);

		StartFrame(&msg);
		*msg++ = USBASP_FUNC_SETISPSCK;
		*msg++ = sck;
		EndFrame(&msg); //read back 1...
	}
	return r;
}

//---------------------------------------------------------

void ProgIF_USBasp_ISP::Hz2P(int u)
{
	if(u >= 1500000) sck = USBASP_ISP_SCK_1500;  else
	if(u >=  750000) sck = USBASP_ISP_SCK_750;   else
	if(u >=  375000) sck = USBASP_ISP_SCK_375;   else
	if(u >=  187500) sck = USBASP_ISP_SCK_187_5; else
	if(u >=   93750) sck = USBASP_ISP_SCK_93_75; else
	if(u >=   32000) sck = USBASP_ISP_SCK_32;    else
	if(u >=   16000) sck = USBASP_ISP_SCK_16;    else
	if(u >=    8000) sck = USBASP_ISP_SCK_8;     else
	if(u >=    4000) sck = USBASP_ISP_SCK_4;     else
	if(u >=    2000) sck = USBASP_ISP_SCK_2;     else
	if(u >=    1000) sck = USBASP_ISP_SCK_1;     else
	if(u >=     500) sck = USBASP_ISP_SCK_0_5;   else
	if(u ==      -1) sck = USBASP_ISP_SCK_AUTO;
}

//---------------------------------------------------------

int ProgIF_USBasp_ISP::P2Hz()
{
	switch(sck) {
	case USBASP_ISP_SCK_AUTO   : return 375000;
	case USBASP_ISP_SCK_0_5    : return    500; //SW
	case USBASP_ISP_SCK_1      : return   1000;
	case USBASP_ISP_SCK_2      : return   2000;
	case USBASP_ISP_SCK_4      : return   4000;
	case USBASP_ISP_SCK_8      : return   8000;
	case USBASP_ISP_SCK_16     : return  16000;
	case USBASP_ISP_SCK_32     : return  32000;
	case USBASP_ISP_SCK_93_75  : return   93750; //HW
	case USBASP_ISP_SCK_187_5  : return  187500;
	case USBASP_ISP_SCK_375    : return  375000;
	case USBASP_ISP_SCK_750    : return  750000;
	case USBASP_ISP_SCK_1500   : return 1500000;
	}
	return 0;
}

//-----------------------------------------------------------------------------

