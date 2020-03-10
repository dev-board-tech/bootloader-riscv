//============================================================================
// Name        : main.c
// Author      : Iulian Gheorghiu
// Version     :
// Copyright   : Your copyright notice
// Description : Hello RISC-V World in C++
//============================================================================

#include "stdbool.h"
#include "string.h"
#include "xio.h"
#include "riscv_hal.h"


#define CPU_FREQ						(72000000)// This is used even for Uart master clock.If your Uart has another master clock source please create one and modify accordingly.
#define _BaudRate						(115200)
#define PGM_MEM_ADDR					0x800000
//-----------------------------------------------------
#define HexBufferLenght					(100)//43 Generic
#define BinBufferLength					(HexBufferLenght/2)
#define TimeToWaitEnterInBootLoader		(1)
#define UART							(*(FPGA_USART_t *) 0x4010)  /* Virtual Port */
#define delay_us						(CPU_FREQ / 500000)
//-----------------------------------------------------
//Intel hex definitions
#define Data_Record						0//Contains data and 16-bit address. The format described above.
#define EndOfFile_Record				1//A file termination record. No data. Has to be the last line of the file, only one per file permitted. Usually ':00000001FF'. Originally the End Of File record could contain a start address for the program being loaded, e.g. :00AB2F0125 would make a jump to address AB2F. This was convenient when programs were loaded from punched paper tape.
#define ExtendedSegmentAddress_Record	2//Segment-base address. Used when 16 bits are not enough, identical to 80x86 real mode addressing. The address specified by the 02 record is multiplied by 16 (shifted 4 bits left) and added to the subsequent 00 record addresses. This allows addressing of up to a megabyte of address space. The address field of this record has to be 0000, the byte count is 02 (the segment is 16-bit). The least significant hex digit of the segment address is always 0.
#define StartSegmentAddress_Record		3//For 80x86 processors, it specifies the initial content of the CS:IP registers. The address field is 0000, the byte count is 04, the first two bytes are the CS value, the latter two are the IP value.
#define ExtendedLinearAddress_Record	4//Allowing for fully 32 bit addressing. The address field is 0000, the byte count is 02. The two data bytes represent the upper 16 bits of the 32 bit address, when combined with the address of the 00 type record.
#define StartLinearAddress_Record		5//The address field is 0000, the byte count is 04. The 4 data bytes represent the 32-bit value loaded into the EIP register of the 80386 and higher CPU.
//-----------------------------------------------------
//Errors
#define Error_LineDefError				'a'
#define Error_SecondHexCharNotFound		'b'
#define Error_CheckSum					'c'
#define Error_LineMismach				'd'
#define Error_NoMemorySelected			'e'
#define Error_NoError					'k'
//-----------------------------------------------------
#define EnterToFlashWrite				(1)
#define EnterToEepWrite					(2)
#define EnterInUndefinedWrite			(255)
//-----------------------------------------------------

volatile unsigned long long STimerCnt = 0;
volatile unsigned char *pgmMem = (unsigned char *)PGM_MEM_ADDR;


#define SET		OUTSET
#define CLEAR	OUTCLR

typedef union
{
unsigned short i16;
	struct
	{
		unsigned char Byte0;
		unsigned char Byte1;
	};
} convert16to8;

typedef union
{
	struct
	{
		unsigned char Byte0;
		unsigned char Byte1;
		unsigned char Byte2;
		unsigned char Byte3;
	};unsigned long LongReturn;
}convert8to32;

void _delay_us(unsigned long time)
{
	STimerCnt = time;
	do
	{
		STimerCnt--;
		for(unsigned int cnt = delay_us; cnt > 0; cnt--);
	}while(STimerCnt);
}

//#####################################################
char RxHexBuffer[HexBufferLenght];
uint8_t RxBinBuffer[BinBufferLength];
uint32_t ExtendedSegmentAddressRecord;

uint8_t CountProcessedBytes = 0;
uint8_t CountToExtractBytesFromBinBuffer = 0;
uint8_t CheckSum = 0;
//#####################################################
void usart_init(void)
{
	convert16to8 BaudRate;
	BaudRate.i16 = (CPU_FREQ/8/_BaudRate)-1;
	UART.BAUDCTRLA = BaudRate.Byte0;
	UART.BAUDCTRLB = BaudRate.Byte1;
	UART.CTRLA = FPGA_USART_RXCINTLVL_OFF_gc | FPGA_USART_TXCINTLVL_OFF_gc | FPGA_USART_DREINTLVL_OFF_gc;
	UART.CTRLB = FPGA_USART_RXEN_bm | FPGA_USART_TXEN_bm | FPGA_USART_CLK2X_bm;
	UART.CTRLC = FPGA_USART_CMODE_ASYNCHRONOUS_gc | FPGA_USART_PMODE_DISABLED_gc | FPGA_USART_CHSIZE_8BIT_gc;
}
//#####################################################
inline bool usart_check_send_busy(void)
{
	if(UART.STATUS&FPGA_USART_DREIF_bm)
		return false;
	else
		return true;
}
//#####################################################
inline bool usart_check_rec_data(void)
{
	if(UART.STATUS&FPGA_USART_RXCIF_bm)
		return true;
	else
		return false;
}
//#####################################################
void usart_putc(char Chr)
{
	while(usart_check_send_busy());
	UART.DATA = Chr;
}
//#####################################################
char usart_getc(void)
{
	return UART.DATA;
}
//#####################################################
void clear_buff(void)
{
	for(uint16_t ClearCount = 0; ClearCount < HexBufferLenght; ClearCount++)
	{
		RxHexBuffer[ClearCount] = 0x00;
	}
}
//#####################################################
bool strcmp_enter_to_boot(void)
{
	if(RxHexBuffer[0] != 'B') return false;
	if(RxHexBuffer[1] != 'o') return false;
	if(RxHexBuffer[2] != 'o') return false;
	if(RxHexBuffer[3] != 't') return false;
	if(RxHexBuffer[4] != 'I') return false;
	if(RxHexBuffer[5] != 'n') return false;
	if(RxHexBuffer[6] != 'i') return false;
	if(RxHexBuffer[7] != 't') return false;
	return true;
}
//#####################################################
bool strcmp_enter_to_flash(void)
{
	if(RxHexBuffer[0] != 'F') return false;
	if(RxHexBuffer[1] != 'l') return false;
	if(RxHexBuffer[2] != 'a') return false;
	if(RxHexBuffer[3] != 's') return false;
	if(RxHexBuffer[4] != 'h') return false;
	if(RxHexBuffer[5] != 'W') return false;
	return true;
}
//#####################################################
bool strcmp_enter_to_eep(void)
{
	if(RxHexBuffer[0] != 'E') return false;
	if(RxHexBuffer[1] != 'E') return false;
	if(RxHexBuffer[2] != 'P') return false;
	if(RxHexBuffer[3] != 'r') return false;
	if(RxHexBuffer[4] != 'o') return false;
	if(RxHexBuffer[5] != 'm') return false;
	if(RxHexBuffer[6] != 'W') return false;
	return true;
}
//#####################################################
bool strcmp_enter_to_exit(void)
{
	if(RxHexBuffer[0] != 'E') return false;
	if(RxHexBuffer[1] != 'x') return false;
	if(RxHexBuffer[2] != 'i') return false;
	if(RxHexBuffer[3] != 't') return false;
	return true;
}
//#####################################################
uint8_t receive_data()
{
	uint32_t TimeCount = 0;
	uint8_t CharCount = 0;
	char Char;
	do
	{
		TimeCount++;
		if(usart_check_rec_data())
		{
			TimeCount = 0;
			Char = usart_getc();
			if(Char == 13)
			{
				RxHexBuffer[CharCount] = 0;
				break;
			}
			RxHexBuffer[CharCount] = Char;
			CharCount++;
		}
		_delay_us(10);
	} while (TimeCount != (TimeToWaitEnterInBootLoader*10000));
	return CharCount;
}
//#####################################################
uint8_t check_if_is_hex_char_and_convert(char Ch)
{
	uint8_t Tmp = (uint8_t)Ch - '0';
	if(Tmp < (':'-'0'))
		return Tmp;
	Tmp -= ('@'-'9');
	if(Tmp < 16)
		return Tmp;
	return 255;
}
//#####################################################
typedef struct
{
	uint8_t nr_of_bytes;
	uint8_t error;
} hex_to_bin_return;
//#####################################################
hex_to_bin_return hex_to_bin()
{
	uint8_t ConvertedByte = 0;
	uint8_t Tmp = 0;
	uint8_t BytesCount = 0;
	hex_to_bin_return Return;
	for (uint8_t ConvertCount = 1; ConvertCount < strlen(RxHexBuffer); ConvertCount += 2)
	{
		ConvertedByte = check_if_is_hex_char_and_convert(RxHexBuffer[ConvertCount])<<4;
		Tmp = check_if_is_hex_char_and_convert(RxHexBuffer[ConvertCount+1]);
		if(Tmp == 255)
		{
			Return.error = Error_SecondHexCharNotFound;
			return Return;
		}
		ConvertedByte |= Tmp;
		RxBinBuffer[BytesCount] = ConvertedByte;
		BytesCount++;
	}
	Return.error = Error_NoError;
	Return.nr_of_bytes = BytesCount;
	return Return;
}
//#####################################################
uint8_t check_sum()
{
	if(CountProcessedBytes != 0)
	{
		do
		{
			CheckSum += RxBinBuffer[CountToExtractBytesFromBinBuffer++];
		}while(--CountProcessedBytes);
	}
	if(RxBinBuffer[CountToExtractBytesFromBinBuffer] == (uint8_t)(0-CheckSum))
		return Error_NoError;
	return Error_CheckSum;
}
//#####################################################
void append_line(uint32_t Addr)
{
	if(CountProcessedBytes)
	{
		unsigned char *PageAddr = 0;
		uint8_t AppendCount = 0;
		uint8_t Byte = 0;
		do
		{
			Byte = RxBinBuffer[CountToExtractBytesFromBinBuffer++];
			CheckSum += Byte;
			PageAddr[(Addr + AppendCount)] = Byte;
			AppendCount++;
		} while (--CountProcessedBytes);
	}
}
//#####################################################
void (*main_pgm)(void) = (void *)PGM_MEM_ADDR;
int
main(void)
{
	uint8_t ReceivedCharNr = 0;
	hex_to_bin_return Return;
	uint8_t DataBytesNrInLine = 0;
	uint8_t AddressByte0 = 0;
	uint8_t AddressByte1 = 0;
	uint8_t LineFunction = 0;
	uint32_t WriteAddr = 0;
	convert8to32 Union32;

	usart_init();
	clear_buff();
	receive_data();
	if(!strcmp_enter_to_boot())
	{
		REG_MSCRATCH = PGM_MEM_ADDR;
		goto *(PGM_MEM_ADDR);
	}
	do
	{
		clear_buff();
		usart_putc('k');
		ReceivedCharNr = receive_data();
		if(strcmp_enter_to_exit())
		{
			break;
		}
		else
		{
			CountToExtractBytesFromBinBuffer = 0;
			Return = hex_to_bin();
			if(Return.error == Error_SecondHexCharNotFound)
			{
				usart_putc(Error_SecondHexCharNotFound);
				break;
			}
			//Verify integrity of bin line
			DataBytesNrInLine = RxBinBuffer[CountToExtractBytesFromBinBuffer++];
			CountProcessedBytes = Return.nr_of_bytes - 5;
			if(CountProcessedBytes != DataBytesNrInLine)
			{
				usart_putc(Error_LineMismach);
				break;
			}
			AddressByte1 = RxBinBuffer[CountToExtractBytesFromBinBuffer++];
			AddressByte0 = RxBinBuffer[CountToExtractBytesFromBinBuffer++];
			LineFunction = RxBinBuffer[CountToExtractBytesFromBinBuffer++];
			CheckSum = DataBytesNrInLine;
			CheckSum += AddressByte1;
			CheckSum += AddressByte0;
			CheckSum += LineFunction;
			if(LineFunction == Data_Record)
			{
				Union32.Byte0 = AddressByte0;
				Union32.Byte1 = AddressByte1;
				Union32.Byte2 = 0;
				Union32.Byte3 = 0;
				WriteAddr = ExtendedSegmentAddressRecord + Union32.LongReturn;
				append_line(WriteAddr);
				uint8_t Tmp = check_sum();
				if(Tmp == Error_CheckSum)
				{
					usart_putc(Error_CheckSum);
					break;
				}
			}
			else if(LineFunction == EndOfFile_Record)
			{
				uint8_t Tmp = check_sum();
				if(Tmp == Error_CheckSum)
				{
					usart_putc(Error_CheckSum);
					break;
				}
			}
			else if(LineFunction == ExtendedSegmentAddress_Record)
			{
				uint8_t Tmp1 = RxBinBuffer[CountToExtractBytesFromBinBuffer++];
				uint8_t Tmp0 = RxBinBuffer[CountToExtractBytesFromBinBuffer++];
				CheckSum += Tmp0;
				CheckSum += Tmp1;
				CountProcessedBytes -= 2;
				Union32.Byte0 = Tmp0;
				Union32.Byte1 = Tmp1;
				Union32.Byte2 = 0;
				Union32.Byte3 = 0;
				ExtendedSegmentAddressRecord = Union32.LongReturn<<4;
			}
			else
				usart_putc(Error_LineDefError);
		}
	} while(true);
	usart_putc('f');
	_delay_us(50000);
}


