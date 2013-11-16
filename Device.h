// vim:ts=4 sts=0 sw=4

class Device;
class AvrFuse
{
public:
	AvrFuse() { next = 0; child = 0; address = mask = value = 0; };
	AvrFuse(Device *,AvrFuse *,const char *, u32);
	~AvrFuse() {};

	void Set(bool);
	bool Get();
	void Select(int);
	int GetSelect();

	AvrFuse *GetAt(int);
	AvrFuse *next;
	AvrFuse *child;
	char name[100];

	Device *dev;
	u8 address,mask,value;
	u8 def; //default
};

class RConfigNode;
class Device
{
public:
	Device();
	~Device();

	Device *GetAt(int);

	void Load();

	AvrFuse *fuses;
	AvrFuse *GetFuses();
	void SetDefaultFuses();

	void GetFuseBits(u32 a, AvrFuse *r, RConfigNode *t, const char *nm);

	u8 GetFuseBuf(u8 b[32]);
	void SetFuseBuf(u8 b[32]);

	RConfigNode *config;

	char name[20];
	u32 if_flags;
	u32 signature;
	u32 jtag_id;
	Device *next;  //ProgIF list
	Device *nextl; //mem management
	int i; //index

	u8 fcnt;
	u8 fusebuf[32];
	u8 maskbuf[32];

};

//DevIf flags

#define DIF_ISP  0x0001 //ISP
#define DIF_HVPP 0x0002 //HVPP
#define DIF_HVSP 0x0004 //HVSP
#define DIF_DW   0x0008 //debugWire
#define DIF_MJ   0x0010 //Mega Jtag
#define DIF_TPI  0x0020 //TPI
#define DIF_PDI  0x0040 //PDI
#define DIF_XJ   0x0080 //Xmega Jtag
#define DIF_AJ   0x0100 //AVR32 Jtag
#define DIF_AW   0x0200 //aWire

#define DIF_MMASK 0x001F //mega/tiny
#define DIF_XMASK 0x00E0 //xmega
#define DIF_AMASK 0x0300 //avr32

#define DIF_JMASK 0x0198 //used for jtagid device detection

#define DIF_BOOT  0x8000 //Bootloader bit

Device *GetAvrList();




