# Specs
* 10 MIDI buttons, 8 for control, 2 for shift/alt functions
* 10 RGB leds for status of buttons
* Rotary Encoder
* Main Display (SPI)
* 4 status displays in between buttons (I2C 1 & I2C 2 SSD13xx)

### DSP
* DSP sense pin for mode detection 
* Separate audio codec chips outside main 

```
 STM32 PHYISICAL PINOUT
COL1            -b12    GND-
COL2            -b13    GND-
COL3            -b14    3V3-
COL4            -b15    RST-
                -a8     b11-    ROW_ENC_M
UART TX         -a9     b10-    ROW_PAD_HIGH
UART RX         -a10     b1-    ROW_PAD_MID
ROW_MID         -a11     b0-    ROW_PAD_LOW
ROW_LOW         -a12     a7-    (MOSI) LED STRIP DIN
                -a15     a6-    
BAT_DIS         -b3      a5-    BAT_I_SENSE
DC_SENSE        -b4      a4-    BAT_V_SENSE
CHARGER_EN      -b5      a3-    ROW_ENC_4
OLED SCL        -b6      a2-    ROW_ENC_3
OLED SDA        -b7      a1-    ROW_ENC_2
SYS_EN          -b8      a0-    ROW_ENC_1
SYS_EN_SENSE    -b9     c15-
                -5V     c14-
                -GND    c13-    LED
                -3V3   vbat-
```