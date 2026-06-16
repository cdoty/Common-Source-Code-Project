ASIEL PECOS Emulator

Supports bios file PECOS_BIOS111.rom (v1.11) or PECOS_V2.00_PRB0127.rom (v2.00) for model PECOS, or PECOS_128A_V400.rom (v4.00) for model PRICK.

Preliminary disk drive support has been implemented. It is able to load the boot sector, and might be able to load a command processor to $EA00.
It will load the boot sector, from FloppyImage.img, on each model. The PECOS 128k doesn't have the BDOS functions at F400, so it is unable to print the boot sector message.