/*
	ASIEL PECOS Emulator 'ePECOS'

	Author: cdoty
	Date   : 2026.06.05-

	[ memory ]
*/

#include "memory.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	ram_selected = false;
	module_byte_index	= 0;

	memset(module, 0xff, sizeof(module));
	memset(ipl, 0xff, sizeof(ipl));
	memset(ram, 0, sizeof(ram));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load ipl
	FILEIO* fio = new FILEIO();

	if(fio->Fopen(create_local_path(_T("PECOS_128A_V400.rom")), FILEIO_READ_BINARY) || 
	   fio->Fopen(create_local_path(_T("PECOS_V2.00_PRB0127.rom")), FILEIO_READ_BINARY) || 
	   fio->Fopen(create_local_path(_T("PECOS_BIOS111.rom")), FILEIO_READ_BINARY))
	{
		long	fileLength	= fio->FileLength();

		fio->Fread(ipl, fileLength, 1);
		fio->Fclose();

		if (0x8000 == fileLength)
		{
			emu->setIsPrick(true);
		}
	}
	delete fio;
	
	// set memory map
	close_module();
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	if(addr >= 0xfffd) {
		if(bank[addr - 0xfffd] != 0xff) {
			bank[addr - 0xfffd] = data;
			update_bank();
		}
	}
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::open_module(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(module, 0xff, sizeof(module));
		fio->Fread(module, sizeof(module), 1);
		if(fio->Ftell() > 0x8000 + MODULE_HEADER_SIZE) {
			// ホーム麻雀(32KB+16KB) or ロレッタの肖像(128KB)
			bank[0] = 0;
			bank[1] = 1;
			bank[2] = 2;
		} else {
			bank[0] = bank[1] = bank[2] = 0xff;
		}
		fio->Fclose();
		inserted = true;
		
		// set memory map
		update_bank();
	}
	delete fio;

	emu->reset();
}

void MEMORY::close_module()
{
	memset(module, 0xff, sizeof(module));
	inserted = false;
	bank[0] = bank[1] = bank[2] = 0xff;
	
	// set memory map
	update_bank();
}

void MEMORY::update_bank()
{
	if(!inserted) {
		if (true == emu->isPrick())
		{
			SET_BANK(0x0000, 0x7fff, ram + 0x0000, ipl);
			SET_BANK(0x8000, 0xffff, ram + 0x8000, ram + 0x8000);
		}

		else
		{
			SET_BANK(0x0000, 0x1fff, ram + 0x0000, ipl);
			SET_BANK(0x2000, 0xffff, ram + 0x2000, ram + 0x2000);
		}
	}
	if(ram_selected) {
		if (true == emu->isPrick())
		{
			SET_BANK(0x0000, 0x7fff, ram, ram);
		}

		else
		{
			SET_BANK(0x0000, 0x1fff, ram, ram);
		}
	}
}

#define STATE_VERSION	2

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(inserted);
	state_fio->StateValue(ram_selected);
	state_fio->StateArray(bank, sizeof(bank), 1);
	
	// post process
	if(loading) {
		update_bank();
	}
	return true;
}

void MEMORY::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	if (0x40 == (addr & 0xFF))
	{
		ram_selected = (data != 0);

		update_bank();
	}

	else if (0x80 == (addr & 0xFF))
	{
		// Start module loader
		if (true == inserted)
		{
			module_byte_index	= 0;
		}
	}
}

uint32_t MEMORY::read_io8w(uint32_t addr, int* wait)
{
	if (0x80 == (addr & 0xFF) && true == inserted)
	{
		// Load module byte
		if (module_byte_index < sizeof(module))
		{
			uint32_t	value	= module[module_byte_index];
			
			module_byte_index++;

			return	value;
		}
	}

	return	0xFF;
}
