#ifndef __OTG_HAL_H__
#define	__OTG_HAL_H__


// otg common register
#define	otg_CommonUSB_FAddr				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x00))
#define	otg_CommonUSB_Power				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x01))
#define	otg_CommonUSB_IntrTx			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x02))
#define	otg_CommonUSB_IntrRx			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x04))
#define	otg_CommonUSB_IntrTxEn			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x06))
#define	otg_CommonUSB_IntrRxEn			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x08))
#define	otg_CommonUSB_IntrUSB			((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x0A))
#define	otg_CommonUSB_IntrUSBE			((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x0B))
#define	otg_CommonUSB_Frame				((WORD XDATA*)(SYS_OTG_REG_BASE + 0x0C))
#define	otg_CommonUSB_Index				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x0E))
#define	otg_CommonUSB_TestMode			((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x0F))

// otg indexed register
#define	otg_IndexedCSR_TxMaxP			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x10))
#define	otg_IndexedCSR_CSR0				((WORD XDATA*)(SYS_OTG_REG_BASE + 0x12))
#define	otg_IndexedCSR_TxCSR			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x12))
#define	otg_IndexedCSR_RxMaxP			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x14))
#define	otg_IndexedCSR_RxCSR			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x16))
#define	otg_IndexedCSR_Count0			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x18))
#define	otg_IndexedCSR_RxCount			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x18))
#define	otg_IndexedCSR_host_TxType		((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x1A))
#define	otg_IndexedCSR_host_NAKLimit0	((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x1B))
#define	otg_IndexedCSR_host_TxInterval	((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x1B))
#define	otg_IndexedCSR_host_RxType		((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x1C))
#define	otg_IndexedCSR_host_RxInterval	((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x1D))
#define	otg_IndexedCSR_dev_ConfigData	((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x1F))
#define	otg_IndexedCSR_dev_FIFOSize		((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x1F))

// otg endpoint fifo register
#define	otg_EndpointFIFO				((DWORD XDATA*)(SYS_OTG_REG_BASE + 0x20))
#define	otg_CtrlFIFO_DevCtl				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x60))
#define	otg_CtrlFIFO_TxFIFOSz			((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x62))
#define	otg_CtrlFIFO_RxFIFOSz			((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x63))
#define	otg_CtrlFIFO_TxFIFOadd			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x64))
#define	otg_CtrlFIFO_RxFIFOadd			((WORD XDATA*)(SYS_OTG_REG_BASE + 0x66))
#define	otg_CtrlFIFO_VControlStatus		((DWORD XDATA*)(SYS_OTG_REG_BASE + 0x68))
#define	otg_CtrlFIFO_HWVers				((WORD XDATA*)(SYS_OTG_REG_BASE + 0x6C))

// otg ULPI register
#define	otg_ULPI_CarKitCtrl				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x70))
#define	otg_ULPI_VBusCtrl				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x71))
#define	otg_ULPI_IntSrc					((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x72))
#define	otg_ULPI_IntMask				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x73))
#define	otg_ULPI_RegAddr				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x74))
#define	otg_ULPI_RegData				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x75))
#define	otg_ULPI_RawData				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x76))
#define	otg_ULPI_RegCtrl				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x77))

// otg config register
#define	otg_Config_EPInfo				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x78))
#define	otg_Config_RAMInfo				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x79))
#define	otg_Config_LinkInfo				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x7A))
#define	otg_Config_VPLen				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x7B))
#define	otg_Config_HS_EOF				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x7C))
#define	otg_Config_FS_EOF				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x7D))
#define	otg_Config_LS_EOF				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x7E))

// otg DMA control reisger, only use channel 1 register
#define	otg_DMA_Intr					((DWORD XDATA*)(SYS_OTG_REG_BASE + 0x200))
#define	otg_DMA_Ctrl					((DWORD XDATA*)(SYS_OTG_REG_BASE + 0x204))
#define	otg_DMA_Addr					((DWORD XDATA*)(SYS_OTG_REG_BASE + 0x208))
#define	otg_DMA_Count					((DWORD XDATA*)(SYS_OTG_REG_BASE + 0x20C))

#define	otg_RqPktCount					((DWORD XDATA*)(SYS_OTG_REG_BASE + 0x300))

#define	otg_PhyLineStatus				((BYTE XDATA*)(SYS_OTG_REG_BASE + 0x328))


// otg_CommonUSB_Power
#define	Power_prw_IsoUpdate				(1 << 7)			
#define	Power_prw_SoftConn				(1 << 6)				
#define	Power_prw_HSEnab				(1 << 5)		
#define	Power_pr_HSMode					(1 << 4)	
#define	Power_pr_Reset					(1 << 3)	
#define	Power_prw_Resume				(1 << 2)	
#define	Power_pr_SuspendMode			(1 << 1)	
#define	Power_prw_EnSuspendMode			(1 << 0)
	
#define	Power_hrw_HSEnab				(1 << 5)
#define	Power_hr_HSMode					(1 << 4)		
#define	Power_hrw_Reset					(1 << 3)	
#define	Power_hrw_Resume				(1 << 2)
#define	Power_hs_SuspendMode			(1 << 1)			
#define	Power_hrw_EnSuspendMode			(1 << 0)		
	

// otg_CommonUSB_IntrUSB
#define	IntUSB_VBusErr					(1 << 7)
#define	IntUSB_SessReq					(1 << 6)
#define	IntUSB_DisCon					(1 << 5)
#define	IntUSB_Conn						(1 << 4)
#define	IntUSB_SOF						(1 << 3)
#define	IntUSB_p_Reset					(1 << 2)
#define	IntUSB_h_Babble					(1 << 2)
#define	IntUSB_Resume					(1 << 1)
#define	IntUSB_Suspend					(1 << 0)


// otg_CommonUSB_TestMode
#define	TestMode_rw_ForceHost			(1 << 7)
#define	TestMode_rw_FIFOAccess			(1 << 6)
#define	TestMode_rw_ForceFS				(1 << 5)
#define	TestMode_rw_ForceHS				(1 << 4)
#define	TestMode_rw_Packet				(1 << 3)
#define	TestMode_rw_K					(1 << 2)
#define	TestMode_rw_J					(1 << 1)
#define	TestMode_rw_SE0_NAK				(1 << 0)


// otg_IndexedCSR_CSR0
#define	CSR0_ps_FlushFIFO				(1 << 8)
#define	CSR0_ps_ServicedSetupEnd		(1 << 7)
#define	CSR0_ps_ServicedRxPktRdy		(1 << 6)
#define	CSR0_ps_SendStall				(1 << 5)
#define	CSR0_pr_SetupEnd				(1 << 4)
#define	CSR0_ps_DataEnd					(1 << 3)
#define	CSR0_prc_SentStall				(1 << 2)
#define	CSR0_prs_TxPktRdy				(1 << 1)
#define	CSR0_pr_RxPktRdy				(1 << 0)

#define	CSR0_hrw_DisPing				(1 << 11)
#define	CSR0_hs_FlushFIFO				(1 << 8)
#define	CSR0_hrc_NAKTimeout				(1 << 7)
#define	CSR0_hrw_StatusPkt				(1 << 6)
#define	CSR0_hrw_ReqPkt					(1 << 5)
#define	CSR0_hrc_Error					(1 << 4)
#define	CSR0_hrw_SetupPkt				(1 << 3)
#define	CSR0_hrc_RxStall				(1 << 2)
#define	CSR0_hrs_TxPktRdy				(1 << 1)
#define	CSR0_hrc_RxPktRdy				(1 << 0)


// otg_IndexedCSR_TxCSR
#define	TxCSR_prw_AutoSet				(1 << 15)
#define TxCSR_prw_ISO					(1 << 14)
#define	TxCSR_prw_Mode					(1 << 13)
#define	TxCSR_prw_DMAReqEnab			(1 << 12)
#define	TxCSR_prw_FrcDataTog			(1 << 11)
#define	TxCSR_prw_DMAReqMode			(1 << 10)
#define	TxCSR_prc_IncompTx				(1 << 7)
#define	TxCSR_ps_ClrDataTog				(1 << 6)
#define	TxCSR_prc_SentStall				(1 << 5)
#define	TxCSR_prw_SendStall				(1 << 4)
#define	TxCSR_ps_FlushFIFO				(1 << 3)
#define	TxCSR_prc_UnderRun				(1 << 2)
#define	TxCSR_prc_FIFONotEmpty			(1 << 1)
#define	TxCSR_prs_TxPktRdy				(1 << 0)

#define	TxCSR_hrw_AutoSet				(1 << 15)
#define	TxCSR_hrw_Mode					(1 << 13)
#define	TxCSR_hrw_DMAReqEnab			(1 << 12)
#define	TxCSR_hrw_FrcDataTog			(1 << 11)
#define	TxCSR_hrw_DMAReqMode			(1 << 10)
#define	TxCSR_hrc_NAKTimeout_IncompTx	(1 << 7)
#define	TxCSR_hs_ClrDataTog				(1 << 6)
#define	TxCSR_hrc_RxStall				(1 << 5)
#define	TxCSR_hs_FlushFIFO				(1 << 3)
#define	TxCSR_hrc_Error					(1 << 2)
#define	TxCSR_hrc_FIFONotEmpty			(1 << 1)
#define	TxCSR_hrs_TxPktRdy				(1 << 0)


// otg_IndexedCSR_RxCSR
#define	RxCSR_prw_AutoClear				(1 << 15)
#define	RxCSR_prw_ISO					(1 << 14)
#define	RxCSR_prw_DMAReqEnab			(1 << 13)
#define	RxCSR_prw_DisNyet_pr_PIDError	(1 << 12)
#define	RxCSR_prw_DMAReqMode			(1 << 11)
#define	RxCSR_prc_IncompRx				(1 << 8)
#define	RxCSR_ps_ClrDataTog				(1 << 7)
#define	RxCSR_prc_SentStall				(1 << 6)
#define	RxCSR_prw_SendStall				(1 << 5)
#define	RxCSR_ps_FlushFIFO				(1 << 4)
#define	RxCSR_pr_DataError				(1 << 3)
#define	RxCSR_prc_OverRun				(1 << 2)
#define	RxCSR_pr_FIFOFull				(1 << 1)
#define	RxCSR_prc_RxPktRdy				(1 << 0)

#define	RxCSR_hrw_AutoClear				(1 << 15)
#define	RxCSR_hrw_AutoReq				(1 << 14)
#define	RxCSR_hrw_DMAReqEnab			(1 << 13)
#define	RxCSR_hr_PIDError				(1 << 12)
#define	RxCSR_hrw_DMAReqMode			(1 << 11)
#define	RxCSR_hrc_IncompRx				(1 << 8)
#define	RxCSR_hs_ClrDataTog				(1 << 7)
#define	RxCSR_hrc_RxStall				(1 << 6)
#define	RxCSR_hrw_ReqPkt				(1 << 5)
#define	RxCSR_hs_FlushFIFO				(1 << 4)
#define	RxCSR_hrc_DataError_NAKTimeout	(1 << 3)
#define	RxCSR_hrc_Error					(1 << 2)
#define	RxCSR_hr_FIFOFull				(1 << 1)
#define	RxCSR_hrc_RxPktRdy				(1 << 0)


// otg_IndexedCSR_host_TxType
#define	TxType_rw_Protocol				(3 << 4)
#define	TxType_Protocol_Illegal			(0 << 4)
#define	TxType_Protocol_Isochronous		(1 << 4)
#define	TxType_Protocol_Bulk			(2 << 4)
#define	TxType_Protocol_Interrupt		(3 << 4) 
#define	TxType_rw_TargetEndpointNumber	(15 << 0)


// otg_IndexedCSR_host_RxType
#define	RxType_rw_Protocol				(3 << 4)
#define	RxType_Protocol_Illegal			(0 << 4)
#define	RxType_Protocol_Isochronous		(1 << 4)
#define	RxType_Protocol_Bulk			(2 << 4)
#define	RxType_Protocol_Interrupt		(3 << 4) 
#define	RxType_rw_TargetEndpointNumber	(15 << 0)


// otg_IndexedCSR_dev_ConfigData
#define	ConfigData_r_MPRxE				(1 << 7)
#define	ConfigData_r_MPTxE				(1 << 6)
#define	ConfigData_r_BigEndian			(1 << 5)
#define	ConfigData_r_HBRxE				(1 << 4)
#define	ConfigData_r_HBTxE				(1 << 3)
#define	ConfigData_r_DynFIFOSizing		(1 << 2)
#define	ConfigData_r_SoftConE			(1 << 1)
#define	ConfigData_r_UTMIDataWidth		(1 << 0)
		

// otg_CtrlFIFO_DevCtl
#define	DevCtl_r_B_Device				(1 << 7)
#define	DevCtl_r_FSDev					(1 << 6)
#define	DevCtl_r_LSDev					(1 << 5)
#define DevCtl_r_VBus					(3 << 3)
#define	DevCtl_VBus_bSE					(0 << 3)
#define	DevCtl_VBus_aSE_bAV				(1 << 3)
#define	DevCtl_VBus_aAV_bVBus			(2 << 3)
#define	DevCtl_VBus_aVBus				(3 << 3)
#define	DevCtl_r_HostMode				(1 << 2)
#define	DevCtl_rw_HostReq				(1 << 1)
#define	DevCtl_rw_Session				(1 << 0)

// otg_CtrlFIFO_TxFIFOSz
#define	TxFIFOsz_rw_DPB					(1 << 4)

// otg_CtrlFIFO_RxFIFOSz
#define	RxFIFOsz_rw_DPB					(1 << 4)

// otg_DMA_Intr
#define	DMA_Intr_Ch1					(1 << 0)

// otg_DMA_Ctrl
#define	DMA_Ctrl_EnDMA					(1 << 0)
#define	DMA_Ctrl_Direction				(1 << 1)
#define	DMA_Ctrl_Mode					(1 << 2)
#define	DMA_Ctrl_IntrEn					(1 << 3)
#define	DMA_Ctrl_EpNum					(15 << 4)
#define	DMA_Ctrl_BusErr					(1 << 8)
#define	DMA_Ctrl_BurstMode				(3 << 9)



// otg register operation 
BYTE ReadOTGReg8(WORD Addr);
VOID WriteOTGReg8(WORD Addr, BYTE Val);
#define SetBitOTGReg8(Addr, Mask)	(WriteOTGReg8(Addr, ReadOTGReg8(Addr) | (BYTE)(Mask)))
#define ClrBitOTGReg8(Addr, Mask)	(WriteOTGReg8(Addr, ReadOTGReg8(Addr) & (~((BYTE)(Mask)))))

#define ReadOTGReg16(Addr)			(ReadOTGReg8((WORD)(Addr)) | (ReadOTGReg8((WORD)(Addr) + 1) << 8))
#define WriteOTGReg16(Addr, Val)	(WriteOTGReg8((WORD)(Addr), (BYTE)(Val)), WriteOTGReg8((WORD)(Addr) + 1, (BYTE)((WORD)(Val) >> 8)))
#define SetBitOTGReg16(Addr, Mask)	(WriteOTGReg16(Addr, ReadOTGReg16(Addr) | (WORD)(Mask)))
#define ClrBitOTGReg16(Addr, Mask)	(WriteOTGReg16(Addr, ReadOTGReg16(Addr) & (~((WORD)(Mask)))))

#define ReadOTGReg32(Addr)			(ReadOTGReg8((WORD)(Addr)) | (ReadOTGReg8((WORD)(Addr) + 1) << 8) | (ReadOTGReg8((WORD)(Addr) + 2) << 16) | (ReadOTGReg8((WORD)(Addr) + 3) << 24))
#define WriteOTGReg32(Addr, Val)	(XBYTE[(WORD)(Addr)] = ((BYTE)(Val)), XBYTE[(WORD)(Addr) + 1] = (BYTE)((WORD)(Val) >> 8), XBYTE[(DWORD)(Addr) + 2] = (BYTE)((WORD)(Val) >> 16), XBYTE[(DWORD)(Addr) + 3] = (BYTE)((DWORD)(Val) >> 24))
#define SetBitOTGReg32(Addr, Mask)	(WriteOTGReg32(Addr, ReadOTGReg32(Addr) | (DWORD)(Mask)))
#define ClrBitOTGReg32(Addr, Mask)	(WriteOTGReg32(Addr, ReadOTGReg32(Addr) & (~((DWORD)(Mask)))))


// Load data from memory to otg send fifo.
VOID LoadFIFOData(WORD FifoAddr, BYTE Len, BYTE* Buf);

// Unload data to memory from otg send fifo.
VOID UnloadFIFOData(WORD FifoAddr, BYTE Len, BYTE* Buf);


#endif