/**
 ******************************************************************************
 * @file    mfrc522_regs.h
 * @author  Ilmurat Ownuk
 * @brief   MFRC522 Register and Bit Mask Definitions
 *
 * @details
 * This file contains register addresses and bit mask definitions
 * for the MFRC522 RFID reader IC, based on the official datasheet.
 *
 * Designed for use with STM32 HAL and FreeRTOS-based embedded systems.
 *
 ******************************************************************************
 */
#ifndef __MFRC522_REGS_H 
#define __MFRC522_REGS_H

// ============================== REGISTERS ==================================
// Page 0: Command and Status
#define CommandReg                      0x01
#define ComIEnReg                       0x02
#define DivIEnReg                       0x03
#define ComIrqReg                       0x04
#define DivIrqReg                       0x05
#define ErrorReg                        0x06
#define Status1Reg                      0x07
#define Status2Reg                      0x08
#define FIFODataReg                     0x09
#define FIFOLevelReg                    0x0A
#define WaterLevelReg                   0x0B
#define ControlReg                      0x0C
#define BitFramingReg                   0x0D
#define CollReg                         0x0E

// Page 1: Communication
#define ModeReg                         0x11
#define TxModeReg                       0x12
#define RxModeReg                       0x13
#define TxControlReg                    0x14
#define TxASKReg                        0x15
#define TxSelReg                        0x16
#define RxSelReg                        0x17
#define RxThresholdReg                  0x18
#define DemodReg                        0x19
#define MfTxReg                         0x1C
#define MfRxReg                         0x1D
#define SerialSpeedReg                  0x1F

// Page 2: Configuration
#define CRCResultRegH                   0x21
#define CRCResultRegL                   0x22
#define ModWidthReg                     0x24
#define RFCfgReg                        0x26
#define GsNReg                          0x27
#define CWGsPReg                        0x28
#define ModGsPReg                       0x29
#define TModeReg                        0x2A
#define TPrescalerReg                   0x2B
#define TReloadRegH                     0x2C
#define TReloadRegL                     0x2D
#define TCounterValueRegH               0x2E
#define TCounterValueRegL               0x2F

// Page 3: Test Registers
#define TestSel1Reg                     0x31
#define TestSel2Reg                     0x32
#define TestPinEnReg                    0x33
#define TestPinValueReg                 0x34
#define TestBusReg                      0x35
#define AutoTestReg                     0x36
#define VersionReg                      0x37
#define AnalogTestReg                   0x38
#define TestDAC1Reg                     0x39
#define TestDAC2Reg                     0x3A
#define TestADCReg                      0x3B

// MFRC522 Commands
#define PCD_IDLE                        0x00
#define PCD_MEM                         0x01
#define PCD_GENERATE_RANDOM_ID          0x02
#define PCD_CALCCRC                     0x03
#define PCD_TRANSMIT                    0x04
#define PCD_NO_CMD_CHANGE               0x07
#define PCD_RECEIVE                     0x08
#define PCD_TRANSCEIVE                  0x0C
#define PCD_MFAUTHENT                   0x0E
#define PCD_SOFTRESET                   0x0F


// ============================== BIT MASKS ==================================
// Page 0: Command and Status 
// CommandReg (0x01) 
#define CommandReg_CommandMask          0x0F
#define CommandReg_PowerDown            0x10
#define CommandReg_RcvOff               0x20

// ComIEnReg (0x02) 
#define ComIEnReg_TimerIEn              0x01
#define ComIEnReg_ErrIEn                0x02
#define ComIEnReg_LoAlertIEn            0x04
#define ComIEnReg_HiAlertIEn            0x08
#define ComIEnReg_IdleIEn               0x10
#define ComIEnReg_RxIEn                 0x20
#define ComIEnReg_TxIEn                 0x40
#define ComIEnReg_IRQInv                0x80

// DivIEnReg (0x03) 
#define DivIEnReg_CRCIEn                0x04
#define DivIEnReg_MfinActIEn            0x10
#define DivIEnReg_IRQPushPull           0x80

// ComIrqReg (0x04) 
#define ComIrqReg_TimerIRq              0x01
#define ComIrqReg_ErrIRq                0x02
#define ComIrqReg_LoAlertIRq            0x04
#define ComIrqReg_HiAlertIRq            0x08
#define ComIrqReg_IdleIRq               0x10
#define ComIrqReg_RxIRq                 0x20
#define ComIrqReg_TxIRq                 0x40
#define ComIrqReg_Set1                  0x80

// DivIrqReg (0x05) 
#define DivIrqReg_CRCIRq                0x04
#define DivIrqReg_MfinActIRq            0x10
#define DivIrqReg_Set2                  0x80

// ErrorReg (0x06) 
#define ErrorReg_ProtocolErr            0x01
#define ErrorReg_ParityErr              0x02
#define ErrorReg_CRCErr                 0x04
#define ErrorReg_CollErr                0x08
#define ErrorReg_BufferOvfl             0x10
#define ErrorReg_TempErr                0x40
#define ErrorReg_WrErr                  0x80

// Status1Reg (0x07) 
#define Status1Reg_LoAlert              0x01
#define Status1Reg_HiAlert              0x02
#define Status1Reg_TRunning             0x08
#define Status1Reg_IRq                  0x10
#define Status1Reg_CRCReady             0x20
#define Status1Reg_CRCOk                0x40

// Status2Reg (0x08) 
#define Status2Reg_ModemStateMask       0x07
#define Status2Reg_MFCrypto1On          0x08
#define Status2Reg_IICForceHS           0x40
#define Status2Reg_TempSensClear        0x80

// FIFODataReg (0x09)
#define FIFODataReg_DataMask            0xFF

// FIFOLevelReg (0x0A) 
#define FIFOLevelReg_LevelMask          0x7F
#define FIFOLevelReg_FlushBuffer        0x80

// WaterLevelReg (0x0B) 
#define WaterLevelReg_LevelMask         0x3F

// ControlReg (0x0C) 
#define ControlReg_RxLastBitsMask       0x07
#define ControlReg_TStartNow            0x40
#define ControlReg_TStopNow             0x80

// BitFramingReg (0x0D) 
#define BitFramingReg_TxLastBitsMask    0x07
#define BitFramingReg_RxAlignMask       0x70
#define BitFramingReg_StartSend         0x80

// CollReg (0x0E) 
#define CollReg_CollPosMask             0x1F
#define CollReg_CollPosNotValid         0x20
#define CollReg_ValuesAfterColl         0x80

// Page 1: Communication
// ModeReg (0x11) 
#define ModeReg_CRCPresetMask           0x03
#define ModeReg_PolMFin                 0x08
#define ModeReg_TxWaitRF                0x20
#define ModeReg_MSBFirst                0x80

// TxModeReg (0x12) 
#define TxModeReg_InvMod                0x08
#define TxModeReg_TxSpeedMask           0x70
#define TxModeReg_TxCRCEnable           0x80

// RxModeReg (0x13) 
#define RxModeReg_RxMultiple            0x04
#define RxModeReg_RxNoErr               0x08
#define RxModeReg_RxSpeedMask           0x70
#define RxModeReg_RxCRCEnable           0x80

// TxControlReg (0x14) 
#define TxControlReg_Tx1RFEn            0x01
#define TxControlReg_Tx2RFEn            0x02
#define TxControlReg_Tx1CW              0x04
#define TxControlReg_Tx2CW              0x08
#define TxControlReg_InvTx1RFOff        0x10
#define TxControlReg_InvTx2RFOff        0x20
#define TxControlReg_InvTx1RFOn         0x40
#define TxControlReg_InvTx2RFOn         0x80

// TxASKReg (0x15) 
#define TxASKReg_Force100ASK            0x40

// TxSelReg (0x16) 
#define TxSelReg_MFOutSelMask           0x0F
#define TxSelReg_DriverSelMask          0x30

// RxSelReg (0x17) 
#define RxSelReg_RxWaitMask             0x3F
#define RxSelReg_UARTSelMask            0xC0

// RxThresholdReg (0x18)
#define RxThresholdReg_CollLevelMask    0x07
#define RxThresholdReg_MinLevelMask     0xF0

// DemodReg (0x19) 
#define DemodReg_TauSyncMask            0x03
#define DemodReg_TauRcvMask             0x0C
#define DemodReg_TPrescalEven           0x10
#define DemodReg_FixIQ                  0x20
#define DemodReg_AddIQMask              0xC0

// MfTxReg (0x1C)
#define MfTxReg_TxWaitMask              0x03

// MfRxReg (0x1D)
#define MfRxReg_ParityDisable           0x10

// SerialSpeedReg (0x1F)
#define SerialSpeedReg_BR_T1Mask        0x1F
#define SerialSpeedReg_BR_T0Mask        0xE0

// Page 2: Configuration
// CRCResultRegH (0x21)
#define CRCResultRegH_CRCResultMSBMask  0xFF

// CRCResultRegL (0x22) 
#define CRCResultRegL_CRCResultLSBMask  0xFF

// ModWidthReg (0x24)
#define ModWidthReg_ModWidthMask        0xFF

// RFCfgReg (0x26)
#define RFCfgReg_RxGainMask             0x70

// GsNReg (0x27)
#define GsNReg_ModGsNMask               0x0F
#define GsNReg_CWGsNMask                0xF0

// CWGsPReg (0x28)
#define CWGsPReg_CWGsPMask              0x3F

// ModGsPReg (0x29)
#define ModGsPReg_ModGsPMask            0x3F

// TModeReg (0x2A)
#define TModeReg_TPrescaler_HiMask      0x0F
#define TModeReg_TAutoRestart           0x10
#define TModeReg_TGatedMask             0x60
#define TModeReg_TAuto                  0x80

// TPrescalerReg (0x2B)
#define TPrescalerReg_TPrescaler_LoMask 0xFF

// TReloadRegH (0x2C)
#define TReloadRegH_TReloadVal_HiMask   0xFF

// TReloadRegL (0x2D)
#define TReloadRegL_TReloadVal_LoMask   0xFF

// TCounterValueRegH (0x2E)
#define TCounterValueRegH_TCounterVal_HiMask 0xFF

// TCounterValueRegL (0x2F)
#define TCounterValueRegL_TCounterVal_LoMask 0xFF


// Page 3: Test Registers
// TestSel1Reg (0x31)
#define TestSel1Reg_TstBusBitSelMask    0x07                     

// TestSel2Reg (0x32)
#define TestSel2Reg_TestBusSelMask      0x1F
#define TestSel2Reg_PRBS15              0x20
#define TestSel2Reg_PRBS9               0x40
#define TestSel2Reg_TstBusFlip          0x80

// TestPinEnReg (0x33)
#define TestPinEnReg_TestPinEnMask      0x7E
#define TestPinEnReg_RS232LineEn        0x80

// TestPinValueReg (0x34)
#define TestPinValueReg_TestPinValMask  0x7E
#define TestPinValueReg_UseIO           0x80

// TestBusReg (0x35)
#define TestBusReg_TestBusMask          0xFF

// AutoTestReg (0x36)
#define AutoTestReg_SelfTestMask        0x0F
#define AutoTestReg_AmpRcv              0x40

// VersionReg (0x37)
#define VersionReg_VersionMask          0xFF

// AnalogTestReg (0x38)
#define AnalogTestReg_AnalogSelAux2Mask 0x0F
#define AnalogTestReg_AnalogSelAux1Mask 0xF0

// TestDAC1Reg (0x39)
#define TestDAC1Reg_TestDAC1Mask        0x3F

// TestDAC2Reg (0x3A)
#define TestDAC2Reg_TestDAC2Mask        0x3F

// TestADCReg (0x3B)
#define TestADCReg_ADC_QMask            0x0F
#define TestADCReg_ADC_IMask            0xF0

#endif
