
NAME	FILE_DEC_ASM

?PR?_BufferConvert?FILE_DEC_ASM                  SEGMENT CODE 

	PUBLIC	_BufferConvert



//�����ܺ����������������������ֽ����0x55
//�û��ɰ����Լ��ļ��ܷ�ʽ���޸�
//�����㷨��Ӧ̫���ӣ����򲥷Ÿ����ʸ���ʱ�����ٶȻ�������

//VOID BufferConvert(WORD Addr, WORD Len)
//R6R7: 	Addr
//R4R5: 	Len

	RSEG  ?PR?_BufferConvert?FILE_DEC_ASM
_BufferConvert:
	USING	0

	MOV  	DPL,	R7
	MOV  	DPH,	R6

	MOV		A, 		R5
	JNZ		BUFFER_CONVERT_LOOP2

BUFFER_CONVERT_LOOP1:
	MOV		A, 		R4
	JZ		BUFFER_CONVERT_EXIT
	DEC		R4

BUFFER_CONVERT_LOOP2: 
	MOVX 	A,		@DPTR
	XRL  	A,		#055H
	MOVX 	@DPTR,	A
	INC		DPTR

	DJNZ	R5, 	BUFFER_CONVERT_LOOP2 
	LJMP	BUFFER_CONVERT_LOOP1


BUFFER_CONVERT_EXIT:
	RET


	END