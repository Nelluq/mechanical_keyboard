#include "keyboard.h"
#include "usb_hid.h"

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

bool hid_init = false;

static void setup_clock(void) {
    rcc_osc_on(RCC_HSE);
	rcc_wait_for_osc_ready(RCC_HSE);
	rcc_set_sysclk_source(RCC_HSE);

	rcc_set_hpre(RCC_CFGR_HPRE_NODIV);
	rcc_set_ppre(RCC_CFGR_PPRE_NODIV);

	flash_prefetch_enable();
	flash_set_ws(FLASH_ACR_LATENCY_024_048MHZ);

	// set PLL for 48MHz (12MHz HSI * 4 = 48MHz)
	rcc_set_pll_multiplication_factor(RCC_CFGR_PLLMUL_MUL4);
	rcc_set_pll_source(RCC_CFGR_PLLSRC_HSE_CLK);
	rcc_set_pllxtpre(RCC_CFGR_PLLXTPRE_HSE_CLK);

	rcc_osc_on(RCC_PLL);
	rcc_wait_for_osc_ready(RCC_PLL);
	rcc_set_sysclk_source(RCC_PLL);

	rcc_apb1_frequency = 48000000;
	rcc_ahb_frequency = 48000000;

	systick_set_clocksource(STK_CSR_CLKSOURCE_EXT);
	systick_set_frequency(1000 / KEYBOARD_POLL_INTERVAL_MS, rcc_ahb_frequency);
	systick_counter_enable();
	systick_interrupt_enable();
}

void sys_tick_handler(void) {
	keyboard_poll();
}

int main(void) {
    setup_clock();
	usb_hid_init();
    keyboard_init();

    while(1) {
		usb_hid_poll();
    }
}
