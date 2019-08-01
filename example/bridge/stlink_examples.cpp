
// WARNING: Instantiation, initialization steps are missing from the examples

// EXAMPLE FOR Bridge devices detection and enumeration: STLinkInterface() STLinkInterface::LoadStlinkLibrary(),
// STLinkInterface::EnumDevices() STLinkInterface::GetDeviceInfo2()

Brg_StatusT brgStat = BRG_NO_ERR;
STLinkIf_StatusT ifStat = STLINKIF_NO_ERR;
char path[MAX_PATH];
#ifdef WIN32  // Defined for applications for Win32 and Win64.
char *pEndOfPrefix;
#endif
uint32_t i, numDevices;
int firstDevNotInUse = -1;
STLink_DeviceInfo2T devInfo2;
Brg *m_pBrg = NULL;
STLinkInterface *m_pStlinkIf = NULL;
char m_serialNumber[SERIAL_NUM_STR_MAX_LEN];

// Note: cErrLog g_ErrLog; to be instanciated and initialized if used with USING_ERRORLOG

// In case previously used, close the previous connection (not the case here)

// USB interface initialization and device detection done using STLinkInterface

// Create USB BRIDGE interface
m_pStlinkIf = new STLinkInterface(STLINK_BRIDGE);
#ifdef USING_ERRORLOG
m_pStlinkIf->BindErrLog(&g_ErrLog);
#endif

// Init path to STLinkUSBdriver library path
strcpy(path, "STLinkUSBdriver library path");  // to be updated

// Load STLinkUSBDriver library
ifStat = m_pStlinkIf->LoadStlinkLibrary(path);
if (ifStat != STLINKIF_NO_ERR) {
    printf("STLinkUSBDriver library (dll) issue \n");
}

if (ifStat == STLINKIF_NO_ERR) {
    ifStat = m_pStlinkIf->EnumDevices(&numDevices, false);
    // or brgStat = Brg::ConvSTLinkIfToBrgStatus(m_pStlinkIf->EnumDevices(&numDevices, false));
}

// Choose the first STLink Bridge available
if ((ifStat == STLINKIF_NO_ERR) || (ifStat == STLINKIF_PERMISSION_ERR)) {
    printf("%d BRIDGE device found\n", (int)numDevices);

    for (i = 0; i < numDevices; i++) {
        ifStat = m_pStlinkIf->GetDeviceInfo2(i, &devInfo2, sizeof(devInfo2));
        printf("Bridge %d PID: 0X%04hx SN:%s\n", (int)i, (unsigned short)devInfo2.ProductId, devInfo2.EnumUniqueId);

        if ((firstDevNotInUse == -1) && (devInfo2.DeviceUsed == false)) {
            firstDevNotInUse = i;
            memcpy(m_serialNumber, &devInfo2.EnumUniqueId, SERIAL_NUM_STR_MAX_LEN);
            printf(" SELECTED BRIDGE Stlink SN:%s\n", m_serialNumber);
        }
    }
} else if (ifStat == STLINKIF_CONNECT_ERR) {
    printf("No STLink BRIDGE device detected\n");
} else {
    printf("enum error\n");
}

brgStat = Brg::ConvSTLinkIfToBrgStatus(ifStat);

// EXAMPLE FOR Bridge STLink connection: Brg() StlinkDevice::SetOpenModeExclusive() Brg::OpenStlink() Brg::GetTargetVoltage()

STLinkInterface *m_pStlinkIf;
STLinkIf_StatusT ifStat ********* * [Missing init steps] **********Brg_StatusT brgStat = BRG_NO_ERR;
int firstDevNotInUse = 0;
STLink_DeviceInfo2T devInfo2;
Brg *m_pBrg = NULL;

// Note: cErrLog g_ErrLog; to be instanciated and initialized if used with USING_ERRORLOG

// See previous example for enumeration STLink index selection...
// USB interface: m_pStlinkIf must have been initialized, library loaded (see previous example)
// firstDevNotInUse point on a connected STLink having Bridge interface (see previous example)
brgStat = Brg::ConvSTLinkIfToBrgStatus(ifStat);

// USB Connection to a given device done with Brg
if (brgStat == BRG_NO_ERR) {
    m_pBrg = new Brg(*m_pStlinkIf);
#ifdef USING_ERRORLOG
    m_pBrg->BindErrLog(&g_ErrLog);
#endif
}

// Open the STLink connection
if (brgStat == BRG_NO_ERR) {
    m_pBrg->SetOpenModeExclusive(true);

    brgStat = m_pBrg->OpenStlink(firstDevNotInUse);

    if (brgStat == BRG_NOT_SUPPORTED) {
        brgStat = Brg::ConvSTLinkIfToBrgStatus(m_pStlinkIf->GetDeviceInfo2(0, &devInfo2, sizeof(devInfo2)));
        printf("BRIDGE not supported PID: 0X%04hx SN:%s\n", (unsigned short)devInfo2.ProductId, devInfo2.EnumUniqueId);
    }

    if (brgStat == BRG_OLD_FIRMWARE_WARNING) {
        brgStat = BRG_NO_ERR;
    }
}
// Test Voltage command
if (brgStat == BRG_NO_ERR) {
    float voltage = 0;
    // T_VCC pin must be connected to target voltage on debug connector
    brgStat = m_pBrg->GetTargetVoltage(&voltage);
    if ((brgStat != BRG_NO_ERR) || (voltage == 0)) {
        printf("BRIDGE get voltage error (check if T_VCC pin is connected to target voltage on debug connector)\n");
    } else {
        printf("BRIDGE get voltage: %f V \n", (double)voltage);
    }
}

// EXAMPLE FOR Bridge STLink Deconnection: Brg::CloseBridge() Brg::CloseStlink()

Brg *m_pBrg;
STLinkInterface *m_pStlinkIf;
**********[Missing init steps] * *********

                                         if (m_pBrg != NULL) {
    m_pBrg->CloseBridge(COM_UNDEF_ALL);
    m_pBrg->CloseStlink();
    delete m_pBrg;
    m_pBrg = NULL;
}
// unload STLinkUSBdriver library
if (m_pStlinkIf != NULL) {
    // always delete STLinkInterface after Brg (because Brg uses STLinkInterface)
    delete m_pStlinkIf;
    m_pStlinkIf = NULL;
}
