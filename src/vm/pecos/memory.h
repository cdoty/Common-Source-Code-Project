/*
	ASIEL PECOS Emulator 'ePECOS'

	Author: cdoty
	Date   : 2026.06.05-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define	MODULE_HEADER_SIZE	8	// It is 9 for the non patched bios, but the bios only uses 8 bytes.

class MEMORY : public DEVICE
{
private:	
	// memory
	uint8_t module[0x10000 + MODULE_HEADER_SIZE];	// Module file
	uint8_t ipl[0x1000];			// PECOS bios
	uint8_t ram[0x10000];
	
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	uint8_t* wbank[16];
	uint8_t* rbank[16];
	
	bool inserted;
	bool ram_selected;
	int module_byte_index;
	uint8_t bank[3];
	
	void update_bank();
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void open_module(const _TCHAR* file_path);
	void close_module();
	bool is_module_inserted()
	{
		return inserted;
	}

	void write_io8w(uint32_t addr, uint32_t data, int* wait) override;
	uint32_t read_io8w(uint32_t addr, int* wait) override;
};

#endif

