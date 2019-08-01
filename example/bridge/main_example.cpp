/**
 ******************************************************************************
 * @file    main_example.cpp
 * @author  MCD Application Team
 * @brief   This module is an example, it can be integrated to a C++ console project
 *          to do a basic USB connection and  GPIO bridge test with the
 *          STLINK-V3SET probe, using the Bridge C++ open API.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */

#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include "bridge.h"

using namespace std;

/**
 * \brief Function to test the GPIO
 *
 * \param io_bridge Pointer on an initialized bridge object
 * \return true
 * \return false
 */
bool test_gpio(Brg* io_bridge) {
    Brg_StatusT bridge_status;
    uint8_t bridgeCom = COM_GPIO;
    Brg_GpioInitT gpio_parameters;
    Brg_GpioConfT gpio_configuration[BRG_GPIO_MAX_NB];
    Brg_GpioValT gpio_read_value[BRG_GPIO_MAX_NB];
    uint8_t gpio_selection_mask = 0, gpio_error_mask;

    // Check that the bridge is correctly initialized
    if (io_bridge == NULL) {
        printf("Bridge not initialized\r\n");
        return false;
    }

    // Configure all GPIO as inputs, with a pull up activated
    gpio_selection_mask = BRG_GPIO_ALL;
    gpio_parameters.GpioMask = gpio_selection_mask;
    gpio_parameters.ConfigNb = BRG_GPIO_MAX_NB;
    for (uint8_t i_gpio = 0; i_gpio < BRG_GPIO_MAX_NB; i_gpio++) {
        gpio_configuration[i_gpio].Mode = GPIO_MODE_INPUT;
        gpio_configuration[i_gpio].Speed = GPIO_SPEED_MEDIUM;
        gpio_configuration[i_gpio].Pull = GPIO_PULL_UP;
    }
    gpio_parameters.pGpioConf = &gpio_configuration[0];

    // Initialize the GPIO
    bridge_status = io_bridge->InitGPIO(&gpio_parameters);
    if (bridge_status != BRG_NO_ERR) {
        printf("Bridge Gpio init failed (mask=%d, gpio0: mode= %d, pull = %d, ...)\n", (int)gpio_parameters.GpioMask,
               (int)gpio_configuration[0].Mode, (int)gpio_configuration[0].Pull);
        return false;
    }

    // Try to read the values of the GPIO
    bridge_status = io_bridge->ReadGPIO(gpio_selection_mask, &gpio_read_value[0], &gpio_error_mask);
    if ((bridge_status != BRG_NO_ERR) || (gpio_error_mask != 0)) {
        printf(" Bridge Read error\n");
        return false;
    }

    // Verify that all GPIO have a value equal to one (because of the pull-up) (GPIO_SET = 1)
    for (uint8_t i = 0; i < BRG_GPIO_MAX_NB; i++) {
        if (gpio_read_value[i] != GPIO_SET) {
            bridge_status = BRG_VERIF_ERR;
            printf(" Bridge Read Verif error ( gpio %d != SET)\n", i);
        }
    }

    // If all GPIO had a value of 1, the test is a success
    if (bridge_status == BRG_NO_ERR) {
        return true;
        printf("GPIO Test1 OK \n");
    } else {
        return false;
    }
}

int main(int argc, char** argv) {
    Brg_StatusT brgStat = BRG_NO_ERR;
    STLinkIf_StatusT ifStat = STLINKIF_NO_ERR;
    char path[MAX_PATH];
    uint32_t i, numDevices;
    int firstDevNotInUse = -1;
    STLink_DeviceInfo2T devInfo2;
    Brg* m_pBrg = NULL;
    STLinkInterface* m_pStlinkIf = NULL;
    char m_serialNumber[SERIAL_NUM_STR_MAX_LEN];

    // USB interface initialization and device detection done using STLinkInterface
    // Create USB BRIDGE interface
    m_pStlinkIf = new STLinkInterface(STLINK_BRIDGE);
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

    // USB Connection to a given device done with Brg
    if (brgStat == BRG_NO_ERR) {
        m_pBrg = new Brg(*m_pStlinkIf);
    }

    // The firmware may not be the very last one, but it may be OK like that (just inform)
    bool bOldFirmwareWarning = false;

    // Open the STLink connection
    if (brgStat == BRG_NO_ERR) {
        m_pBrg->SetOpenModeExclusive(true);

        brgStat = m_pBrg->OpenStlink(firstDevNotInUse);

        if (brgStat == BRG_NOT_SUPPORTED) {
            brgStat = Brg::ConvSTLinkIfToBrgStatus(m_pStlinkIf->GetDeviceInfo2(0, &devInfo2, sizeof(devInfo2)));
            printf("BRIDGE not supported PID: 0X%04hx SN:%s\n", (unsigned short)devInfo2.ProductId, devInfo2.EnumUniqueId);
        }

        if (brgStat == BRG_OLD_FIRMWARE_WARNING) {
            // Status to restore at the end if all is OK
            bOldFirmwareWarning = true;
            brgStat = BRG_NO_ERR;
        }
    }
    // Test Voltage command
    if (brgStat == BRG_NO_ERR) {
        float voltage = 0;
        // T_VCC pin must be connected to target voltage on debug connector
        brgStat = m_pBrg->GetTargetVoltage(&voltage);
        if ((brgStat != BRG_NO_ERR) || (voltage == 0)) {
            printf(
                "BRIDGE get voltage error (check if T_VCC pin is connected to target voltage on "
                "debug connector)\n");
        } else {
            printf("BRIDGE get voltage: %f V \n", (double)voltage);
        }
    }

    if ((brgStat == BRG_NO_ERR) && (bOldFirmwareWarning == true)) {
        // brgStat = BRG_OLD_FIRMWARE_WARNING;
        printf("BRG_OLD_FIRMWARE_WARNING: v%d B%d \n", (int)m_pBrg->m_Version.Major_Ver, (int)m_pBrg->m_Version.Bridge_Ver);
    }

    // Fiddle GPIO
    if (brgStat == BRG_NO_ERR) {
        uint8_t bridgeCom = COM_GPIO;
        Brg_GpioInitT gpioParams;
        Brg_GpioConfT gpioConf[BRG_GPIO_MAX_NB];
        Brg_GpioValT gpioSetVal[BRG_GPIO_MAX_NB];
        uint8_t gpioMsk = 0, gpioErrMsk;
        int i;

        if (bridgeCom == COM_GPIO) {
            printf("Run BRIDGE GPIO test\n");
        } else {
            brgStat = BRG_NOT_SUPPORTED;
        }

        if (brgStat == BRG_NO_ERR) {
            gpioMsk = BRG_GPIO_ALL;
            gpioParams.GpioMask = gpioMsk;          // BRG_GPIO_0 1 2 3
            gpioParams.ConfigNb = BRG_GPIO_MAX_NB;  // must be BRG_GPIO_MAX_NB or 1 (if 1
                                                    // pGpioConf[0] used for all gpios)
            gpioParams.pGpioConf = &gpioConf[0];
            for (i = 0; i < BRG_GPIO_MAX_NB; i++) {
                gpioConf[i].Mode = GPIO_MODE_OUTPUT;
                gpioConf[i].Speed = GPIO_SPEED_HIGH;
                gpioConf[i].Pull = GPIO_NO_PULL;
                gpioConf[i].OutputType = GPIO_OUTPUT_PUSHPULL;
            }
            brgStat = m_pBrg->InitGPIO(&gpioParams);
            if (brgStat != BRG_NO_ERR) {
                printf("Bridge Gpio init failed (mask=%d, gpio0: mode= %d, pull = %d, ...)\n", (int)gpioParams.GpioMask,
                       (int)gpioConf[0].Mode, (int)gpioConf[0].Pull);
            }
        }
        if (brgStat == BRG_NO_ERR) {
            uint32_t loop_iteration = 0;
            while (loop_iteration < 10000) {
                for (i = 0; i < BRG_GPIO_MAX_NB; i++) {
                    gpioSetVal[i] = (gpioSetVal[i] == GPIO_SET ? GPIO_RESET : GPIO_SET);
                }

                brgStat = m_pBrg->SetResetGPIO(gpioMsk, &gpioSetVal[0], &gpioErrMsk);
                if ((brgStat != BRG_NO_ERR) || (gpioErrMsk != 0)) {
                    printf(" Bridge Read error\n");
                }
                usleep(10);
                loop_iteration++;
            }
        }

        if (brgStat == BRG_NO_ERR) {
            printf("GPIO Test1 OK \n");
        }
    }

    // Disconnect
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

    if (brgStat == BRG_NO_ERR) {
        printf("TEST SUCCESS \n");
    } else {
        printf("TEST FAIL (Bridge error: %d) \n", (int)brgStat);
    }

    return 0;
}
