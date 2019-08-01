
// WARNING: Instantiation, initialization steps are missing from the examples

// EXAMPLE FOR CAN input CLK, Brg::GetClk()

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT brgStat = BRG_NO_ERR;
uint32_t currFreqKHz = 0;
uint8_t com = COM_CAN;
uint32_t StlHClkKHz, comInputClkKHz;
// Get the current bridge input Clk
brgStat = m_pBrg->GetClk(com, &comInputClkKHz, &StlHClkKHz);
printf("CAN input CLK: %d KHz, STLink HCLK: %d KHz \n", (int)comInputClkKHz, (int)StlHClkKHz);

// EXAMPLE FOR CAN Initialization, Brg::InitCAN(), Brg::GetCANbaudratePrescal()

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT brgStat = BRG_NO_ERR;
Brg_CanInitT canParam;
uint32_t prescal;
uint32_t reqBaudrate = 125000;  // 125kbps
uint32_t finalBaudrate = 0;

// N=sync+prop+seg1+seg2= 1+2+7+6= 16, 125000 bps (-> prescal = 24 = (CanClk = 48MHz)/(16*125000))
canParam.BitTimeConf.propSegInTq = 2;
canParam.BitTimeConf.phaseSeg1InTq = 7;
canParam.BitTimeConf.phaseSeg2InTq = 6;
canParam.BitTimeConf.sjwInTq = 3;
brgStat = m_pBrg->GetCANbaudratePrescal(&canParam.BitTimeConf, reqBaudrate, (uint32_t *)&prescal, (uint32_t *)&finalBaudrate);
if (brgStat == BRG_COM_FREQ_MODIFIED) {
    printf("WARNING Bridge CAN init baudrate asked %d bps but applied %d bps \n", (int)reqBaudrate, (int)finalBaudrate);
} else if (brgStat == BRG_COM_FREQ_NOT_SUPPORTED) {
    printf("ERROR Bridge CAN init baudrate %d bps not possible (invalid prescaler: %d) change Bit Time or baudrate settings. \n",
           (int)reqBaudrate, (int)prescal);
} else if (brgStat == BRG_NO_ERR) {
    canParam.Prescaler = prescal;
    canParam.Mode = CAN_MODE_NORMAL;
    canParam.bIsTxfpEn = false;
    canParam.bIsRflmEn = false;
    canParam.bIsNartEn = false;
    canParam.bIsAwumEn = false;
    canParam.bIsAbomEn = false;
    brgStat = m_pBrg->InitCAN(&canParam, BRG_INIT_FULL);
}

// EXAMPLE FOR CAN loopback test, Brg::StartMsgReceptionCAN() Brg::InitFilterCAN() Brg::WriteMsgCAN() Brg::GetRxMsgNbCAN()
// Brg::GetRxMsgCAN()</B>\n

Brg *m_pBrg;
**********[Missing init steps] *
    *********
            // use init with canParam.Mode = CAN_MODE_LOOPBACK or connect a target that send back the received message

            Brg_StatusT brgStat = BRG_NO_ERR;
uint8_t dataRx[8], dataTx[8];
int i, nb;
Brg_CanFilterConfT filterConf;
Brg_CanRxMsgT canRxMsg;
Brg_CanTxMsgT canTxMsg;
uint8_t size = 0;
uint16_t msgNb = 0;

brgStat = m_pBrg->StartMsgReceptionCAN();
if (brgStat != BRG_NO_ERR) {
    printf("CAN StartMsgReceptionCAN failed \n");
}

// Loopback_test
// Receive all messages (no filter) with all DLC possible size (0->8)
// Filter0: CAN prepare receive (no filter: ID_MASK with Id =0 & Mask = 0) receive all in FIFO0
filterConf.AssignedFifo = CAN_MSG_RX_FIFO0;
filterConf.bIsFilterEn = true;
filterConf.FilterBankNb = 0;                 // 0 to 13
filterConf.FilterMode = CAN_FILTER_ID_MASK;  // CAN_FILTER_ID_LIST
filterConf.FilterScale = CAN_FILTER_16BIT;   // CAN_FILTER_32BIT
for (i = 0; i < 4; i++) {
    filterConf.Id[i].ID = 0;
    filterConf.Id[i].IDE = CAN_ID_STANDARD;
    filterConf.Id[i].RTR = CAN_DATA_FRAME;
}
for (i = 0; i < 2; i++) {
    filterConf.Mask[i].ID = 0;
    filterConf.Mask[i].IDE = CAN_ID_STANDARD;
    filterConf.Mask[i].RTR = CAN_DATA_FRAME;
}
if (brgStat == BRG_NO_ERR) {
    brgStat = m_pBrg->InitFilterCAN(&filterConf);
    if (brgStat != BRG_NO_ERR) {
        printf("CAN filter0 init failed \n");
    }
}

canRxMsg.ID = 0;
canRxMsg.IDE = CAN_ID_EXTENDED;  // must be = canTxMsg.IDE for the test
canRxMsg.RTR = CAN_DATA_FRAME;   // must be = canTxMsg.RTR for the test
canRxMsg.DLC = 0;

canTxMsg.ID = 0x12345678;  // must be <=0x7FF for CAN_ID_STANDARD, <=0x1FFFFFFF
canTxMsg.IDE = CAN_ID_EXTENDED;
canTxMsg.RTR = CAN_DATA_FRAME;
canTxMsg.DLC = 0;

nb = 0;
while ((brgStat == BRG_NO_ERR) && (nb < 255)) {
    for (i = 0; i < 8; i++) {
        dataRx[i] = 0;
        dataTx[i] = (uint8_t)(nb + i);
    }
    canRxMsg.DLC = 0;
    canTxMsg.DLC = 2;          // unused in CAN_DATA_FRAME
    size = (uint8_t)(nb % 9);  // try 0 to 8 DLC size

    if (nb == 200) {  // do it only once
        // REINIT command with same settings must be transparent and  must not break filter configuration
        brgStat = m_pBrg->InitCAN(&canParam, BRG_REINIT);
    }
    if (brgStat == BRG_NO_ERR) {
        brgStat = CanMsgTxRxVerif(&canTxMsg, dataTx, &canRxMsg, dataRx, CAN_MSG_RX_FIFO0, size);
    }
    nb++;
}
if (brgStat == BRG_NO_ERR) {
    printf(" Loopback_test1 OK \n");
}

// send a message and verify it is received and that TX = Rx
Brg_StatusT BrgTest::CanMsgTxRxVerif(Brg_CanTxMsgT *pCanTxMsg, uint8_t *pDataTx, Brg_CanRxMsgT *pCanRxMsg, uint8_t *pDataRx,
                                     Brg_CanRxFifoT rxFifo, uint8_t size) {
    Brg_StatusT brgStat = BRG_NO_ERR;
    uint16_t msgNb = 0;
    // Send message
    if (brgStat == BRG_NO_ERR) {
        brgStat = m_pBrg->WriteMsgCAN(pCanTxMsg, pDataTx, size);
        if (brgStat != BRG_NO_ERR) {
            printf("CAN Write Message error (Tx ID: 0x%08X)\n", (unsigned int)pCanTxMsg->ID);
        }
    }
    // Receive message
    if (brgStat == BRG_NO_ERR) {
        uint16_t dataSize;
        int retry = 100;
        while ((retry > 0) && (msgNb == 0)) {
            brgStat = m_pBrg->GetRxMsgNbCAN(&msgNb);
            retry--;
            Sleep(1);
        }
        if (msgNb == 0) {  // check if enough messages available
            brgStat = BRG_TARGET_CMD_TIMEOUT;
            printf("CAN Rx error (not enough msg available: 0/1)\n");
        }
        if (brgStat == BRG_NO_ERR) {  // read only 1 msg even if more available
            brgStat = m_pBrg->GetRxMsgCAN(pCanRxMsg, 1, pDataRx, 8, &dataSize);
        }
        if (brgStat != BRG_NO_ERR) {
            printf("CAN Read Message error (Tx ID: 0x%08X, nb of Rx msg available: %d)\n", (unsigned int)pCanTxMsg->ID, (int)msgNb);
        } else {
            if (pCanRxMsg->Fifo != rxFifo) {
                printf("CAN Read Message FIFO error (Tx ID: 0x%08X in FIFO%d instead of %d)\n", (unsigned int)pCanTxMsg->ID,
                       (int)pCanRxMsg->Fifo, (int)rxFifo);
                brgStat = BRG_VERIF_ERR;
            }
        }
    }
    // verif Rx = Tx
    if (brgStat == BRG_NO_ERR) {
        if ((pCanRxMsg->ID != pCanTxMsg->ID) || (pCanRxMsg->IDE != pCanTxMsg->IDE) || (pCanRxMsg->DLC != size) ||
            (pCanRxMsg->Overrun != CAN_RX_NO_OVERRUN)) {
            brgStat = BRG_CAN_ERR;
            printf("CAN ERROR ID Rx: 0x%08X Tx 0x%08X, IDE Rx %d Tx %d, DLC Rx %d size Tx %d\n", (unsigned int)pCanRxMsg->ID,
                   (unsigned int)pCanTxMsg->ID, (int)pCanRxMsg->IDE, (int)pCanTxMsg->IDE, (int)pCanRxMsg->DLC, (int)size);
        } else {
            for (int i = 0; i < size; i++) {
                if (pDataRx[i] != pDataTx[i]) {
                    printf("CAN ERROR data[%d] Rx: 0x%02hX Tx 0x%02hX \n", (int)i, (unsigned short)(unsigned char)pDataRx[i],
                           (unsigned short)(unsigned char)pDataTx[i]);
                    brgStat = BRG_VERIF_ERR;
                }
            }
        }
        if (brgStat != BRG_NO_ERR) {
            printf("CAN ERROR Read/Write verification \n");
        }
    }
    return brgStat;
}

// EXAMPLE FOR CAN close, Brg::CloseBridge()

Brg *m_pBrg;
**********[Missing init steps] * *********Brg_StatusT brgStat = BRG_NO_ERR;

brgStat = m_pBrg->CloseBridge(COM_CAN);
