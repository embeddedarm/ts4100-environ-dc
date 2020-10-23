/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdarg.h>
#include <string.h>

#include "ts_zpu.h"


#define BKLT_EN	(1 << 14)	// DIO_41/5_78/Bank 2
#define PIR_IN	(1 << 15)	// DIO_42/5_79/Bank 2

/* This works so long as the maximum delay count is not more than half the span
 * of the free running counter. The counter in this is 32 bits wide, so limit
 * the max delay to 16 bits. In practice, this MUXBUS application will come
 * nowhere near that max.
 */
void delay_clks(unsigned short cnt)
{
	unsigned long end_time;

	end_time = TIMER_REG + cnt;
	while ((signed long)(end_time - TIMER_REG) > 0);
}

/* ZPU TS-4100-ENVIRON-DC OpenPIR Backlight driver
 *
 * This simple application allows for control of the LCD backlight from the
 * attached OpenPIR sensor. If the sensor is left unconnected, the backlight
 * will remain on indefinitely. If the ZPU program is not loaded, the CPU can
 * maintain control of the backlight via FPGA GPIO.
 *
 * Once the input is asserted from the OpenPIR, turn on the backlight for 10 s
 * If at any time during this delay the PIR sensor is re-asserted, restart the
 * timeout for longer on time. This also prevents a stuttering right around
 * the timeout time.
 */
int main(void)
{
	int i;
	/* Set backlight as a low output */
	O_REG2 &= ~(BKLT_EN);
	OE_REG2 |= ~(BKLT_EN);
	/* Set the PIR enable as an input */
	OE_REG2 &= ~(PIR_IN);


	while (1) {
		if (!!(I_REG2 & PIR_IN)) {
			O_REG2 |= (BKLT_EN);
			/* 1 s of delay is ~961 loops of delay_clks(0xFFFF) */
			for (i = 0; i < 9610; i++) {
				/* Reset the counter if PIR is tripped again */
				if (!!(I_REG2 & PIR_IN)) i = 0;
				delay_clks(0xFFFF);
			}
			O_REG2 &= ~(BKLT_EN);
		}
	}

	return 1;
}

