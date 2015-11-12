#ifndef __NVM_H__
#define	__NVM_H__


#define MAX_NVM_LEN		150

// Write data into NVM
BOOL NvmWriteOneByte(BYTE NvmAddr, BYTE Dat);

// Read data from NVM
BYTE NvmReadOneByte(BYTE NvmAddr);

// Write Multiply-Bytes.
BOOL NvmWriteMulti(BYTE Addr, BYTE* Buff, BYTE Len);

// Read Multiply-Bytes.
BOOL NvmReadMulti(BYTE Addr, BYTE* Buff, BYTE Len);

#endif