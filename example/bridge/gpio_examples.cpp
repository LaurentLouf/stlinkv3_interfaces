
// WARNING: Instantiation, initialization steps are missing from the examples

// EXAMPLE FOR GPIO input CLK, Brg::GetClk()

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT brgStat = BRG_NO_ERR;
uint32_t currFreqKHz = 0;
uint8_t com = COM_GPIO;
uint32_t StlHClkKHz, comInputClkKHz;
// Get the current bridge input Clk
brgStat = m_pBrg->GetClk(com, &comInputClkKHz, &StlHClkKHz);
printf("GPIO input CLK: %d KHz, STLink HCLK: %d KHz \n", (int)comInputClkKHz, (int)StlHClkKHz);

// EXAMPLE FOR GPIO Initialization, Brg::InitGPIO()

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT BrgStatus = BRG_NO_ERR;
Brg_GpioInitT gpioParams;
Brg_GpioConfT gpioConf[BRG_GPIO_MAX_NB];
Brg_GpioValT gpioReadVal[BRG_GPIO_MAX_NB];
Brg_GpioValT gpioWriteVal[BRG_GPIO_MAX_NB];
uint8_t gpioMsk = 0, gpioErrMsk;

int i;
gpioMsk = BRG_GPIO_ALL;
gpioParams.GpioMask = gpioMsk;          // BRG_GPIO_0 1 2 3
gpioParams.ConfigNb = BRG_GPIO_MAX_NB;  // must be BRG_GPIO_MAX_NB or 1 (if 1 pGpioConf[0] used for all gpios)
gpioParams.pGpioConf = &gpioConf[0];
for (i = 0; i < BRG_GPIO_MAX_NB; i++) {
    gpioConf[i].Mode = GPIO_MODE_INPUT;             // GPIO_MODE_INPUT GPIO_MODE_OUTPUT GPIO_MODE_ANALOG
    gpioConf[i].Speed = GPIO_SPEED_MEDIUM;          // GPIO_SPEED_LOW GPIO_SPEED_MEDIUM GPIO_SPEED_HIGH GPIO_SPEED_VERY_HIGH
    gpioConf[i].Pull = GPIO_PULL_UP;                // GPIO_NO_PULL GPIO_PULL_UP GPIO_PULL_DOWN
    gpioConf[i].OutputType = GPIO_OUTPUT_PUSHPULL;  // GPIO_OUTPUT_PUSHPULL GPIO_OUTPUT_OPENDRAIN
}
BrgStatus = m_pBrg->InitGPIO(&gpioParams);
if (BrgStatus != BRG_NO_ERR) {
    printf("Bridge Gpio init failed (mask=%d, gpio0: mode= %d, pull = %d, ...)\n", (int)gpioParams.GpioMask, (int)gpioConf[0].Mode,
           (int)gpioConf[0].Pull);
}

// EXAMPLE FOR GPIO Read, Brg::ReadGPIO()

Brg *m_pBrg;
**********[Missing init steps] * *********
                                         // same config as InitGPIO() above

                                         BrgStatus = m_pBrg->ReadGPIO(gpioMsk, &gpioReadVal[0], &gpioErrMsk);
if ((BrgStatus != BRG_NO_ERR) || (gpioErrMsk != 0)) {
    printf(" Bridge Read error\n");
} else {
    // verify all gpio read to 1 (input pull up)
    for (i = 0; i < BRG_GPIO_MAX_NB; i++) {
        if (gpioReadVal[i] != GPIO_SET) {
            BrgStatus = BRG_VERIF_ERR;
            printf(" Bridge Read Verif error ( gpio %d != SET)\n", i);
        }
    }
}

if (BrgStatus == BRG_NO_ERR) {
    printf("GPIO Test OK \n");
}

// EXAMPLE FOR GPIO close, Brg::CloseBridge()

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT brgStat = BRG_NO_ERR;

brgStat = m_pBrg->CloseBridge(COM_GPIO);
