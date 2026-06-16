/*
	ASIEL PECOS Emulator 'ePECOS'

	Author: cdoty
	Date   : 2026.06.05-

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define	MAX_TRACK_SECTORS	9
#define	MAX_DISK_TRACKS		80
#define	DISK_SECTOR_SIZE	512
#define	EVENT_READ_BYTE		0
#define	DISK_TRANSFER_RATE	(250000 / 8)	// Byte transfer rate to match 250kbps transfer rate

class MEMORY;

class FLOPPY : public DEVICE
{
private:	
	uint8_t	statusRegister;
	uint8_t	trackRegister;
	uint8_t	sectorRegister;
	uint8_t	dataRegister;
	bool	stepIn;
	bool	readingSectors;
	bool	floppyLoaded;
	bool	motorRunning;

	int	dataOffset;
	int	readCount;
	int readEventID;

	uint8_t	floppyDisk[MAX_DISK_TRACKS * MAX_TRACK_SECTORS * DISK_SECTOR_SIZE];

	outputs_t outputs_irq;
	outputs_t outputs_nmi;

public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy Drive"));

		initialize_output_signals(&outputs_irq);
		initialize_output_signals(&outputs_nmi);
	}
	~FLOPPY() {}
	
	// common functions
	void initialize();
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);

	void executeCommand(uint32_t data);

	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}

	void set_context_nmi(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_nmi, device, id, mask);
	}

	void write_io8w(uint32_t addr, uint32_t data, int* wait) override;
	uint32_t read_io8w(uint32_t addr, int* wait) override;

	void startDiskReadEvent();
	void endDiskReadEvent();
	void updateDataLocation();
};

#endif

