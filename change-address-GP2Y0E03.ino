/*
   Code for changing address values on gp2y0e03 - Main source file
   25/02/2020 by Joao Pedro Vilas <joaopedrovbs@gmail.com>
   Changelog:
     2020-02-25 - initial release.
   Disclaimer:
    This code is heavily based on dbaba's version of code the changer I just ported it to run on
    the arduino enviroment, so all the credits to the main code go to him, please give him all 
    the love for this over at his repo https://github.com/dbaba/gp2y0e03-address-programmer

    AND FOLLOW HIS INSTRUCTIONS!

    Thanks https://github.com/dbaba
*/
/* ============================================================================================
 This code is placed under the MIT license
Copyright (c) 2020 João Pedro Vilas Boas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 ================================================================================================
 */
#include <Wire.h>

#define VPP_PIN     3
#define NEW_ADDRESS 0x01

#define wait_us     delayMicroseconds

static void _i2c_write(uint8_t register_address1, uint8_t data) {
    Wire.beginTransmission(0x80); // transmit to device #8
       Wire.write(register_address1);        // sends five bytes
       Wire.write(data);              // sends one byte
    Wire.endTransmission();    // stop transmitting
}

static uint8_t _i2c_read(uint8_t register_address) {
    
    uint8_t data;
    
    Wire.requestFrom(0x80, 1);    // request 2 bytes from slave device #112

    if (Wire.available()) { // if two bytes were received
      data = Wire.read();  // receive high byte (overwrites previous reading)
    }
    return data;
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 1
 * Data=0xFF is set in Address=0xEC.
 * 3.3V is applied in the Vpp terminal.
 */
static void e_fuse_stage1() {
    Serial.print("stage 1\r\n");
    _i2c_write(0xEC, 0xFF);
    digitalWrite(VPP_PIN, HIGH);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 2
 * Data=0x00 is set in Address=0xC8.
 */
static void e_fuse_stage2() {
    Serial.print("stage 2\r\n");
    _i2c_write(0xC8, 0x00);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 3
 * Data=0x45 is set in Address=0xC9.
 * + programming bit #: 5 => 5 - 1 = 4
 * + bank value: 5 => Bank E
 */
static void e_fuse_stage3() {
    Serial.print("stage 3\r\n");
    _i2c_write(0xC9, 0x45);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 4
 * Data=0x01 is set in Address=0xCD.
 * (Data=0x01 for slave address being changed to 0x10(write) and 0x11(read))
 * @param new_address 0-15 (Default address is 8, 0x80 for writing and 0x81 for reading)
 */
static void e_fuse_stage4(uint8_t new_address) {
    Serial.print("stage 4\r\n");
    _i2c_write(0xCD, new_address);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 5
 * Data=0x01 is set in Address=0xCA.
 * Wait for 500us.
 */
static void e_fuse_stage5() {
    Serial.print("stage 5\r\n");
    _i2c_write(0xCA, 0x01);
    wait_us(500);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 6
 * Data=0x00 is set in Address=0xCA.
 * Vpp terminal is grounded.
 */
static void e_fuse_stage6() {
    Serial.print("stage 6\r\n");
    _i2c_write(0xCA, 0x00);
    digitalWrite(VPP_PIN, LOW);  
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 7
 * Data=0x00 is set in Address=0xEF.
 * Data=0x40 is set in Address=0xC8.
 * Data=0x00 is set in Address=0xC8.
 */
static void e_fuse_stage7() {
    Serial.print("stage 7\r\n");
    _i2c_write(0xEF, 0x00);
    _i2c_write(0xC8, 0x40);
    _i2c_write(0xC8, 0x00);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 8
 * Data=0x06 is set in Address=0xEE.
 */
static void e_fuse_stage8() {
    Serial.print("stage 8\r\n");
    _i2c_write(0xEE, 0x06);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 9
 * Data=0xFF is set in Address=0xEC.
 * Data=0x03 is set in Address=0xEF.
 * Read out the data in Address=0x27.
 * Data=0x00 is set in Address=0xEF.
 * Data=0x7F is set in Address=0xEC.
 *
 * @return 0 for success, 1 for failure : 0x27[4:0] & 0b10000(0x10)
 */
static uint8_t e_fuse_stage9() {
    Serial.print("stage 9\r\n");
    // Table.20 List of E-Fuse program flow and setting value
    _i2c_write(0xEF, 0x00); // add this though it's missing in 12-6 Example of E-Fuse Programming
    _i2c_write(0xEC, 0xFF);
    _i2c_write(0xEF, 0x03);
    const uint8_t check_value = _i2c_read(0x27);
    const uint8_t check = check_value & 0x1f;
    Serial.print("Check 0x27[4:0] => ");
    Serial.println(check);
    const uint8_t success = check & 0x10;
    // When lower 5bits data[4:0] is 00001, E-Fuse program is finished.
    // When lower 5bits data[4:0] is not 00001, go to stage10(bit replacement).
    _i2c_write(0xEF, 0x00);
    _i2c_write(0xEC, 0x7F);
    // Check Result
    return success;
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-1
 * Data=0xFF is set in Address=0xEC.
 * 3.3V is applied in Vpp terminal.
 */
static void e_fuse_stage10_1_1() {
    Serial.print("stage 10-1-1\r\n");
    _i2c_write(0xEC, 0xFF);
    digitalWrite(VPP_PIN, HIGH);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-2
 * Data=0x37 is set in Address=0xC8.
 */
static void e_fuse_stage10_2_1() {
    Serial.print("stage 10-2-1\r\n");
    _i2c_write(0xC8, 0x37);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-3
 * Data=0x74 is set in Address=0xC9.
 */
static void e_fuse_stage10_3_1() {
    Serial.print("stage 10-3-1\r\n");
    _i2c_write(0xC9, 0x74);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-4
 * Data=0x04 is set in Address=0xCD.
 */
static void e_fuse_stage10_4_1() {
    Serial.print("stage 10-4-1\r\n");
    _i2c_write(0xCD, 0x04);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-5
 * Data=0x01 is set in Address=0xCA.
 * Wait for 500us.
 */
static void e_fuse_stage10_5_1() {
    Serial.print("stage 10-5-1\r\n");
    _i2c_write(0xCA, 0x01);
    wait_us(500);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-6
 * Data=0x00 is set in Address=0xCA.
 * Vpp terminal is grounded.
 */
static void e_fuse_stage10_6_1() {
    Serial.print("stage 10-6-1\r\n");
    _i2c_write(0xCA, 0x00);
    digitalWrite(VPP_PIN, LOW);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-1'
 * Data=0xFF is set in Address=0xEC.
 * 3.3V is applied in Vpp terminal.
 */
static void e_fuse_stage10_1_2() {
    Serial.print("stage 10-1-2\r\n");
    _i2c_write(0xEC, 0xFF);
    digitalWrite(VPP_PIN, HIGH);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-2'
 * Data=0x3F is set in Address=0xC8.
 */
static void e_fuse_stage10_2_2() {
    Serial.print("stage 10-2-2\r\n");
    _i2c_write(0xC8, 0x3F);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-3'
 * Data=0x04 is set in Address=0xC9.
 */
static void e_fuse_stage10_3_2() {
    Serial.print("stage 10-3-2\r\n");
    _i2c_write(0xC9, 0x04);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-4'
 * Data=0x01 is set in Address=0xCD.
 */
static void e_fuse_stage10_4_2() {
    Serial.print("stage 10-4-2\r\n");
    _i2c_write(0xCD, 0x01);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-5'
 * Data=0x01 is set in Address=0xCA.
 * Wait for 500us.
 */
static void e_fuse_stage10_5_2() {
    Serial.print("stage 10-5-2\r\n");
    _i2c_write(0xCA, 0x01);
    wait_us(500);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-6'
 * Data=0x00 is set in Address=0xCA.
 * Vpp terminal is grounded.
 */
static void e_fuse_stage10_6_2() {
    Serial.print("stage 10-6-2\r\n");
    _i2c_write(0xCA, 0x00);
    digitalWrite(VPP_PIN, LOW);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-7
 * Data=0x00 is set in Address=0xEF.
 * Data=0x40 is set in Address=0xC8.
 * Data=0x00 is set in Address=0xC8.
 */
static void e_fuse_stage10_7() {
    Serial.print("stage 10-7\r\n");
    _i2c_write(0xEF, 0x00);
    _i2c_write(0xC8, 0x40);
    _i2c_write(0xC8, 0x00);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-8
 * Data=0x06 is set in Address=0xEE.
 */
static void e_fuse_stage10_8() {
    Serial.print("stage 10-8\r\n");
    _i2c_write(0xEE, 0x06);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-9
 * Data=0xFF is set in Address=0xEC.
 * Data=0x03 is set in Address=0xEF.
 * Read out the data in Address=0x18 and Address=0x19.
 */
static void e_fuse_stage10_9() {
    Serial.print("stage 10-9\r\n");
    _i2c_write(0xEC, 0xFF);
    _i2c_write(0xEF, 0x03);
    const uint8_t bit_replacemnt_18 = _i2c_read(0x18);
    const uint8_t bit_replacemnt_19 = _i2c_read(0x19);
    Serial.print("Check 0x18 => ");
    Serial.println(bit_replacemnt_18);
    Serial.print("Check 0x19 => ");
    Serial.println(bit_replacemnt_19);
    
    if (bit_replacemnt_18 == 0x82 && bit_replacemnt_19 == 0x00) {
        Serial.print("Bit Replacement (stage 10) is SUCCESSFUL\r\n");
    } else {
        Serial.print("Bit Replacement (stage 10) is FAILURE\r\n");
    }
}

void e_fuse_run(uint8_t new_address) {

    Serial.println("// INITIALIZING CODE CHANGE ALGORITHM \\");
    delay(3000);
    
    if (new_address == 0x08) {
        Serial.print("[ERROR] The new address must be other than 0x08!\r\n");
        return;
    }
    if (new_address > 0x0f) {
        Serial.print("[ERROR] The new address must be 0x0f or lower!\r\n");
        return;
    }

    digitalWrite(VPP_PIN, LOW);
    wait_us(500);

    e_fuse_stage1();
    e_fuse_stage2();
    e_fuse_stage3();
    e_fuse_stage4(new_address);
    e_fuse_stage5();
    e_fuse_stage6();
    e_fuse_stage7();
    e_fuse_stage8();
    const uint8_t result = e_fuse_stage9();
    Serial.print("e_fuse_stage9():result => ");
    Serial.println(result);
    if (result) {
        e_fuse_stage10_1_1();
        e_fuse_stage10_2_1();
        e_fuse_stage10_3_1();
        e_fuse_stage10_4_1();
        e_fuse_stage10_5_1();
        e_fuse_stage10_6_1();

        e_fuse_stage10_1_2();
        e_fuse_stage10_2_2();
        e_fuse_stage10_3_2();
        e_fuse_stage10_4_2();
        e_fuse_stage10_5_2();
        e_fuse_stage10_6_2();

        e_fuse_stage10_7();
        e_fuse_stage10_8();
        e_fuse_stage10_9();
    }
}
,  
void setup() {
  //  Initialize Serial
  Serial.begin(115200);
  //  Connect Sensor Vpp(external pad, not present on the header/cable) to port 3
  pinMode(VPP_PIN, OUTPUT);
  
  Wire.begin(); // join i2c bus (address optional for master)

  // Set the address from 0x00 to 0x0F
  e_fuse_run(NEW_ADDRESS);
}

byte x = 0;

void loop() {
  Serial.print('.');
  delay(1);
}
