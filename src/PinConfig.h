#ifndef PinConfig_h
#define PinConfig_h
#include <sam.h>

void configure_extint() {
    // Clock für EIC aktivieren
    PM->APBAMASK.reg |= PM_APBAMASK_EIC;

    // GCLK für EIC aktivieren
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_EIC |
                        GCLK_CLKCTRL_GEN_GCLK0 |
                        GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY);

    // PA07 (Arduino D9) für EIC konfigurieren
    PORT->Group[PORTA].PINCFG[7].bit.PMUXEN = 1;
    PORT->Group[PORTA].PMUX[7 >> 1].bit.PMUXO = PORT_PMUX_PMUXO_A;

    // Eingang aktivieren und internen Pull-up setzen
    PORT->Group[PORTA].PINCFG[7].bit.INEN = 1;
    PORT->Group[PORTA].PINCFG[7].bit.PULLEN = 1;
    PORT->Group[PORTA].OUTSET.reg = (1 << 7);  // Pull-Up aktivieren

    // EIC deaktivieren vor Konfiguration
    EIC->CTRL.bit.ENABLE = 0;
    while (EIC->STATUS.bit.SYNCBUSY);

    // Sense-Konfiguration: FALLING EDGE
    EIC->CONFIG[PORTA].reg &= ~EIC_CONFIG_SENSE7_Msk;
    EIC->CONFIG[PORTA].reg |= EIC_CONFIG_SENSE7_BOTH;

    // EXTINT7 aktivieren
    EIC->INTENSET.reg = EIC_INTENSET_EXTINT7;

    // EIC aktivieren
    EIC->CTRL.bit.ENABLE = 1;
    while (EIC->STATUS.bit.SYNCBUSY);

    // NVIC aktivieren
    NVIC_EnableIRQ(EIC_IRQn);
}

#endif