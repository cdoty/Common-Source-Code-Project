/*
	ASIEL PECOS Emulator 'ePECOS'

	Author: cdoty
	Date   : 2026.06.05-

	[ keyboard ]
*/

#include "keyboard.h"
#include "../i8255.h"

// Matching layout replaces the backslash and pipe with grave accent and @, matching the layout of the original key. The missing backslash key is moved to the tilde key.
//#define MATCHING_LAYOUT

// In matching mode, remap right shift key to backslash.
//#define	REMAP_RSHIFT

// Table taken from 0xfb73 in memory.
static const uint8_t key_map[8][8] =
{
	{ 0x51, 0x45, 0x57, 0x52, 0x55, 0x54, 0x49, 0x59 },
	{ 0x00, 0x00, 0x00, 0x00, 0x7C, 0x20, 0x0D, 0x00 },
	{ 0x09, 0x31, 0x1B, 0x32, 0x35, 0x33, 0x36, 0x34 },
	{ 0x41, 0x44, 0x53, 0x46, 0x00, 0x47, 0x4A, 0x48 },
	{ 0x16, 0x1E, 0x0B, 0x5A, 0x56, 0x58, 0x42, 0x43 },
#ifdef MATCHING_LAYOUT
	{ 0x4E, 0xBC, 0x4D, 0xBE, 0x08, 0xBF, 0x0C, 0xDC },	// Row 7 is backslash (5C).
	{ 0x37, 0x39, 0x38, 0x30, 0x08, 0xBD, 0x00, 0xDE },	// Row 6 is @ (40). Code needs to handle different keys for backslash.
#else
	{ 0x4E, 0xBC, 0x4D, 0xBE, 0x08, 0xBF, 0x0C, 0x00 },	// Row 7 is backslash (5C). Code needs to handle different keys for backslash.
	{ 0x37, 0x39, 0x38, 0x30, 0x08, 0xBD, 0xDC, 0xDE },	// Row 6 is @ (40).
#endif
	{ 0x4B, 0xBB, 0x4C, 0xBA, 0xDB, 0x4F, 0xDD, 0x50 }
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

			// Handle key modifiers
			if (1 == column)
			{
#if defined MATCHING_LAYOUT && defined REMAP_RSHIFT
				// The rows are processed in the order listed.
				if (3 == row && key_stat[VK_LSHIFT])
				{
					data	|= 1 << column;
				}
#else
				// The rows are processed in the order listed.
				if (3 == row && key_stat[VK_SHIFT])
				{
					data	|= 1 << column;
				}
#endif
				// Simulate caps lock with scroll lock
				else if (4 == row && key_stat[VK_SCROLL])
				{
					data	|= 1 << column;
				}

				else if (1 == row && (key_stat[VK_CONTROL]))
				{
					data	|= 1 << column;
				}

				// Unsure of the use of rows 0, 2, or 7. They are not handled in the ROM and not handled in the character table at 0xFB73.
				else if (0 == row && key_stat[VK_F1])
				{
					data	|= 1 << column;
				}

				else if (2 == row && key_stat[VK_F2])
				{
					data	|= 1 << column;
				}

				else if (7 == row && key_stat[VK_F3])
				{
					data	|= 1 << column;
				}
			}

			else if (3 == column)
			{
				// Unsure of the use of row 4.  They are not handled in the ROM and not handled in the character table at 0xFB73.
				if (4 == row && key_stat[VK_F4])
				{
					data	|= 1 << column;
				}
			}

#ifdef MATCHING_LAYOUT
			else if (5 == column)
			{
#ifdef REMAP_RSHIFT
				// Treat right shift as backslash
				if (7 == row && key_stat[VK_RSHIFT])
				{
					data	|= 1 << column;
				}
#endif
			}

			else if (6 == column)
			{
				// Treat both backslash keys as @ `
				if (6 == row && (key_stat[VK_OEM_5] || key_stat[VK_OEM_102]))
				{
					data	|= 1 << column;
				}
			}
#else
			else if (5 == column)
			{
				// Treat both backslash keys as backslash
				if (7 == row && (key_stat[VK_OEM_5] || key_stat[VK_OEM_102]))
				{
					data	|= 1 << column;
				}
			}
#endif
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
