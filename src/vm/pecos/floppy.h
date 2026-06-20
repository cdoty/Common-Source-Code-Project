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
#include "../noise.h"

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
	bool	motorRunning;
	bool	diskInserted;
	bool	diskProtected;

	int	dataOffset;
	int	readCount;
	int readEventID;

	uint8_t*	floppyDisk;
	int			floppySize;

	outputs_t outputs_irq;
	outputs_t outputs_nmi;

	// drive noise
	NOISE* d_noise_seek;
	NOISE* d_noise_head_down;
	NOISE* d_noise_head_up;

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

	void set_context_noise_seek(NOISE* device)
	{
		d_noise_seek = device;
	}
	NOISE* get_context_noise_seek()
	{
		return d_noise_seek;
	}
	void set_context_noise_head_down(NOISE* device)
	{
		d_noise_head_down = device;
	}
	NOISE* get_context_noise_head_down()
	{
		return d_noise_head_down;
	}
	void set_context_noise_head_up(NOISE* device)
	{
		d_noise_head_up = device;
	}
	NOISE* get_context_noise_head_up()
	{
		return d_noise_head_up;
	}

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

	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool is_disk_inserted(int drv);
	bool is_disk_inserted();	// current hdu
	void is_disk_protected(int drv, bool value);
	bool is_disk_protected(int drv);

	void write_io8w(uint32_t addr, uint32_t data, int* wait) override;
	uint32_t read_io8w(uint32_t addr, int* wait) override;

	void startDiskReadEvent();
	void endDiskReadEvent();
	uint8_t setTrackRegister(int track);
	uint8_t setSectorRegister(int sector);
	void updateDataLocation();
};

#endif

