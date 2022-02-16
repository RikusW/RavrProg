// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"
#include <stdio.h>

//#include <unistd.h> //for usleep
#include <string.h>

//-----------------------------------------------------------------------------

#ifdef WIN32

//TODO add winusb support ?
//TODO add win libusb support ?
//TODO maybe add Jungo support ?

#else //---------------------------------

//Select one, or none

#define LIBUSB

#endif

//-----------------------------------------------------------------------------

//My own programmer :)
ProgU2S u2s;

//Atmel Programmers
ProgSTK500 stk500;
ProgSTK600 stk600;
ProgAVRISPmkI ispmki;
ProgAVRISPmkII ispmkii;

ProgDragon dragon;
ProgJTAGICEmkI jtagmki;
ProgJTAGICEmkIIu jtagmkiiu;
ProgJTAGICEmkIIs jtagmkiis;

//Other programmers
//ProgUSBasp usbasp;
ProgLUFACDC lufacdc; //partial AVR109 implementation

Device *device_list = 0;

//---------------------------------------------------------

Programmer *GetProgrammerList()
{
	device_list = GetAvrList();
	
	u2s.next = &stk500;
	stk500.next = &stk600;
	stk600.next = &ispmki;
	ispmki.next = &ispmkii;
	ispmkii.next = 	&dragon;
	dragon.next = &jtagmki;
	jtagmki.next = &jtagmkiiu;
	jtagmkiiu.next = &jtagmkiis;
	jtagmkiis.next = &lufacdc;

	return &u2s;
}

//---------------------------------------------------------

void FreeProgrammerList()
{
	delete device_list;
}

//-----------------------------------------------------------------------------
//Programmer

Programmer::Programmer()
{
	cmd = 0;
	port = 0;
	next = 0;
	interface = 0;
	pname = "Programmer Name";
	pnm = "pname";
}

//---------------------------------------------------------

Programmer::~Programmer()
{
	delete cmd;
}

//---------------------------------------------------------

Programmer *Programmer::GetAt(int i)
{
	if(i) {
		if(next) {
			return next->GetAt(i-1);
		}else{
			printf("Programmer GetAt failed %i\n",i);
			return 0;
		}
	}else{
		return this;
	}
}

//Programmer
//-----------------------------------------------------------------------------

void Commands::SetProg(Programmer *p)
{
	buf = p->buf;
	port = p->port;
}

//---------------------------------------------------------
/*
void Commands::SetBP(u8 *b, ProgPort *p);
{
	buf = b;
	port = p;
}*/

//---------------------------------------------------------

bool Commands::Connect(const char *c)
{
	if(port) {
		return port->Open(c);
	}
	printf("Commands::Connect null port, forgot to use SetProg ?\n");
	return false;
}

//---------------------------------------------------------

void Commands::Disconnect()
{
	if(port) {
		port->Close();
		return;
	}
	printf("Commands::Disconnect null port, forgot to use SetProg ?\n");
}

//-----------------------------------------------------------------------------

void PCStub(u32,u32,void*) {}

ProgIF::ProgIF()
{
	iname = "Interface Name";
	inm = "ifname";

	cmd = 0;
	next = 0;
	PClockList = 0;
	EClockList = 0;
	bDefaultP = true;
	bDefaultE = true;
	ProgressCallback = PCStub;

	address = 0;
	frblock = erblock = fwblock = ewblock = 256;
}

//---------------------------------------------------------

Device *ProgIF::GetDeviceList()
{
	u16 f = (u16)GetDevIf();

	int i = 0;
	Device *r,*d;
	for(r = device_list, d = device_list->nextl; d; d = d->nextl) {
		if(d->if_flags & f) {
			r->i = i;
			r->next = d;
			r = d;
			i++;
		}
	}
	r->i = i;
	r->next = 0;
	return device_list;
}

//---------------------------------------------------------

Device *ProgIF::DetectDevice()
{
	if(GetDevIf() & DIF_JMASK) {
		//auto detect using jtagid
		u32 dcnt = 0;
		Device *db[10];
		for(Device *d = device_list; d; d = d->next) {
			if(d->jtag_id == (jtag_id & 0xFFFFFFF)) {
				printf("Detected %s\n",d->name);
				db[dcnt++] = d;
			}
		}
		if(dcnt == 0) {
			printf("Failed to detect avr\n");
			return device_list; //auto
		}
/*		if(dcnt == 1) {
			return db[0];
		}*/
		if(dcnt > 1) {
			printf("Detected multiple matching jtag_id's\n");
		}

		//auto detect using jtagid and signature
		u32 scnt = 0;
		Device *sb[10];
		for(u32 u = 0; u < dcnt; u++) {
			if(db[u]->signature == signature) {
				printf("Detected matching signature %s\n",db[u]->name);
				sb[scnt++] = db[u];
			}
		}
		if(scnt == 0) {
			printf("Failed to find a matching signature\n");
			return device_list; //auto
		}
		if(scnt > 1) {
			printf("Detected multiple matching signatures, using %s\n",sb[0]->name);
		/*	for(u32 u = 0; u < scnt; u++) {
				printf("Matched -> %s\n",sb[u]->name);
			}*/
		}
		return sb[0];
	}else{
		//auto detect using signature
		u32 scnt = 0;
		Device *sb[10];
		for(Device *d = device_list; d; d = d->next) {
			if((d->signature == signature)) {
				printf("Detected %s\n",d->name);
				sb[scnt++] = d;
			}
		}
		if(scnt == 0) {
			printf("Failed to detect avr\n");
			return device_list; //auto
		}
		if(scnt > 1) {
			printf("Detected multiple matching signatures, using %s\n",sb[0]->name);
		}
		return sb[0];
	}
}

//---------------------------------------------------------

Device *ProgIF::GetDevice(int i)
{
	Device *d = device_list->GetAt(i); //???XXX
	return d;
}

//---------------------------------------------------------

Device *ProgIF::SetDevice(int i)
{
	int j = i;

	if(!bConnected) {
		return device_list->GetAt(i);
	}

	if(GetDevIf() & DIF_JMASK) {
		jtag_id = ReadJtagID();
		printf("Reading JTAG ID: %08X\n",(uint)jtag_id);
	}else{
		jtag_id = -1;
	}

	if(!i && (GetDevIf() & (DIF_HVPP | DIF_HVSP))) {
		j = 1; //XXX hack !!! do better HVPP detection...
	}

	if(j) {
		device = device_list->GetAt(j);
		device->Load();
		SetDefaults(device);
		SetDevice(device);
	}

	if(Enter()) {
		signature = ReadSignature();
		printf("Reading Signature: %06X\n",(uint)signature);
	}else{
		printf("WARNING Could not read the signature");
		signature = -1;
	}
	Leave();

	if(!i) {
		device = DetectDevice();
		if(device->i) {
			device->Load();
			SetDefaults(device);
			SetDevice(device);
		}
	}else{
		if((GetDevIf() & DIF_JMASK) && (device->jtag_id != (jtag_id & 0xFFFFFFF))) {
				printf("WARNING JTAG id does not match selected device\n");
		}
		if((device->signature != signature)) {
			printf("WARNING Signature does not match selected device\n");
		}
	}
	return device;
}
//---------------------------------------------------------

#include "Rtk-base/RConfig.h"

void ProgIF::SetDefaults(Device *d)
{
	RConfigNode *t,*n = d->config;
	if(!n) {
		printf("ERROR failed to load device config file\n");
		return;
	}
	t = n->GetNode("MEMORY");
	if(!t) {
		printf("ERROR failed to read the MEMORY node\n");
		return;
	}

	fsize = t->GetValue("FLASH");
	esize = t->GetValue("EEPROM");
	fpagesize = t->GetValue2("FLASH");
	epagesize = t->GetValue2("EEPROM");
}

//---------------------------------------------------------

Device *ProgIF::GetDevice(const char *c)
{
	//device selection
	char dname[50];
	switch(*c) {
	case 't': strcpy(dname,"ATtiny"); strcat(dname,c+1); break;
	case 'm': strcpy(dname,"ATmega"); strcat(dname,c+1); break;
	case 'x': strcpy(dname,"ATxmega"); strcat(dname,c+1); break;
	case 'u': strcpy(dname,"AT32UC3"); strcat(dname,c+1); break;
	default: strcpy(dname,c);
	}
	for(Device *dev = device_list->next; ; dev = dev->next) {
		if(!dev) {
			printf("Device not found: %s (%s)\n",c,dname);
			return device_list; //Auto
		}
		if(!strcmp(dev->name,dname)) {
			printf("Device found: %s (%s)\n",c,dname);
			return dev;
		}
	}
	return device_list; //Auto
}

//---------------------------------------------------------

Device *ProgIF::SetDevice(const char *c)
{
	return SetDevice(GetDevice(c)->i);
}

//-----------------------------------------------------------------------------

bool ProgIF::Connect(const char *c)
{
	if(!cmd) {
		puts("ProgIF Connect cmd == NULL !!!\n");
		bConnected = false;
		return false;
	}

	printf("ProgIF::Connect -> %s\n",c);
	if((bConnected = cmd->Connect(c))) {
		if(!(bConnected = cmd->SignOn())) {
			Disconnect();
		}
	}
	return bConnected;
}

//---------------------------------------------------------

void ProgIF::Disconnect()
{
	if(cmd && bConnected) { //XXX
		cmd->SignOff();
		cmd->Disconnect();
	}else{
		puts("ProgIF::Disconnect cmd == 0, shouldn't ever happen...");
	}
	bConnected = false;
}

//---------------------------------------------------------

bool ProgIF::Enter() { return cmd ? cmd->Enter() : false; }
bool ProgIF::Leave() { return cmd ? cmd->Leave() : false; }

//---------------------------------------------------------

void ProgIF::Delay(u32 d)
{
//	usleep(d * 1000);
}

//-----------------------------------------------------------------------------

int ProgIF::SetPClockI(int u)
{
	int *p = GetPClockList();
	if(!p) {
		return 0;
	}
	for(int i = 0; i < 100; i++) {
		if(i == u) {
			break;
		}
		if(!p[i]) {
			return 0;
		}
	}
	return SetPClock(p[u]);
}

//---------------------------------------------------------

int ProgIF::SetEClockI(int u)
{
	int *p = GetEClockList();
	if(!p) {
		return 0;
	}
	for(int i = 0; i < 100; i++) {
		if(i == u) {
			break;
		}
		if(!p[i]) {
			return 0;
		}
	}
	return SetEClock(p[u]);
}

//-----------------------------------------------------------------------------

ProgIF *ProgIF::GetAt(int i)
{
	if(i) {
		if(next) {
			return next->GetAt(i-1);
		}else{
			printf("ProgIF GetAt failed %i\n",i);
			return 0;
		}
	}else{
		return this;
	}
}

//-----------------------------------------------------------------------------

void ProgIF::StartFrame(u8** msg)
{
	if(cmd) {
		cmd->StartFrame(msg);
	}else{
		puts("ProgIF StartFrame cmd == NULL !!!\n");
	}
}

u32 ProgIF::EndFrame(u8** msg)
{
	if(cmd) {
		return cmd->EndFrame(msg);
	}else{
		puts("ProgIF EndFrame cmd == NULL !!!\n");
		return 0;
	}
}

//-----------------------------------------------------------------------------

u32 ProgIF::ReadSignature()
{
	u32 r;
	r  = ReadSig(0); r <<= 8;
	r |= ReadSig(1); r <<= 8;
	r |= ReadSig(2);
	return r;
}

//---------------------------------------------------------

void ProgIF::ReadSerial(u8 *buf)
{
	//XXX check if serial is supported ?

	for(u8 o = 0x07; o<=0x0B; o++) { // sig(0x0E 0x17) ---- ???(0x18 0x1D)
		*buf++ = ReadCal(o);
		*buf++ = ReadSig(o);
	}
}

//-----------------------------------------------------------------------------

u8 ProgIF::ReadFuses(u8 b[32])
{
	b[0] = ReadFuse(0,"Reading Low Fuse      ");
	b[1] = ReadFuse(1,"Reading High Fuse     ");
	b[2] = ReadFuse(2,"Reading Extended Fuse ");
	b[3] = ReadFuse(3,"Reading Lockbits      ");
	return 4;
}

//---------------------------------------------------------

void ProgIF::WriteFuses(u8 b[32])
{
	u8 buf[32];
	ReadFuses(buf);
	printf("\n");
	WriteFuse(0,b[0],buf[0],"Writing Low Fuse      ");
	WriteFuse(1,b[1],buf[1],"Writing High Fuse     ");
	WriteFuse(2,b[2],buf[2],"Writing Extended Fuse ");
	WriteFuse(3,b[3],buf[3],"Writing Lockbits      ");
	Delay(10);
	printf("\n");
	VerifyFuses(b);
}

//---------------------------------------------------------

bool ProgIF::VerifyFuses(u8 b[32])
{
	u8 buf[32];
	ReadFuses(buf);
	printf("\n");
	printf("Verifying Low Fuse       %s\n", b[0] == buf[0] ? "OK" : "FAILED");
	printf("Verifying High Fuse      %s\n", b[1] == buf[1] ? "OK" : "FAILED");
	printf("Verifying Extended Fuse  %s\n", b[2] == buf[2] ? "OK" : "FAILED");
	printf("Verifying Lock bits      %s\n", b[3] == buf[3] ? "OK" : "FAILED");

	return (b[0] == buf[0]) && (b[1] == buf[1]) && (b[2] == buf[2]) && (b[3] == buf[3]);
}

//-----------------------------------------------------------------------------#####
//Flash

void ProgIF::ReadFlash(const char *name, u8 type)
{
	printf("Reading Flash\n");
	if(type == HEXFW) {
		RHexFile hf(fsize + 512);
		hf.Clear();
		ReadFlash(hf.buf,fsize);
		hf.Write(name);
	}else
	if(type == BINFW) {
		printf("BINFW not supported yet\n");
	}else{
		printf("Unsupported file type\n");
	}
	printf("\nDONE\n");
}
	
//---------------------------------------------------------

/*DEBUG
void PrintBlock(u8 *p, u32 size, u32 addr)
{
	printf("0x%08X\n",addr);
	for(u32 u = 0; u < size; u++) {
		if(u % 64 == 0) {
			puts("--");
		}
		printf("%02X ",*p++);
	}
}
		PrintBlock(p,frblock,address);
*/

bool ProgIF::ReadFlash(u8 *p, u32 size)
{
	SetAddress(0);

	for(u32 u = 0; u < size; u += frblock) {
		ProgressCallback(size,u,PCBvoid);
		printf("Reading frame %i\n",(int)(u/frblock));

		if(!ReadFlashBlock(p,frblock,address)) {
			printf("Reading flash failed at %X\n",(uint)u);
		}
		p += frblock;
		address += frblock;
	}
	ProgressCallback(size,size,PCBvoid);
	return true;
}

//---------------------------------------------------------

bool ProgIF::ReadFlashBlock (u8*, u32, u32)
{
	printf("NOT IMPLEMENTED\n");
	return false;
}

//-----------------------------------------------------------------------------

void ProgIF::WriteFlash(const char *name, u8 type)
{
	printf("Writing Flash\n");
/*
	if(!Enter()) {
		printf("Failed to enter progmode...\n");
		Leave();
		return;
	}*/

	if(type == HEXFW) {
		RHexFile hf(fsize + 512);
		hf.Read(name);
		WriteFlash(hf.buf,fsize);
	}else
	if(type == BINFW) {
		printf("BINFW not supported yet\n");
	}else{
		printf("Unsupported file type\n");
	}
	printf("\nDONE\n");
//	Leave();
}
	
//---------------------------------------------------------

bool ProgIF::WriteFlash(u8 *p, u32 size)
{
	if(size > fsize) {
		printf("ERROR read request bigger than flash\n");
		return false;
	}
	if(size % fwblock) {
		printf("ERROR size not a multiple of blocksize\n");
		return false;
	}
	if(fwblock < fpagesize) {
		printf("ERROR large page support not implemented\n");
		return false;
	}

//aa	SetAddress(0);

	printf("page size = fwblock %x\n",(uint)fwblock);
	bool skipped = true; //aa false;
	u32 addr = 0;
	for(u32 u = 0; u < size; u += fwblock) {
		if(IsFFBlock(p,fwblock)) {
			printf("Skipping block %i at %x\n",(int)(u/fwblock),(uint)addr);
			skipped = true;
			goto skip;
		}
		if(skipped) {
			printf("Setting address to %x\n",(uint)addr); //dbg


			SetAddress(addr / 2); //XXX !!! word for stk500   byte for jtag ?!! XXX


			skipped = false;
		}
		printf("Writing block %i at %x\n",(int)(u/fwblock),(uint)addr);
		if(!WriteFlashBlock(p,fwblock,addr)) {
			printf("Writing flash failed at %X\n",(uint)u);
		}//*/
skip:		
		p += fwblock;
		addr += fwblock;
		ProgressCallback(size,u,PCBvoid);
	}
	ProgressCallback(size,size,PCBvoid);
	return true;
}

//---------------------------------------------------------

bool ProgIF::WriteFlashBlock(u8*, u32, u32)
{
	printf("NOT IMPLEMENTED\n");
	return false;
}

//-----------------------------------------------------------------------------

void ProgIF::VerifyFlash(const char *name, u8 type)
{
	name = 0;
	printf("Verifying Flash\n");
	if(type == HEXFW) {
		printf("HEXFW not supported yet\n");
	}else
	if(type == BINFW) {
		printf("BINFW not supported yet\n");
	}else{
		printf("Unsupported file type\n");
	}
	printf("\nDONE\n");
}

//Flash
//-----------------------------------------------------------------------------#####
//Eeprom

void ProgIF::ReadEeprom(const char *name, u8 type)
{
	printf("Reading Eeprom\n");
	if(type == HEXFW) {
		RHexFile hf(esize + 512);
		hf.Clear();
		ReadEeprom(hf.buf,esize);
		hf.Write(name);
	}else
	if(type == BINFW) {
		printf("BINFW not supported yet\n");
	}else{
		printf("Unsupported file type\n");
	}
	printf("\nDONE\n");

}

//---------------------------------------------------------

bool ProgIF::ReadEeprom(u8 *p, u32 size)
{
	SetAddress(0);

	for(u32 u = 0; u < size; u += erblock) {
		printf("Reading frame %i\n",(int)(u/erblock));
		if(!ReadEepromBlock(p,erblock,address)) {
			printf("Reading eeprom failed at %X\n",(uint)u);
		}
		p += erblock;
		address += erblock;
		ProgressCallback(size,u,PCBvoid);
	}
	ProgressCallback(size,size,PCBvoid);
	return true;
}

//---------------------------------------------------------

bool ProgIF::ReadEepromBlock (u8*, u32, u32)
{
	printf("NOT IMPLEMENTED\n");
	return false;
}

//-----------------------------------------------------------------------------

void ProgIF::WriteEeprom(const char *name, u8 type)
{
	printf("Writing Eeprom\n");
	if(type == HEXFW) {
		RHexFile hf(esize + 512);
		hf.Read(name);
		WriteEeprom(hf.buf,esize);
	}else
	if(type == BINFW) {
		printf("BINFW not supported yet\n");
	}else{
		printf("Unsupported file type\n");
	}
	printf("\nDONE\n");

}

//---------------------------------------------------------

bool ProgIF::WriteEeprom(u8 *p, u32 size)
{
//	SetAddress(0);

	u32 addr = 0;
	bool skipped = true;
	for(u32 u = 0; u < size; u += ewblock) {
		if(IsFFBlock(p,ewblock)) {
			printf("Skipping block %i\n",(int)(u/ewblock));
			skipped = true;
			goto skip;
		}
		if(skipped) {
			printf("Setting address to %x\n",(uint)addr); //dbg
			SetAddress(addr);
			skipped = false;
		}
		printf("Writing frame %i\n",(int)(u/ewblock));
		if(!WriteEepromBlock(p,ewblock,addr)) {
			printf("Writing eeprom failed at %X\n",(uint)u);
		}
skip:		
		p += ewblock;
		addr += ewblock;
		ProgressCallback(size,u,PCBvoid);
	}
	ProgressCallback(size,size,PCBvoid);
	return true;
}

//---------------------------------------------------------

bool ProgIF::WriteEepromBlock(u8*, u32, u32)
{
	printf("NOT IMPLEMENTED\n");
	return false;
}

//-----------------------------------------------------------------------------

void ProgIF::VerifyEeprom(const char *name, u8 type)
{
	name = 0;
	printf("Verifying Eeprom\n");
	if(type == HEXFW) {
		printf("HEXFW not supported yet\n");
	}else
	if(type == BINFW) {
		printf("BINFW not supported yet\n");
	}else{
		printf("Unsupported file type\n");
	}
	printf("\nDONE\n");
}

//---------------------------------------------------------

bool ProgIF::IsFFBlock(u8 *p, u32 size)
{
	for(u32 u = 0; u < size; u++) {
		if(p[u] != 0xFF) {
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------#####

void PrintHex(const char *m, u8 *buf, int sz)
{
	printf("%s %i bytes\n",m,sz);

	for(int i = 0; i < sz; i++) {
		printf("%02x ",buf[i]);
		if((i & 0xF) == 0xF) {
			printf("\n");
		}
	}
	printf("\n");
}

//-----------------------------------------------------------------------------#####
//RCom

#if 1

#include "Rtk-base/RCom.h"

RCom rc; //XXX

bool ProgPortSerial::Open(const char *c)
{
	return !rc.Open(c);
}

void ProgPortSerial::SetBaud(u32 b)
{
//	char buf[20];
//	snprintf(buf,15,"%u",(uint)b); // where is itoa ?!! grr

	RString s;
	s.ToDec(b);
	rc.SetBaudRate(s); //8N1 default
//	rc.SetTimeout(0);
	//hw flow ?  read timeout ?
}

void ProgPortSerial::Close()
{
	rc.Close();
}

u32 ProgPortSerial::Read(u8 *p,u32 u)
{
	return rc.Read(p,u);
}

u32 ProgPortSerial::Write(u8 *p,u32 u)
{
	return rc.Write(p,u);
}

#ifdef WIN32 //hack until RCom enum is done...
const char *ports[] = { "COM1", "COM2", "COM3", "COM4", "COM5", "COM6" };
#else
const char *ports[] = { "/dev/ttyS0", "/dev/ttyS1", "/dev/ttyS2", "/dev/ttyS3", "/dev/ttyACM0", "/dev/ttyACM1" };
#endif

const char *ProgPortSerial::GetPort(u32 u)
{
	if(u < (sizeof(ports)/sizeof(const char*))) {
		return ports[u];
	}
	return 0;
}

//RCom
//-----------------------------------------------------------------------------#####
#else
// serial port support off
bool ProgPortSerial::Open(const char *) { return false; }
void ProgPortSerial::SetBaud(u32) {}
void ProgPortSerial::Close() {}
u32 ProgPortSerial::Read(u8 *,u32) { return 0; }
u32 ProgPortSerial::Write(u8 *,u32) { return 0; }
const char *ProgPortSerial::GetPort(u32 u) { return u ? 0 : "Not supported"; }
#endif

//-----------------------------------------------------------------------------#####
//lib-usb

#ifdef LIBUSB

#include <usb.h>

bool bUsbInit = false;

bool ProgPortUSB::Open(const char *c)
{
	bool bFound = false;
	char serial[100];
	char product[100];

	char tt[40]; //usb_find_devices(); seems to destroy c ?!!
	if(c) {
		strcpy(tt,c);
	}else{
		tt[0] = 0;
	}

	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *udev;

	if(!bUsbInit) {
		bUsbInit = true;
		usb_init();
	}
	usb_find_busses();
	usb_find_devices();

	for(bus = usb_get_busses(); bus; bus = bus->next)
	for(dev = bus->devices;     dev; dev = dev->next) {
		udev = usb_open(dev);
		if(udev && dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid) {
			printf("ProgPortUSB::Open Found VID=%04x PID=%04x\n",vid,pid);

			usb_get_string_simple(udev,dev->descriptor.iProduct,product,sizeof(product));
			usb_get_string_simple(udev,dev->descriptor.iSerialNumber,serial,sizeof(serial));
			printf("ProgPortUSB::Open Found <%s> <%s>\n",product,serial);

			bFound = true;
			if(!*tt || !strcmp(tt,serial)) {
				hu = (void*)udev;
				//usb_detach_kernel_driver_np(udev,1);
				return true;
			}
		}
		usb_close(udev);
	}

	if(bFound) {
		puts("USB Programmer serial don't match");
	}else{
		puts("USB Programmer not found");
	}
	return false;
}

void ProgPortUSB::Close()
{
	usb_close((usb_dev_handle*)hu);
	hu = 0;
}

u32 ProgPortUSB::Read(u8 *p,u32 u)
{
	u8 *b = p;
	u32 sz = u, ret;

	printf("Reading %i bytes\n",(int)u);

	do {
		ret = usb_bulk_read((usb_dev_handle*)hu, ep_read, (char*)p, packet_size, 3000);
		p += ret;
		u -= ret;
	} while(ret == packet_size);
	
	PrintHex("Read",b,sz-u);

	return sz-u;
}

u32 ProgPortUSB::Write(u8 *p,u32 u)
{
	u32 sz = u,ret;

	PrintHex("Writing",p,u);

	while(u >= packet_size) {
		ret = usb_bulk_write((usb_dev_handle*)hu, ep_write, (char*)p, packet_size, 3000);
		p += ret;
		u -= ret;
	}
	ret = usb_bulk_write((usb_dev_handle*)hu, ep_write, (char*)p, u, 3000); //short packet
	p += ret;
	u -= ret;

	printf("Written %i bytes\n",(int)(sz-u));

	return sz-u;
}


struct usb_bus *bus;
struct usb_device *dev;
usb_dev_handle *udev;
char usbserials[20][20];

const char *ProgPortUSB::GetPort(u32 u)
{
	char serial[19];
	char product[19];

	if(!u) {
		int i = 0;
	
		if(!bUsbInit) {
			bUsbInit = true;
			usb_init();
		}
		usb_find_busses();
		usb_find_devices();
	
		for(bus = usb_get_busses(); bus; bus = bus->next)
		for(dev = bus->devices;     dev; dev = dev->next) {
			udev = usb_open(dev);
			if(udev && dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid) {
				printf("ProgPortUSB::GetPort Found VID=%04x PID=%04x\n",vid,pid);
	
				usb_get_string_simple(udev,dev->descriptor.iProduct,product,sizeof(product));
				usb_get_string_simple(udev,dev->descriptor.iSerialNumber,serial,sizeof(serial));
				printf("ProgPortUSB::GetPort Found <%s> <%s>\n",product,serial);
	
				strncpy(usbserials[i++],serial,18);
			}
			usb_close(udev);
		}
		usbserials[i][0] = 0;
		printf("Serials found: ");
		for(int j = 0; j < i; j++) printf("<%s>",usbserials[j]);
		printf("\n");
	}

	if(usbserials[u][0]) {
		return usbserials[u];
	}
	return 0;
}

#else //-----------------------------------------

bool ProgPortUSB::Open(const char *c)
{
	return false;
}

void ProgPortUSB::Close()
{
}

u32 ProgPortUSB::Read(u8 *p,u32 u)
{

	return 0;
}

u32 ProgPortUSB::Write(u8 *p,u32 u)
{
	return 0;
}

const char *ProgPortUSB::GetPort(u32 u)
{
	return 0;
}
#endif

//-----------------------------------------------------------------------------#####

