// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"
#include <stdio.h>

//-----------------------------------------------------------------------------

ProgDragon::ProgDragon()
{
	usb.pid = USB_DEVICE_AVRDRAGON;

	isp.next = &hvpp;
	hvpp.next = &hvsp;
	hvsp.next = &mj;

	hvpp.prog = this; hvpp.cmd = hvpp.cmds5 = cmds5j2; hvpp.cmdj2 = cmdj2;
	hvsp.prog = this; hvsp.cmd = hvsp.cmds5 = cmds5j2; hvsp.cmdj2 = cmdj2;

	pname = "AVR Dragon";
	pnm = "dragon";
	isp.iname = "AVR Dragon ISP";
	hvpp.iname= "AVR Dragon HVPP";
	hvsp.iname= "AVR Dragon HVSP";
	mj .iname = "AVR Dragon MEGA JTAG";
	pdi.iname = "AVR Dragon PDI xxx";
	xj .iname = "AVR Dragon XMEGA JTAG xxx";
	aj .iname = "AVR Dragon AVR32 JTAG xxx";
}

//-----------------------------------------------------------------------------

bool ProgIF_Dragon_HVPP::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
		if(!cmdj2->SetEmulatorMode(EMULATOR_MODE_HV)) {
			puts("cmdj2->SetEmulatorMode(EMULATOR_MODE_HV) FAILED");
			Disconnect();
			return false;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

bool ProgIF_Dragon_HVSP::Connect(const char *c)
{
	if(ProgIF::Connect(c)) {
		if(!cmdj2->SetEmulatorMode(EMULATOR_MODE_HV)) {
			puts("cmdj2->SetEmulatorMode(EMULATOR_MODE_HV) FAILED");
			Disconnect();
			return false;
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------

