# Change Address of GP2Y0E03 Sensor Arduino Code

Arduino code for changing GP2Y0E03's address over I2C

This code is heavily based on [dbaba's](https://github.com/dbaba) version of code the changer I just ported it to run on the arduino enviroment, so all the credits to the main code go to him, please give him all the love for this over at his repo [HERE](https://github.com/dbaba/gp2y0e03-address-programmer)!


# How to do the modification

## Change the #defines to your match your setup
Open the code and go to the first lines to change the VPP pin and desired `new address data` for the sensor according to the following table:

### Possible NEW_ADDRESS data

This instructions were also written by [dbaba on his repo](https://github.com/dbaba/gp2y0e03-address-programmer) so go ther to checkout more!

Choose the `NEW_ADDRESS` on the  `Data` column value of the following table.

| Slave ID | 7-bit Address |   Data   |
|----------|---------------|----------|
| 0x00     | 0x00          | **0x00** |
| 0x10     | 0x08          | **0x01** |
| 0x20     | 0x10          | **0x02** |
| 0x30     | 0x18          | **0x03** |
| 0x40     | 0x20          | **0x04** |
| 0x50     | 0x28          | **0x05** |
| 0x60     | 0x30          | **0x06** |
| 0x70     | 0x38          | **0x07** |
| 0x80     | 0x40 DEFAULT! |   0x08   |
| 0x90     | 0x48          | **0x09** |
| 0xA0     | 0x50          | **0x0A** |
| 0xB0     | 0x58          | **0x0B** |
| 0xC0     | 0x60          | **0x0C** |
| 0xD0     | 0x68          | **0x0D** |
| 0xE0     | 0x70          | **0x0E** |
| 0xF0     | 0x78          | **DO NOT USE** |

Please do NOT choose `0x08` as the corresponding Salve ID is the default value.

Updating Slave ID to `0xF0` broke my sensors (I broke 3 sensors...). So I highly recommend you not to use the address `0xF0` (data for `0x0F`)

**Flash the code and open the Serial Monitor on baudrate 115200 to see the process**

If the monitor is displaying dot after dot that means that the operation is complete.