// vim:ts=4 sts=0 sw=4

#include "RavrProg.h"

#include <stdio.h>
#include <string.h>

u8 rb[600];

//-----------------------------------------------------------------------------

int JTAGICEmkI_Plist[] = { -1, 1000000, 500000, 250000, 166000, 125000, 100000, 50000, 0 };

ProgJTAGICEmkI::ProgJTAGICEmkI()
{
	cmd = cmdj1 = new Jtag1Commands();
	cmd->SetProg(this);

	interface = &mj;
	mj.SetCmd(this,cmdj1);

	Clock = 7372800;
	mj.PClockList = JTAGICEmkI_Plist;

	pname = "JTAGICE mkI";
	pnm = "jtag1";

	mj.iname = "JTAGICE mkI MEGA JTAG";
}

//-----------------------------------------------------------------------------

void ProgIF_JTAGICEmkI_MEGA_JTAG::SetDevice(Device *d)
{
	cmdj1->SetDescriptor(d);
}

//---------------------------------------------------------

int ProgIF_JTAGICEmkI_MEGA_JTAG::SetPClock(int f)
{
	Hz2P(f);
	int r = P2Hz();
	printf("ProgIF_JTAGICEmkI_MEGA_JTAG::SetPClock %i => %i (jclk = %02x)\n",f,r,jclk);
	return r;
}

//---------------------------------------------------------

int ProgIF_JTAGICEmkI_MEGA_JTAG::ApplyPClock()
{
	int r;
	if(bDefaultP) {
		Hz2P(-1);
		r = P2Hz();
		printf("Default Programming clock %i (jclk = %02x)\n",r,jclk);
	}else{
		r = P2Hz();
		printf("Applying Programming clock %i (jclk = %02x) ",r,jclk);
		bool b = cmdj1->SetParameter(PAR1_JTAG_CLK,jclk);
		printf("%s\n",b ? "OK" : "FAILED");
	}
	return r;
}

//---------------------------------------------------------
	
void ProgIF_JTAGICEmkI_MEGA_JTAG::Hz2P(int f)
{
	bDefaultP = false;
	if(f == -1) {
		if(bConnected) {
			cmdj1->GetParameter(PAR1_JTAG_CLK,jclk);
		}else{
			jclk = 0xFE; //500kHz
		}
		bDefaultP = true;
	}else{
		if(f >= 1000000) { jclk = 0xFF; } else
		if(f >=  500000) { jclk = 0xFE; } else
		if(f >=  250000) { jclk = 0xFD; } else
		if(f >=  166000) { jclk = 0xFC; } else
		if(f >=  125000) { jclk = 0xFB; } else
		if(f >=  100000) { jclk = 0xFA; } else
		if(f >=   50000) { jclk = 0xF3; } else { jclk = 0xF3; } //XXX
	}
}

//---------------------------------------------------------

int ProgIF_JTAGICEmkI_MEGA_JTAG::P2Hz()
{
	int r;
	switch(jclk) {
	case 0xFF: r = 1000000; break;
	case 0xFE: r =  500000; break;
	case 0xFD: r =  250000; break;
	case 0xFC: r =  166000; break;
	case 0xFB: r =  125000; break;
	case 0xFA: r =  100000; break;
	case 0xF3: r =   50000; break;
	default: r = 1234; printf("Unknown jt1 jclk %02x",jclk); break; //XXX
	}
	return r;
}

//-----------------------------------------------------------------------------

u8 ProgIF_JTAGICEmkI_MEGA_JTAG::ReadSig(u8 a)
{
	cmdj1->ReadMem(rb,MEMP_SIGNATURE,1,a);
	return *rb;
}

//---------------------------------------------------------

u8 ProgIF_JTAGICEmkI_MEGA_JTAG::ReadCal(u8 a)
{
	cmdj1->ReadMem(rb,MEMP_OSCCAL,1,a);
	return *rb;
}

//---------------------------------------------------------

u32 ProgIF_JTAGICEmkI_MEGA_JTAG::ReadJtagID()
{
	u32 r = 0;
	u8 a,b,c,d;
	cmdj1->GetParameter(PAR1_JTAGID0,a);
	cmdj1->GetParameter(PAR1_JTAGID1,b);
	cmdj1->GetParameter(PAR1_JTAGID2,c);
	cmdj1->GetParameter(PAR1_JTAGID3,d);
	r  = d; r <<= 8;
	r |= c; r <<= 8;
	r |= b; r <<= 8;
	r |= a;

	cmdj1->GetParameter(0x70,d); //EECR
	printf("EECR detected as %02x.\n",d);

	return r;
}

//---------------------------------------------------------

u8 ProgIF_JTAGICEmkI_MEGA_JTAG::ReadFuse(u8 a, const char *name)
{
	printf("%s",name);
	if(a > 3) {
		printf("INVALID address %i\n",(uint)a);
		return 0;
	}
	bool b;
	if(a < 3) {
		b = cmdj1->ReadMem(rb,MEMP_FUSES,1,a);
	}else{
		b = cmdj1->ReadMem(rb,MEMP_LOCK,1,0);
	}
	printf("0x%02X %s\n", *rb, b ? "OK" : "FAILED");
	return *rb;
}

//---------------------------------------------------------

void ProgIF_JTAGICEmkI_MEGA_JTAG::WriteFuse(u8 a, u8 v, u8 ov, const char *name)
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

	bool b;
	if(a < 3) {
		b = cmdj1->WriteMem(rb,MEMP_FUSES,1,a);
	}else{
		b = cmdj1->WriteMem(rb,MEMP_LOCK,1,0);
	}
	printf("0x%02X %s\n", *rb, b ? "OK" : "FAILED");
}

//-----------------------------------------------------------------------------

void ProgIF_JTAGICEmkI_MEGA_JTAG::ChipErase()
{
	cmdj1->ChipErase();
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkI_MEGA_JTAG::ReadFlashBlock(u8 *p, u32 size, u32 addr)
{
	return cmdj1->ReadMem(p,MEMP_FLASH,size,addr);
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkI_MEGA_JTAG::WriteFlashBlock(u8 *p, u32 size, u32 addr)
{
	return cmdj1->WriteMem(p,MEMP_FLASH,size,addr);
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkI_MEGA_JTAG::ReadEepromBlock(u8 *p, u32 size, u32 addr)
{
	return cmdj1->ReadMem(p,MEMP_EEPROM,size,addr);
}

//---------------------------------------------------------

bool ProgIF_JTAGICEmkI_MEGA_JTAG::WriteEepromBlock(u8 *p, u32 size, u32 addr)
{
	return cmdj1->WriteMem(p,MEMP_EEPROM,size,addr);
}

//-----------------------------------------------------------------------------#####
//-----------------------------------------------------------------------------#####
// Jtag1Commands

bool Jtag1Commands::Connect(const char *c)
{
	if(Commands::Connect(c)) {
		port->SetBaud(19200);
		return true;
	}
	return false;
}

//---------------------------------------------------------

bool Jtag1Commands::SignOn()
{
	StartFrame(&msg);
	strcpy((char*)msg,"        S  "); //10
	msg += strlen((char*)msg);
	TxFrame(&msg);

	u32 s = 0, to = 16;
	while(RxFrame(&msg,1) && to--) {
		switch(msg[0]) {
		case 'A': s = s == 7 ? 8 : 1; to = 8; break;
		case 'V': s = s == 1 ? 2 : 1; to = 8; break;
		case 'R': s = s == 2 ? 3 : 1; to = 4; break;
		case 'N': s = s == 3 ? 4 : 1; to = 4; break;
		case 'O': s = s == 4 ? 5 : 1; to = 4; break;
		case 'C': s = s == 5 ? 6 : 1; to = 4; break;
		case 'D': s = s == 6 ? 7 : 1; to = 4; break;
		default: s = 0; break;
		}
		if(s == 0 ) {
			printf("dumping %02x\n",msg[0]);
		}

		if(s == 8) {
			printf("SignOn: AVRNOCD\n");
			return true;
		}
	}

	printf("Jtag1Commands::SignOn failed\n");
	return false;
}

//---------------------------------------------------------

bool Jtag1Commands::SignOff()
{
	return true;
}

//---------------------------------------------------------

bool Jtag1Commands::GetSync()
{
	StartFrame(&msg);
	*msg++ = CMND1_GET_SYNC;
	TxFrame(&msg);
	return RxFrame(&msg,1) && msg[0] == RSP1_OK;
}

//---------------------------------------------------------

bool Jtag1Commands::Enter()
{
	StartFrame(&msg);
	*msg++ = CMND1_ENTER_PROGMODE;
	return EndFrame2(&msg);
}

//---------------------------------------------------------

bool Jtag1Commands::Leave()
{
	StartFrame(&msg);
	*msg++ = CMND1_LEAVE_PROGMODE;
	return EndFrame2(&msg);
}

//---------------------------------------------------------

bool Jtag1Commands::ChipErase()
{
	StartFrame(&msg);
	*msg++ = CMND1_CHIP_ERASE;
	if(!EndFrame2(&msg)) {
		printf("Jtag1Commands::ChipErase EndFrame FAILED\n");
		return false;
	}
	return true;
/*
	StartFrame(&msg);
	*msg++ = CMND1_CHIP_ERASE;
	return EndFrame2(&msg);*/
}

//---------------------------------------------------------

bool Jtag1Commands::ReadMem(u8* p, u8 type, u32 count, u32 address)
{
	u32 cnt = count;
	if(type == MEMP_FLASH) { //words
		cnt >>= 1;
	}
	StartFrame(&msg);
	*msg++ = CMND1_READ_MEMORY;
	*msg++ = type;
	*msg++ = (u8)(cnt - 1); //0 is 1, 255 is 256 bytes
	*msg++ = (u8)(address >> 16); //MSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >>  0); //LSB
	if(!EndFrame(&msg)) {
		printf("Jtag1Commands::ReadMem FAILED");
		return false;
	}

	if(!RxFrame(&msg,(u16)(count + 2)) || msg[count + 1] != RSP1_OK) {
		printf("Jtag1Commands::ReadMem rx data FAILED");
		return false;
	}

	for(u32 i = 0; i < count; i++) {
		*p++ = msg[i];
	}
	if(count & 1) {
		*p++ = 0xFF;
	}
	return true;
}

//---------------------------------------------------------

bool Jtag1Commands::WriteMem(u8* p, u8 type, u32 count, u32 address)
{
	u32 cnt = count;
	if(type == MEMP_FLASH) { //words
		cnt >>= 1;
		if(count & 1) cnt++; //XXX ?!
	}
	StartFrame(&msg);
	*msg++ = CMND1_WRITE_MEMORY;
	*msg++ = type; 
	*msg++ = (u8)(cnt - 1); //0 is 1
	*msg++ = (u8)(address >> 16); //MSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >>  0); //LSB
	if(!EndFrame(&msg)) {
		printf("Jtag1Commands::WriteMem FAILED");
		return false;
	}

	StartFrame(&msg);
	*msg++ = CMND1_DATA;
	for(u32 i = 0; i < count; i++) {
		*msg++ = *p++;
	}
	if(count & 1) { //XXX ??!!!
		*msg++ = 0xFF;
	}
	TxFrame(&msg);

	return true;
}

//---------------------------------------------------------===

bool Jtag1Commands::GetParameter(u8 p, u8 &v)
{
	StartFrame(&msg);
	*msg++ = CMND1_GET_PARAMETER;
	*msg++ = p;
	if(!EndFrame(&msg)) {
		printf("Jtag1Commands::GetParameter failed\n");
		return false;
	}

	if(!RxFrame(&msg,2) || msg[1] != RSP1_OK) {
		printf("Jtag1Commands::GetParameter get failed\n");
		return false;
	}
	v = *msg;
	return true;
}

//---------------------------------------------------------

bool Jtag1Commands::SetParameter (u8 p, u8 v)
{
	StartFrame(&msg);
	*msg++ = CMND1_SET_PARAMETER;
	*msg++ = p;
	*msg++ = v;
	return EndFrame2(&msg);
}

//-----------------------------------------------------------------------------+++++
//Debug

bool Jtag1Commands::Go()
{
	StartFrame(&msg);
	*msg++ = CMND_GO;
	if(!EndFrame(&msg)) {
		printf("Jtag1Commands::Go failed\n");
		return false;
	}
	return true;
}

//---------------------------------------------------------

u32 Jtag1Commands::Stop()
{
	StartFrame(&msg);
	*msg++ = CMND1_FORCED_STOP;
	if(!EndFrame(&msg)) {
		printf("Jtag1Commands::Stop failed\n");
		return false;
	}
	
	if(!RxFrame(&msg,4) || msg[3] != RSP1_OK) {
		printf("Jtag1Commands::Stop get pc failed\n");
		return 0;
	}
	u32 u;
	u  = msg[1]; u <<= 8;
	u |= msg[2];
	return u;
}

//---------------------------------------------------------

bool Jtag1Commands::Reset()
{
	StartFrame(&msg);
	*msg++ = CMND1_RESET;
	if(!EndFrame2(&msg)) {
		printf("Jtag1Commands::Reset failed\n");
		return false;
	}
	return true;
}

//---------------------------------------------------------

bool Jtag1Commands::Step()
{
	StartFrame(&msg);
	*msg++ = CMND1_SINGLE_STEP;
	if(!EndFrame(&msg)) {
		printf("Jtag1Commands::Step failed\n");
		return false;
	}
	return true;
}

//---------------------------------------------------------

u32 Jtag1Commands::GetPC()
{
	StartFrame(&msg);
	*msg++ = CMND1_READ_PC;
	if(!EndFrame(&msg)) {
		printf("Jtag1Commands::Go failed\n");
		return 0;
	}

	if(!RxFrame(&msg,4) || msg[3] != RSP1_OK) {
		printf("Jtag1Commands::Stop get pc failed\n");
		return false;
	}
	u32 u;
	u  = msg[0]; u <<= 8;
	u |= msg[1]; u <<= 8;
	u |= msg[2];
	return u;
}

//---------------------------------------------------------

bool Jtag1Commands::SetPC(u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND1_WRITE_PC;
	*msg++ = (u8)(address >> 16); //MSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >>  0); //LSB
	if(!EndFrame2(&msg)) {
		printf("Jtag1Commands::SetPC FAILED\n");
		return false;
	}
	return true;
}

//---------------------------------------------------------

bool Jtag1Commands::ErasePageSPM(u32 address)
{
	StartFrame(&msg);
	*msg++ = CMND1_ERASEPAGE_SPM;
	*msg++ = (u8)(address >> 16); //MSB
	*msg++ = (u8)(address >>  8);
	*msg++ = (u8)(address >>  0); //LSB
	if(!EndFrame2(&msg)) {
		printf("Jtag1Commands::ErasePageSPM FAILED\n");
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------

void Jtag1Commands::StartFrame(u8** msg)
{
	*msg = buf;
}

//---------------------------------------------------------

u32 Jtag1Commands::EndFrame(u8** msg)
{
	*(*msg)++ = 0x20; //Sync_CRC
	*(*msg)++ = 0x20; //EOP

	u16 sz = *msg - buf;
	port->Write(buf,sz);
	if(!RxFrame(msg,1) || **msg != RSP1_OK) {
		printf("Jtag1Commands::EndFrame sync error\n");
		return 0;
	}
	return 1;
}

//---------------------------------------------------------

bool Jtag1Commands::EndFrame2(u8** msg)
{
	if(!EndFrame(msg)) {
		return false;
	}
	if(!RxFrame(msg,1) || **msg != RSP1_OK) {
		return false;
	}
	return true;
}

//---------------------------------------------------------

u32 Jtag1Commands::TxFrame(u8** msg)
{
	u16 sz = *msg - buf;
	return port->Write(buf,sz);
}

//---------------------------------------------------------

bool Jtag1Commands::RxFrame(u8** msg, u16 u)
{
	*msg = buf;
	return port->Read(*msg,u) == u;
}

//-----------------------------------------------------------------------------#####

/*unsigned char desc_m162[126] = {
0xF7,0x6F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xF3,0x66,0xFF,0xFF,0xFF,0xFF,0xFF,0xFA,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x18,0x00,0x30,0xF3,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x18,0x00,0x20,0xF3,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x04, 0x57, 0x00,   0x80,0,   0x04,     0x80,0x1F,0x00,0x00,     0x8B };*/

typedef struct
{
	u8 ucRead[8]; //bit0 byte0 == IO 0x00 --- bit7 byte7 == IO 0x3F
	u8 ucReadShadow[8];
	u8 ucWrite[8];
	u8 ucWriteShadow[8];
	u8 ucExtRead[20];
	u8 ucExtReadShadow[20];
	u8 ucExtWrite[20];
	u8 ucExtWriteShadow[20];

	u8 ucIDRAddress;   //IO
	u8 ucSPMCRAddress; //RAM
	u8 ucRAMPZAddress; //IO

	u8 uiFlashPageSize[2]; //bytes, LE
	u8 ucEepromPageSize;   //bytes

	u8 ulBootAddress[4]; //smallest, word, LE
	u8 uiUpperExtIOLoc;  //RAM
} jtag1_dd;

#include "Rtk/RConfig.h"

bool Jtag1Commands::SetDescriptor(Device *dev)
{
	u32 i;
	jtag1_dd dd;
	u8 *ddp = (u8*)&dd;

	dev->Load();//XXX debug

	RConfigNode *t,*cfg = dev->config;

	for(i = 0; i < sizeof(dd); i++) {
		*ddp++ = 0;
	}

	t = cfg->GetNode("OCD");
	t->GetBin("ucRead", dd.ucRead, 8);
	t->GetBin("ucWrite", dd.ucWrite, 8);
	t->GetBin("ucExtRead", dd.ucExtRead, 20);
	t->GetBin("ucExtWrite", dd.ucExtWrite, 20);

	dd.ucIDRAddress    = (u8)t->GetValue("OCDR");
	dd.uiUpperExtIOLoc = (u8)t->GetValue("ucUpperExtIOLoc");

	t = cfg->GetNode("IO_PORTS");
	dd.ucRAMPZAddress = (u8)t->GetValue("RAMPZ");
	if(dd.ucRAMPZAddress) {
		dd.ucRAMPZAddress -= 0x20; //convert to IO
	}
	if(!(dd.ucSPMCRAddress = (u8)t->GetValue("SPMCSR"))) {
		 dd.ucSPMCRAddress = (u8)t->GetValue("SPMCR"); // a few old parts m32 (m8 m8515 m161 m163)
	}

	t = cfg->GetNode("MEMORY");
	i = t->GetValue2("FLASH"); // == cfg->GetValue2("MEMORY/FLASH");
	dd.uiFlashPageSize[0] = (u8)(i >> 0);
	dd.uiFlashPageSize[1] = (u8)(i >> 8);
	dd.ucEepromPageSize = (u8)t->GetValue2("EEPROM");

	i = cfg->GetValue("BOOT/BOOTSIZES");
	dd.ulBootAddress[0] = (u8)(i >>  0);
	dd.ulBootAddress[1] = (u8)(i >>  8);
	dd.ulBootAddress[2] = (u8)(i >> 16);
	dd.ulBootAddress[3] = (u8)(i >> 24);

//-------------------------------------

	u8 *p = (u8*)&dd; //desc_m162;
	printf("Sizeof jtag1_dd 126 = %i\n",(int)sizeof(dd));
	StartFrame(&msg);
	*msg++ = CMND1_SET_DEVICE_DESCRIPTOR;
	for(i = 0; i < sizeof(dd); i++) { //data
		*msg++ = *p++;
	}
	if(!EndFrame2(&msg)) {
		printf("Jtag1Commands::SetDescriptor failed\n");
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------#####



/*
bool ProgJTAGICEmkI::SignOn()
{
//-------------------------------------
//nasty bit of sync code

	uint l,a,u,r;
	StartFrame(&msg);
	strcpy((char*)msg,"          "); //10
	l = strlen((char*)msg);
	msg += l;
	TxFrame(&msg);
	if(!RxFrame(&msg,l)) {
		printf("SignOn failed to read sync %u -> %u\n",l,r);
	}
next:
	for(a = 0, u = 0; u < l; u++) {
		if(msg[u] != RSP1_OK) {
			printf("dumping %02x\n",msg[u]);
			a++;
		}else{
//			printf("RSP1_OK\n");
		}
	}
	if(a) { //dumped some ?
		l = a;
		if(!RxFrame(&msg,l)) {
			printf("SignOn failed to read sync2 %u -> %u\n",l,r);
		}
		goto next;	
	}

//nasty bit of sync code
//-------------------------------------

	StartFrame(&msg);
	*msg++ = CMND1_GET_SIGN_ON;
	if(EndFrame(&msg) != 1 || *msg != RSP1_OK) {
		return false;
	}
	if(!RxFrame(&msg,8) || msg[7] != RSP1_OK || strncmp((char*)msg,"AVRNOCD",7)) {
		return false;
	}
	msg[7] = 0;
	printf("SignOn: %s\n",(char*)msg);

	return true;
}*/


