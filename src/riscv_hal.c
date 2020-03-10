#include <stdint.h>
#include "riscv_hal.h"
#include "xio.h"

#ifdef __cplusplus
extern "C" {
#endif
int IntDefaultHandler(unsigned int reason);

//*****************************************************************************
//
// Macro for weak symbol aliasing
//
//*****************************************************************************
#define WEAK_ALIAS(x) __attribute__ ((weak, alias(#x)))
volatile int dummy_interrupt_return;
/*------------------------------------------------------------------------------
 *
 */
/*int IRQHandler_1(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_2(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_3(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_4(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_5(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_6(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_7(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_8(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_9(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_10(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_11(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_12(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_13(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_14(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_15(unsigned int) WEAK_ALIAS(IntDefaultHandler);
int IRQHandler_16(unsigned int) WEAK_ALIAS(IntDefaultHandler);*/


/*int (* const g_pfnVectors[])(unsigned int) =
{
	IRQHandler_1,
	IRQHandler_2,
	IRQHandler_3,
	IRQHandler_4,
	IRQHandler_5,
	IRQHandler_6,
	IRQHandler_7,
	IRQHandler_8,
	IRQHandler_9,
	IRQHandler_10,
	IRQHandler_11,
	IRQHandler_12,
	IRQHandler_13,
	IRQHandler_14,
	IRQHandler_15,
	IRQHandler_16,
};*/


ISR(IRQHandler)
{
	//ISR_PROLOGUE();
	/*asm volatile
	(
		"xor t0, t0, t0" "\n\t"
		"xor t1, t1, t1" "\n\t"
		"xor t2, t2, t2" "\n\t"
		"xor s0, s0, s0" "\n\t"
		"xor s1, s1, s1" "\n\t"
		"xor a0, a0, a0" "\n\t"
		"xor a1, a1, a1" "\n\t"
		"xor a2, a2, a2" "\n\t"
		"xor a3, a3, a3" "\n\t"
		"xor a4, a4, a4" "\n\t"
		"xor a5, a5, a5" "\n\t"
		"xor a6, a6, a6" "\n\t"
		"xor a7, a7, a7" "\n\t"
		"xor s2, s2, s2" "\n\t"
		"xor s3, s3, s3" "\n\t"
		"xor a0, a0, a0" "\n\t"
		"xor s4, s4, s4" "\n\t"
		"xor s5, s5, s5" "\n\t"
		"xor s6, s6, s6" "\n\t"
		"xor s7, s7, s7" "\n\t"
		"xor s8, s8, s8" "\n\t"
		"xor s9, s9, s9" "\n\t"
		"xor s10, s10, s10" "\n\t"
		"xor s11, s11, s11" "\n\t"
		"xor t3, t3, t3" "\n\t"
		"xor t4, t4, t4" "\n\t"
		"xor t5, t5, t5" "\n\t"
		"xor t6, t6, t6" "\n\t"
	);*/
	/*int interrupt_nr;
	__asm__ __volatile__ (
		"lw %0, 1*4(sp)"
	: "=r" (interrupt_nr)
	:);

	int (*int_to_call)(unsigned int) = g_pfnVectors[interrupt_nr];
	int_to_call(interrupt_nr);

	ISR_EPILOGUE();*/
}

/*------------------------------------------------------------------------------
 *
 */
/*------------------------------------------------------------------------------
 * RISC-V interrupt handler for external interrupts.
 */
/*__attribute__ ((section(".vectors"), used))
void vector_table()
{
	asm volatile ("j _entry");
	asm volatile ("j IRQHandler");
}*/

//*****************************************************************************
//
//! This is the code that gets called when the processor receives an unexpected
//! interrupt. This simply enters an infinite loop, preserving the system state
//! for examination by a debugger.
//
//*****************************************************************************
int IntDefaultHandler(unsigned int reason)
{
    //
    // Go into an infinite loop.
    //
    while(1)
    {
    }
}
/*------------------------------------------------------------------------------
 * Disable all interrupts.
 */
void __disable_irq(void)
{
	REG_MSTATUS = (REG_MSTATUS & ~REG_MSTATUS_MIE_bm);
	//clear_csr(mstatus, MSTATUS_MPIE);
    //clear_csr(mstatus, MSTATUS_MIE);
}

/*------------------------------------------------------------------------------
 * Enabler all interrupts.
 */
void __enable_irq(void)
{
	REG_MSTATUS = (REG_MSTATUS | REG_MSTATUS_MIE_bm);
    //set_csr(mstatus, MSTATUS_MIE);
}

#ifdef __cplusplus
}
#endif
