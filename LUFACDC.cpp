// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"

#include <stdio.h>

//-----------------------------------------------------------------------------

ProgLUFACDC::ProgLUFACDC()
{
	cmd = cmdlc = new LUFACDCCommands();
	cmd->SetProg(this);

	interface = &boot;
	boot.prog = this; boot.cmd = boot.cmdlc = cmdlc;

	pname = "LUFACDC";
	pnm = "lufacdc";
}

//-----------------------------------------------------------------------------

ProgIF_LUFACDC_Boot::ProgIF_LUFACDC_Boot()
{
	inm = "boot";
	iname = "LUFACDC Bootloader";
}

//---------------------------------------------------------

void ProgIF_LUFACDC_Boot::SetDevice(Device *)
{
	fwblock = fpagesize; //ISP write using page sized blocks
	ewblock = epagesize; //??
}

//---------------------------------------------------------

void ProgIF_LUFACDC_Boot::Disconnect()
{
	StartFrame(&msg);
	*msg++ = 'E';
	EndFrame(&msg,1);

	ProgIF::Disconnect();
}

//---------------------------------------------------------

bool ProgIF_LUFACDC_Boot::Enter()
{
	StartFrame(&msg);
	*msg++ = 'P';
	EndFrame(&msg,1);
	if(msg[0] != '\r') {
		return false;
	}
	StartFrame(&msg); //check for block support
	*msg++ = 'b';
	EndFrame(&msg,1);
	if(msg[0] != 'Y') {
		return false;
	}
	cmdlc->RxFrame(&msg,2);
	u32 pagesize = msg[1];
	pagesize |= ((u32)msg[0] << 8);

	//XXX compare and check here...

	return true;
}

//---------------------------------------------------------

bool ProgIF_LUFACDC_Boot::Leave()
{
	StartFrame(&msg);
	*msg++ = 'L';
	EndFrame(&msg,1);

	return msg[0] == '\r';
}

//---------------------------------------------------------=====

u32 ProgIF_LUFACDC_Boot::ReadSignature()
{
	StartFrame(&msg);
	*msg++ = 's';
	EndFrame(&msg,3);

	u32 r;
	r  = msg[0];
	r |= ((u32)msg[1]) << 8;
	r |= ((u32)msg[2]) << 16;
	return r;
}

//---------------------------------------------------------

u8 ProgIF_LUFACDC_Boot::ReadCal(u8)
{
	return 0;
}

//---------------------------------------------------------=====

u8 ProgIF_LUFACDC_Boot::ReadFuse(u8 a, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return 0;
	}

	u8 c[4] = { 'F', 'N', 'Q', 'r' };
	u8 u = c[a & 3];

	StartFrame(&msg);
	*msg++ = u;
	EndFrame(&msg,1);

	return msg[0];
}

//---------------------------------------------------------=====

void ProgIF_LUFACDC_Boot::WriteFuse(u8, u8, u8, const char*)
{
}

//-----------------------------------------------------------------------------

bool ProgIF_LUFACDC_Boot::SetAddress(u32 a)
{
	StartFrame(&msg);
	*msg++ = 'A';
	*msg++ = (u8)(a >> 8);
	*msg++ = (u8)(a);
	EndFrame(&msg,1);

	return msg[0] == '\r';
}

//---------------------------------------------------------

void ProgIF_LUFACDC_Boot::ChipErase()
{
	StartFrame(&msg);
	*msg++ = 'e';
	EndFrame(&msg,1);

	//return msg[0] == '\r';
}

//-----------------------------------------------------------------------------

bool ProgIF_LUFACDC_Boot::ReadFlashBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = 'g';
	*msg++ = (u8)(size >> 8);
	*msg++ = (u8)(size);
	*msg++ = 'F';
	EndFrame(&msg,(u16)size);
	for(u32 i = 0; i < size; i++) { //data
		*p++ = *msg++;
	}

	return msg[0] == '\r';
}

//---------------------------------------------------------

//must be one page at a time
bool ProgIF_LUFACDC_Boot::WriteFlashBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = 'B';
	*msg++ = (u8)(size >> 8);
	*msg++ = (u8)(size);
	*msg++ = 'F';
	for(u32 i = 0; i < size; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg,1);

	return msg[0] == '\r';
}

//---------------------------------------------------------

bool ProgIF_LUFACDC_Boot::ReadEepromBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = 'g';
	*msg++ = (u8)(size >> 8);
	*msg++ = (u8)(size);
	*msg++ = 'E';
	EndFrame(&msg,(u16)size);
	for(u32 i = 0; i < size; i++) { //data
		*p++ = *msg++;
	}

	return msg[0] == '\r';
}

//---------------------------------------------------------

bool ProgIF_LUFACDC_Boot::WriteEepromBlock(u8 *p, u32 size, u32)
{
	StartFrame(&msg);
	*msg++ = 'B';
	*msg++ = (u8)(size >> 8);
	*msg++ = (u8)(size);
	*msg++ = 'E';
	for(u32 i = 0; i < size; i++) { //data
		*msg++ = *p++;
	}
	EndFrame(&msg,1);

	return msg[0] == '\r';
}

//-----------------------------------------------------------------------------

#include <string.h>

bool LUFACDCCommands::SignOn()
{
	StartFrame(&msg);
	*msg++ = 'S';
	EndFrame(&msg,7);

	return !strcmp((const char *)msg,"LUFACDC");
}

//---------------------------------------------------------

bool LUFACDCCommands::SignOff()
{
	return true;
}

//---------------------------------------------------------

u32 ProgIF_LUFACDC_Boot::EndFrame(u8** msg, u16 c)
{
	return cmdlc->EndFrame(msg,c);
}

//---------------------------------------------------------

void LUFACDCCommands::StartFrame(u8** msg)
{
	*msg = buf;
}

//---------------------------------------------------------

u32 LUFACDCCommands::EndFrame(u8** msg, u16 c)
{
	u16 sz = *msg - buf;
	port->Write(buf,sz);
	if(!RxFrame(msg,c)) {
		printf("LUFACDCCommands::EndFrame sync error\n");
		return 0;
	}
	return 1;
}

//---------------------------------------------------------

bool LUFACDCCommands::RxFrame(u8** msg, u16 c)
{
	*msg = buf;
	return port->Read(*msg,c) == c;
}


//-----------------------------------------------------------------------------

