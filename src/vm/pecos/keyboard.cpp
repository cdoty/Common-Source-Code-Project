/*
	ASIEL PECOS Emulator 'ePECOS'

	Author: cdoty
	Date   : 2026.06.05-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

// Table taken from 0xfb73 in memory.
static const uint8_t key_map[8][8] =
{
	{ 0x51, 0x45, 0x57, 0x52, 0x55, 0x54, 0x49, 0x59 },
	{ 0x00, 0x00, 0x00, 0x00, 0x7C, 0x20, 0x0D, 0x00 },
	{ 0x09, 0x31, 0x1B, 0x32, 0x35, 0x33, 0x36, 0x34 },
	{ 0x41, 0x44, 0x53, 0x46, 0x00, 0x47, 0x4A, 0x48 },
	{ 0x16, 0x1E, 0x0B, 0x5A, 0x56, 0x58, 0x42, 0x43 },
	{ 0x4E, 0x2C, 0x4D, 0x2E, 0x08, 0x2F, 0x0C, 0x5C },
	{ 0x37, 0x39, 0x38, 0x30, 0x08, 0x2D, 0x40, 0x5E },
	{ 0x4B, 0x3B, 0x4C, 0x3A, 0x5B, 0x4F, 0x5D, 0x50 }
};

void KEYBOARD::initialize()
{
	key_stat = emu->get_key_buffer();
	
	// register event to update the key status
	register_frame_event(this);
}

void KEYBOARD::event_frame()
{
}

uint32_t KEYBOARD::get_row(int row)
{
	uint32_t data = 0;
	
	if(row >= 0 && row < 8)
	{
		// keyboard
		for(int column = 0; column < 8; column++) {
			if(key_stat[key_map[column][row]]) {
				data |= (1 << column);
			}
		}
	}

	return	(~data);
}

#define STATE_VERSION	1

bool KEYBOARD::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	return true;
}

uint32_t KEYBOARD::read_io8w(uint32_t addr, int* wait)
{
	uint32_t	row		= (addr & 0xFF00) >> 8;
	uint32_t	data	= (get_row(row)) & 0xFF;

	return	data;
}
