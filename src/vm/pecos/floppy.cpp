/*
	ASIEL PECOS Emulator 'ePECOS'

	Author: cdoty
	Date   : 2026.06.05-

	[ floppy ]
*/

#include "floppy.h"
#include "memory.h"

void FLOPPY::initialize()
{
	statusRegister	= 0;
	trackRegister	= 0;
	sectorRegister	= 0;
	dataRegister	= 0;
	stepIn			= false;
	readingSectors	= false;
	seekNextSector	= false;
	floppyLoaded	= false;
	motorRunning	= false;

	dataOffset	= 0;
	readCount	= 0;

	// load floppy disk
	FILEIO* fio = new FILEIO();

	if(fio->Fopen(create_local_path(_T("FloppyDisk.img")), FILEIO_READ_BINARY))
	{
		long	fileLength	= fio->FileLength();

		fio->Fread(floppyDisk, fileLength, 1);
		fio->Fclose();

		floppyLoaded	= true;
	}

	else
	{
		floppyLoaded	= false;
	}

	delete	fio;
}

void FLOPPY::event_callback(int event_id, int err)
{
	if (true == readingSectors)
	{
		if (dataOffset < sizeof(floppyDisk) && readCount > 0)
		{
			write_signals(&outputs_nmi, 0xffffffff);
		}

		else
		{
			endDiskReadEvent();

			write_signals(&outputs_irq, 0xffffffff);
		}
	}
}

#define STATE_VERSION	2

bool FLOPPY::process_state(FILEIO* state_fio, bool loading)
{
	return true;
}

void FLOPPY::executeCommand(uint32_t data)
{
	uint32_t	command	= data >> 5;

#ifdef _FLOPPY_DEBUG_LOG
	char	szCommand[256]	= {0};
#endif

	switch (command)
	{
		case 0:
			if (0 == (data & 0x10))
			{
				// Seek track 0
#ifdef _FLOPPY_DEBUG_LOG
				sprintf(szCommand, "Seek track 0\n");
#endif
				if (true == floppyLoaded)
				{
					dataOffset		= 0;
					statusRegister	= 0x04;

					stepIn			= false;
					sectorRegister	= 0;
					trackRegister	= 0;
				}

				else
				{
					statusRegister	= 0x00;
				}

				write_signals(&outputs_irq, 0xffffffff);
			}

			else
			{
				int	track	= data & 0x1F;

				// Seek track in data register
#ifdef _FLOPPY_DEBUG_LOG
				sprintf(szCommand, "Seek track %d\n", track);
#endif
				if (true == floppyLoaded && track >= 0 && track < MAX_DISK_TRACKS)
				{
					statusRegister	= 0x00;
					trackRegister	= track;
				}

				else
				{
					statusRegister	= 0x81;
				}

				write_signals(&outputs_irq, 0xffffffff);			
			}

			updateDataLocation();

			break;
	
		case 1:
			// Step track in previously set direction
#ifdef _FLOPPY_DEBUG_LOG
			sprintf(szCommand, "Step track\n");
#endif
			if (true == floppyLoaded)
			{
				if (false == stepIn)
				{
					if (trackRegister > 0)
					{
						trackRegister--;

						statusRegister	= 0x00;
					}

					else
					{
						statusRegister	= 0x81;
					}

				}

				else
				{
					if (trackRegister < MAX_DISK_TRACKS - 1)
					{
						trackRegister--;

						statusRegister	= 0x00;
					}

					else
					{
						statusRegister	= 0x81;
					}
				}

				updateDataLocation();
			}
			
			else
			{
				statusRegister	= 0x81;
			}

			write_signals(&outputs_irq, 0xffffffff);
			
			break;

		case 2:
#ifdef _FLOPPY_DEBUG_LOG
			sprintf(szCommand, "Step-in\n");
#endif
			stepIn	= true;

			write_signals(&outputs_irq, 0xffffffff);
			
			break;

		case 3:
#ifdef _FLOPPY_DEBUG_LOG
			sprintf(szCommand, "Step-out\n");
#endif
			stepIn	= false;

			write_signals(&outputs_irq, 0xffffffff);
			
			break;

		case 4:
		{
#ifdef _FLOPPY_DEBUG_LOG
			sprintf(szCommand, "Read sector %d\n", sectorRegister);
#endif
			seekNextSector	= (data & 0x10) != 0;

			if (true == floppyLoaded)
			{
				readCount		= DISK_SECTOR_SIZE;
				statusRegister	= 0x00;

				startDiskReadEvent();
			}

			else
			{
				statusRegister	= 0x81;

				write_signals(&outputs_irq, 0xffffffff);
			}

			break;
		}

		case 5:
#ifdef _FLOPPY_DEBUG_LOG
			sprintf(szCommand, "Write sector %d\n", sectorRegister);
#endif
			seekNextSector	= (data & 0x10) != 0;

			break;

		case 6:
			if (0 == (data & 0x10))
			{
				// Read address of next sector ID.
#ifdef _FLOPPY_DEBUG_LOG
				sprintf(szCommand, "Read address %d\n", dataRegister);
#endif		
			}

			else
			{
				// Force interrupt. Stop all actions.
#ifdef _FLOPPY_DEBUG_LOG
				sprintf(szCommand, "Force interrupt\n");
#endif
				statusRegister	= 0x00;
			}

			break;

		case 7:
			if (0 == (data & 0x10))
			{
#ifdef _FLOPPY_DEBUG_LOG
				sprintf(szCommand, "Read track %d\n", trackRegister);
#endif
				readCount		= MAX_TRACK_SECTORS * DISK_SECTOR_SIZE;
				statusRegister	= 0x00;

				startDiskReadEvent();
			}

			else
			{
#ifdef _FLOPPY_DEBUG_LOG
				sprintf(szCommand, "Write track %d\n", trackRegister);
#endif
			}

			break;
	}

#ifdef _FLOPPY_DEBUG_LOG
	if (strlen(szCommand) > 0)
	{
		out_debug_log(szCommand);
	}
#endif
}

void FLOPPY::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	uint8_t	port	= addr & 0xFF;

	if (0xd0 == port)
	{
		executeCommand(data);
	}

	else if (0xd1 == port)
	{
		trackRegister	= data;

		if (false == floppyLoaded)
		{
			statusRegister	= 0x81;
		}
	
		else
		{
			updateDataLocation();
		}
	}

	else if (0xd2 == port)
	{
		sectorRegister	= data;

		if (false == floppyLoaded)
		{
			statusRegister	= 0x81;
		}
	
		else
		{
			updateDataLocation();
		}
	}

	else if (0xd3 == port)
	{
		dataRegister	= data;

		if (false == floppyLoaded)
		{
			statusRegister	= 0x81;
		}
	}

	else if (0xd4 == port)
	{
		motorRunning	= data & 0x08;	// Motor control? Unsure of actual bit or port. Bit 0 might be reset.

		if (false == floppyLoaded)
		{
			statusRegister	= 0x81;
		}
	}

	else
	{
#ifdef _FLOPPY_DEBUG_LOG
		char	szMessage[256];

		sprintf(szMessage, "Writting unknown floppy port %02x:%02x\n", port, data);

		out_debug_log(szMessage);
#endif
	}
}

uint32_t FLOPPY::read_io8w(uint32_t addr, int* wait)
{
	uint8_t	port	= addr & 0xFF;

	if (0xd0 == port)
	{
		return	statusRegister;
	}

	else if (0xd1 == port)
	{
		return	trackRegister;
	}

	else if (0xd2 == port)
	{
		return	sectorRegister;
	}

	else if (0xd3 == port)
	{
		if (true == floppyLoaded)
		{
			if (dataOffset < sizeof(floppyDisk) && readCount > 0)
			{
				uint32_t	value	= floppyDisk[dataOffset];

				dataOffset++;
				readCount--;

				statusRegister	= 0;

				if ((DISK_SECTOR_SIZE - 1) == dataOffset % DISK_SECTOR_SIZE && true == seekNextSector)
				{
					if ((sectorRegister % MAX_TRACK_SECTORS) < MAX_TRACK_SECTORS - 1)
					{
						sectorRegister++;

						updateDataLocation();
					}
				}

				return	value;
			}
		}

		else
		{
			statusRegister	= 0x81;

			write_signals(&outputs_irq, 0xffffffff);			
		}

		return	0xFF;
	}

	else if (0xd4 == port)
	{
		return	motorRunning;
	}

	else
	{
#ifdef _FLOPPY_DEBUG_LOG
		char	szMessage[256];

		sprintf(szMessage, "Reading unknown floppy port %02x\n", port);

		out_debug_log(szMessage);
#endif
	}

	return	0xFF;
}

void FLOPPY::startDiskReadEvent()
{
	readingSectors	= true;

	// Register event to transfer a byte from the floppy
	register_event(this, EVENT_READ_BYTE, 1000000.0 / DISK_TRANSFER_RATE, true, &readEventID);
}

void FLOPPY::endDiskReadEvent()
{
	readingSectors	= false;

	cancel_event(this, readEventID);
}

void FLOPPY::updateDataLocation()
{
	dataOffset	= trackRegister * (MAX_TRACK_SECTORS * DISK_SECTOR_SIZE) + sectorRegister * DISK_SECTOR_SIZE;
}