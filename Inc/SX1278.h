#ifndef _SX1278_H
#define _SX1278_H


#define LR_RegFifo                0x00

// Common settings
#define LR_RegOpMode              0x01
#define LR_RegFrMsb               0x06
#define LR_RegFrMid               0x07
#define LR_RegFrLsb               0x08

// Tx settings
#define LR_RegPaConfig            0x09
#define LR_RegPaRamp              0x0A
#define LR_RegOcp                 0x0B

// Rx settings
#define LR_RegLna                 0x0C

// LoRa registers
#define LR_RegFifoAddrPtr         0x0D
#define LR_RegFifoTxBaseAddr      0x0E
#define LR_RegFifoRxBaseAddr      0x0F
#define LR_RegFifoRxCurrentaddr   0x10
#define LR_RegIrqFlagsMask        0x11
#define LR_RegIrqFlags            0x12
#define LR_RegRxNbBytes           0x13
#define LR_RegRxHeaderCntValueMsb 0x14
#define LR_RegRxHeaderCntValueLsb 0x15
#define LR_RegRxPacketCntValueMsb 0x16
#define LR_RegRxPacketCntValueLsb 0x17
#define LR_RegModemStat           0x18
#define LR_RegPktSnrValue         0x19
#define LR_RegPktRssiValue        0x1A
#define LR_RegRssiValue           0x1B
#define LR_RegHopChannel          0x1C
#define LR_RegModemConfig1        0x1D
#define LR_RegModemConfig2        0x1E
#define LR_RegSymbTimeoutLsb      0x1F
#define LR_RegPreambleMsb         0x20
#define LR_RegPreambleLsb         0x21
#define LR_RegPayloadLength       0x22
#define LR_RegMaxPayloadLength    0x23
#define LR_RegHopPeriod           0x24
#define LR_RegFifoRxByteAddr      0x25
#define LR_RegFeiMsb              0x28
#define LR_RegFeiMid              0x29
#define LR_RegFeiLsb              0x2A


// I/O settings
#define REG_LR_DIOMAPPING1        0x40
#define REG_LR_DIOMAPPING2        0x41

// Version
#define REG_LR_VERSION            0x42

// Additional settings
#define REG_LR_PLLHOP             0x44
#define REG_LR_TCXO               0x4B
#define REG_LR_PADAC              0x4D
#define REG_LR_FORMERTEMP         0x5B
#define REG_LR_AGCREF             0x61
#define REG_LR_AGCTHRESH1         0x62
#define REG_LR_AGCTHRESH2         0x63
#define REG_LR_AGCTHRESH3         0x64

/********************FSK/ook mode***************************/
#define RegFIFO                   0x00 //FIFO
#define RegOpMode                 0x01 //Operation mode
#define RegBitRateMsb             0x02 //BR MSB
#define RegBitRateLsb             0x03 //BR LSB
#define RegFdevMsb                0x04 //FD MSB
#define RegFdevLsb                0x05 //FD LSB
#define RegFreqMsb                0x06 //Freq MSB
#define RegFreqMid                0x07 //Freq Middle byte
#define RegFreqLsb                0x08 //Freq LSB
#define RegPaConfig               0x09
#define RegPaRamp                 0x0a
#define RegOcp                    0x0b
#define RegLna                    0x0c
#define RegRxConfig               0x0d
#define RegRssiConfig             0x0e
#define RegRssiCollision          0x0f
#define RegRssiThresh             0x10
#define RegRssiValue              0x11
#define RegRxBw                   0x12
#define RegAfcBw                  0x13
#define RegOokPeak                0x14
#define RegOokFix                 0x15
#define RegOokAvg                 0x16
#define RegAfcFei                 0x1a
#define RegAfcMsb                 0x1b
#define RegAfcLsb                 0x1c
#define RegFeiMsb                 0x1d
#define RegFeiLsb                 0x1e
#define RegPreambleDetect         0x1f
#define RegRxTimeout1             0x20
#define RegRxTimeout2             0x21
#define RegRxTimeout3             0x22
#define RegRxDelay                0x23
#define RegOsc                    0x24 // SET OSC
#define RegPreambleMsb            0x25
#define RegPreambleLsb            0x26
#define RegSyncConfig             0x27
#define RegSyncValue1             0x28
#define RegSyncValue2             0x29
#define RegSyncValue3             0x2a
#define RegSyncValue4             0x2b
#define RegSyncValue5             0x2c
#define RegSyncValue6             0x2d
#define RegSyncValue7             0x2e
#define RegSyncValue8             0x2f
#define RegPacketConfig1          0x30
#define RegPacketConfig2          0x31
#define RegPayloadLength          0x32
#define RegNodeAdrs               0x33
#define RegBroadcastAdrs          0x34
#define RegFifoThresh             0x35
#define RegSeqConfig1             0x36
#define RegSeqConfig2             0x37
#define RegTimerResol             0x38
#define RegTimer1Coef             0x39
#define RegTimer2Coef             0x3a
#define RegImageCal               0x3b
#define RegTemp                   0x3c
#define RegLowBat                 0x3d
#define RegIrqFlags1              0x3e
#define RegIrqFlags2              0x3f
#define RegDioMapping1            0x40
#define RegDioMapping2            0x41
#define RegVersion                0x42
#define RegPllHop                 0x44
#define RegPaDac                  0x4d
#define RegBitRateFrac            0x5d

#define rgRadioIdle 0
#define rgRadioRx   1
#define rgRadioTx   2

extern ui8 RadioRegim;
extern ui8 RadioStatus;
extern ui8 RadioBuffer[128];
extern si8 RadioRxRSSI;
extern ui32 RadioTimeOut;

void RadioStandby(void);
void RadioSleep(void);
void RadioInit(void);

ui8 RadioRxOn(void);
ui8 RadioReadPacket(ui8 *RxData, ui8 maxLen);
si8 RadioRSSI(void);
ui8 RadioRxLen(void);

void RadioTxPacket(ui8 *txData, ui8 len);

void RadioLoRaClearIrq(void);
#endif //_SX1278_H
