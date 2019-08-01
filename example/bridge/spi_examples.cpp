
// WARNING: Instantiation, initialization steps are missing from the examples

// EXAMPLE FOR SPI input CLK, Brg::GetClk()

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT brgStat = BRG_NO_ERR;
uint32_t currFreqKHz = 0;
uint8_t com = COM_SPI;
uint32_t StlHClkKHz, comInputClkKHz;
// Get the current bridge input Clk
brgStat = m_pBrg->GetClk(com, &comInputClkKHz, &StlHClkKHz);
printf("SPI input CLK: %d KHz, STLink HCLK: %d KHz \n", (int)comInputClkKHz, (int)StlHClkKHz);

// EXAMPLE FOR SPI initialization: Brg::InitSPI() Brg::GetSPIbaudratePrescal()

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT brgStat = BRG_NO_ERR;
Brg_SpiInitT spiParam;
Brg_SpiBaudrateT prescal;
uint32_t reqFreqKHz = 6000;  // 6MHz
uint32_t currFreqKHz = 0;
uint32_t *pCurrFreqKHz = &currFreqKHz;
bool bBrgSpiBLnssPulse = false;
// SPI config: Full Duplex, 8-bit MSB, Speed; 6 MHz, Polarity: CPOL Low, CPHA ; Low, NSS software.
// SPI_CPHA_1EDGE SPI_CPOL_LOW; SPI_CRC_DISABLE; SPI_DATASIZE_8B; SPI_DIRECTION_2LINES_FULLDUPLEX;
// SPI_FIRSTBIT_MSB; SPI_FRF_MOTOROLA; SPI_MODE_MASTER; SPI_NSS_SOFT; SPI_NSS_NO_PULSE;

brgStat = m_pBrg->GetSPIbaudratePrescal(reqFreqKHz, &prescal, pCurrFreqKHz);
if (brgStat == BRG_COM_FREQ_MODIFIED) {
    brgStat = BRG_NO_ERR;
    printf("WARNING Bridge SPI init freq asked %d KHz but applied %d KHz \n", (int)reqFreqKHz, (int)*pCurrFreqKHz);
}
if (brgStat == BRG_NO_ERR) {
    spiParam.baudrate = prescal;
    spiParam.cpha = SPI_CPHA_1EDGE;
    spiParam.cpol = SPI_CPOL_LOW;
    spiParam.crc = SPI_CRC_DISABLE;
    spiParam.crcPoly = 0;
    spiParam.dataSize = SPI_DATASIZE_8B;
    spiParam.direction = SPI_DIRECTION_2LINES_FULLDUPLEX;
    spiParam.firstBit = SPI_FIRSTBIT_MSB;
    spiParam.frameFormat = SPI_FRF_MOTOROLA;
    spiParam.mode = SPI_MODE_MASTER;
    spiParam.nss = SPI_NSS_SOFT;
    spiParam.nssPulse = SPI_NSS_NO_PULSE;
    spiParam.spiDelay = DEFAULT_NO_DELAY;
    brgStat = m_pBrg->InitSPI(&spiParam);
    if ((brgStat == BRG_NO_ERR) && (pTestParam->spiInit.nss == SPI_NSS_SOFT) && (spiParam.cpol == SPI_CPOL_LOW)) {
        if (bBrgSpiBLnssPulse == true) {
            // for target detecting the falling edge of NSS, need to set it to HIGH before (to do pulse)
            brgStat = m_pBrg->SetSPIpinCS(SPI_NSS_HIGH);
        } else {
            // select target slave
            brgStat = m_pBrg->SetSPIpinCS(SPI_NSS_LOW);
        }
    }
}

// EXAMPLE FOR SPI read/write loopback, Brg::WriteSPI() Brg::ReadSPI()

Brg *m_pBrg;
**********[Missing init steps] * *********
                                         // SPI LOOPBACK
                                         // following protocol:
                                         // SPI configured in master mode,
                                         // 1- send 4 bytes = data size,
                                         // 2- wait to receive it back
                                         // 3- send data size bytes
                                         // 4- wait to receive same data size bytes back.
                                         // 5- restart to 1
                                         // Note: SPI target must be configured in slave mode to answer this protocol
                                         Brg_StatusT brgStat = BRG_NO_ERR;
uint8_t dataRx[3072], dataTx[3072];  // max size must be aligned with target buffer
uint16_t size;
InitTestBuffer(BUFF_0X00, dataRx, 3072);

size = 8;  // can be 1 - up to buffer size bytes
InitTestBuffer(BUFF_PATTERN1, dataTx, size);
brgStat = BrgRxTxVerifData(pTestParam, dataRx, dataTx, size);

// BrgRxTxVerifData function
Brg_StatusT BrgTest::BrgRxTxVerifData(Integ_TestParamT *pTestParam, uint8_t *pRxBuff, uint8_t *pTxBuff, uint16_t size) {
    Brg_StatusT brgStat = BRG_NO_ERR;
    int i, nb;
    uint16_t sizeWithoutErr = 0;
    uint32_t txSize, rxSize = 0;

    txSize = (uint32_t)size;

    // 1- send 4 bytes = data size,
    brgStat = m_pBrg->WriteSPI((uint8_t *)&txSize, 4, &sizeWithoutErr);
    if (brgStat != BRG_NO_ERR) {
        printf("BRG Write data size error (sent %d instead of 4)\n", (int)sizeWithoutErr);
    }
    // 2- wait to receive it back
    if (brgStat == BRG_NO_ERR) {
        sizeWithoutErr = 0;
        brgStat = m_pBrg->ReadSPI((uint8_t *)&rxSize, 4, &sizeWithoutErr);
        if (brgStat != BRG_NO_ERR) {
            printf("BRG Read back data size error (read %d instead of 4)\n", (int)sizeWithoutErr);
        } else {
            if (rxSize != txSize) {
                brgStat = BRG_VERIF_ERR;
                printf("BRG Read back RxSize = %d different from TxSize = %d \n", (int)rxSize, (int)txSize);
            }
        }
    }
    // 3- send data size bytes
    if (brgStat == BRG_NO_ERR) {
        sizeWithoutErr = 0;
        brgStat = m_pBrg->WriteSPI(pTxBuff, size, &sizeWithoutErr);
        if (brgStat != BRG_NO_ERR) {
            printf("BRG Write data error (sent %d instead of %d)\n", (int)sizeWithoutErr, (int)size);
        }
    }
    // 4- wait to receive same data size bytes back.
    if (brgStat == BRG_NO_ERR) {
        sizeWithoutErr = 0;
        brgStat = m_pBrg->ReadSPI(pRxBuff, size, &sizeWithoutErr);
        if (brgStat != BRG_NO_ERR) {
            printf("BRG Read back data error (read %d instead of %d)\n", (int)sizeWithoutErr, (int)size);
        } else {
            if (memcmp(pRxBuff, pTxBuff, size) != 0) {
                brgStat = BRG_VERIF_ERR;
                printf("BRG ERROR Read/Write verification error(s)\n");
            }
        }
    }
    return brgStat;
}

// EXAMPLE FOR SPI close, Brg::CloseBridge()</B>\n

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT brgStat = BRG_NO_ERR;

brgStat = m_pBrg->CloseBridge(COM_SPI);
