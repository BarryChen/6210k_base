#ifndef __STORAGE_IF_H__
#define __STORAGE_IF_H__

// Read data blocks from storage device, it is the interface 
// between file system and storage device.
// Example: You want to read 4 blocks from block 100 of device 1, 
// and data buffer start address is 0xC000, call this funciton as follow
// "ReadBlock(1, 100, (BYTE XDATA *)0xC000, 4);" After this funciton, 
// 4 blocks data have been loaded into the buffer, which start address is 0xC000.
BOOL ReadBlock(DWORD Block, WORD Buffer, BYTE Size);



#endif
