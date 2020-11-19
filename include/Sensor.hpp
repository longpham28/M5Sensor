// myIMU.h
#ifndef SENSOR_HPP

#define SENSOR_HPP
// myIMU.h
#include <math.h>
#include <M5Stack.h>
#include <Wire.h>

#define MPU6886_ADDRESS           0x68  //The slave address of the MPU-6886 is b110100X which is 7 bits long. 
#define MPU6886_WHOAMI            0x75

// 設定用
#define MPU6886_SMPLRT_DIV        0x19
#define MPU6886_CONFIG            0x1A
#define MPU6886_GYRO_CONFIG       0x1B
#define MPU6886_ACCEL_CONFIG      0x1C
#define MPU6886_ACCEL_CONFIG2     0x1D
#define MPU6886_INT_PIN_CFG       0x37
#define MPU6886_INT_ENABLE        0x38
#define MPU6886_ACCEL_INTEL_CTRL  0x69
#define MPU6886_PWR_MGMT_1        0x6B
#define MPU6886_PWR_MGMT_2        0x6C

// データ取得用
#define MPU6886_TEMP_DATA_START   0x41
#define MPU6886_ACCEL_DATA_START  0x3B
#define MPU6886_GYRO_DATA_START   0x43



class myIMU{
private:
  void write_byte(uint8_t addr, uint8_t data){
    Wire.beginTransmission(MPU6886_ADDRESS);
    Wire.write(addr);
    Wire.write(data);
    Wire.endTransmission();
  }


  //Gyro Full Scale Select: 00 = ±250 dps. 01= ±500 dps. 10 = ±1000 dps. 11 = ±2000 dps.
  //ACCEL_FS_SEL[1:0] Accel Full Scale Select:  ±2g (00), ±4g (01), ±8g (10), ±16g (11) 

  /* 
     ジャイロセンサの精度は、±250dps. ±500dps. ±1000dps. ±2000dps　の4種類で選択できる。
     変更方法はsetGyroScale関数を実行（0～3の値を指定）
     gScaleはI2Cで得られた16bitのデータをfloat型に変換するためのもの。
     例えば、±2000dpsに設定している場合、16bitのデータがb000 0000 0000 0000 → float=-2000に変換し、b1111 1111 1111 1111 → float=+2000に変換する
  */

  /* 
     加速度センサの精度は、±2g, ±4g, ±8g, ±16g の4種類で選択できる。
     変更方法はsetAccelScale関数を実行（0～3の値を指定）
     accScaleはI2Cで得られた16bitのデータをfloat型に変換するためのもの。
     例えば、±16gに設定している場合、16bitのデータがb000 0000 0000 0000 → float=-16.0に変換し、b1111 1111 1111 1111 → float=+16.0に変換する
  */
  float gScale, accScale;


public:
  /* 
  　　本当は、モジュール内部のデバイスとI2C通信する時にはWireではなくWire1を使うようなのだが、Wireでも動いたのでこのままにしてある
　 */
  void init(){
    Wire.begin();
    // 400 kHz Fast Mode I2C for communicating with all registers
    //I2Cのクロックの設定。無くても動いた
    // Wire.setClock(400000);

    //----------- Ensure that Accelerometer is running  --------------
    //In PWR_MGMT_1 register (0x6B) set CYCLE = 0, SLEEP = 0, and GYRO_STANDBY = 0 
    //0x6B(PWR_MGMT_1) DEVICE_RE_SET,  SLEEP,  CYCLE,  GYRO_STANDBY,  TEMP_DIS,  CLKSEL[2:0] 
    write_byte(MPU6886_PWR_MGMT_1, 0x81);  delay(10);  //b1000 0001 

    
    //In PWR_MGMT_2 register (0x6C) set STBY_XA = STBY_YA = STBY_ZA = 0, and STBY_XG = STBY_YG = STBY_ZG = 1 
    // 0x6C(PWR_MGMT_2) -, -,  STBY_XA STBY_YA STBY_ZA STBY_XG STBY_YG STBY_ZG 
    //write_byte(MPU6886_PWR_MGMT_2, 0x07);  delay(10);  //b00000111 


    // -----------  Set Accelerometer LPF bandwidth to 218.1 Hz   ----------- 
    //In ACEEL_CONFIG2 register (0x1D) set ACCEL_FCHOICE_B = 0 and A_DLPF_CFG[2:0] = 1 (b001) 
    // 0x1D(ACCEL_CONFIG_2)   -, -,  DEC2_CFG, DEC2_CFG, ACCEL_FCHOICE_B,  A_DLPF_CFG,  A_DLPF_CFG,  A_DLPF_CFG,  
    write_byte(MPU6886_ACCEL_CONFIG2, 0x01);  delay(10);  //b00000111 


    // -----------  Enable Motion Interrupt   ----------- 
    // In INT_ENABLE register (0x38) set WOM_INT_EN = 111 to enable motion interrupt 
    // 0x38 (INT_ENABLE) WOM_X_INT_EN,  WOM_Y_INT_EN , WOM_Z_INT_EN , FIFO _OFLOW_EN , - , GDRIVE_INT_EN,  -,  DATA_RDY_INT_EN 
    write_byte(MPU6886_INT_PIN_CFG,0x22);  delay(10);//追加
    write_byte(MPU6886_INT_ENABLE, 0xE0);   delay(10);



    // -----------  Enable Accelerometer Hardware Intelligence    ----------- 
    // In ACCEL_INTEL_CTRL register (0x69) set ACCEL_INTEL_EN = ACCEL_INTEL_MODE = 1; Ensure that bit 0 is set to 0 
    // 0x69 (ACCEL_INTEL_CTRL) ACCEL_INTEL_EN,  ACCEL_INTEL_MODE, -, -, -, -,  OUTPUT_LIMIT , WOM_TH_MO DE , 
    write_byte(MPU6886_ACCEL_INTEL_CTRL, 0xC0);   delay(10); //b1100 0000


    write_byte(MPU6886_CONFIG, 0x01);  delay(10); // 追加


    // -----------  Set Frequency of Wake-Up     ----------- 
    // In SMPLRT_DIV register (0x19) set SMPLRT_DIV[7:0] = 3.9 Hz – 500 Hz
    write_byte(MPU6886_SMPLRT_DIV, 0x05);  delay(10);

    
    // -----------　Enable Cycle Mode (Accelerometer Low-Power Mode) -----------
    //In PWR_MGMT_1 register (0x6B) set CYCLE = 1 
    //0x6B(PWR_MGMT_1) DEVICE_RE_SET,  SLEEP,  CYCLE,  GYRO_STANDBY,  TEMP_DIS,  CLKSEL[2:0] 
    write_byte(MPU6886_PWR_MGMT_1, 0x08); delay(10);

    setAccelScale(3); //16g
    setGyroScale(3);  //2000dps
  }


  //scale = 0 ~ 3,  0=±250dps,  1=±500dps. 2 = ±1000 dps. 3 = ±2000 dps.
  void setGyroScale(uint8_t scale){
    //gyro scale  -> [4:3] bitのFS_SEL[1:0] で設定
    switch(scale){
      case 0:  write_byte(MPU6886_GYRO_CONFIG, 0x15);  delay(10); gScale = 250.0 /32768.0; break; //b0000 0000
      case 1:  write_byte(MPU6886_GYRO_CONFIG, 0x16);  delay(10); gScale = 500.0 /32768.0; break; //b0000 1000
      case 2:  write_byte(MPU6886_GYRO_CONFIG, 0x17);  delay(10); gScale = 1000.0/32768.0; break; //b0001 0000
      case 3:  write_byte(MPU6886_GYRO_CONFIG, 0x18);  delay(10); gScale = 2000.0/32768.0; break; //b0001 1000

      default:
        M5.Lcd.print("[error] wrong gyro scale!");
        break;
    }
  }


  void setAccelScale(uint8_t scale){
    //accel scale -> [4:3] bitのACCEL_FS_SEL[1:0] で設定
    //ACCEL_FS_SEL[1:0] Accel Full Scale Select:  ±2g (00), ±4g (01), ±8g (10), ±16g (11) 
    switch(scale){
      case 0:  write_byte(MPU6886_ACCEL_CONFIG, 0x15);  delay(10); accScale = 2.0 /32768.0; break; //b0000 0000
      case 1:  write_byte(MPU6886_ACCEL_CONFIG, 0x16);  delay(10); accScale = 4.0 /32768.0; break; //b0000 1000
      case 2:  write_byte(MPU6886_ACCEL_CONFIG, 0x17);  delay(10); accScale = 8.0 /32768.0; break; //b0001 0000
      case 3:  write_byte(MPU6886_ACCEL_CONFIG, 0x18);  delay(10); accScale = 16.0/32768.0; break; //b0001 1000

      default:
        M5.Lcd.print("[error] wrong accel scale!");
        break;
    }
  }

  void getAccel(float *accX, float *accY, float *accZ){
    Wire.beginTransmission(MPU6886_ADDRESS);
    Wire.write(MPU6886_ACCEL_DATA_START);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6886_ADDRESS, 6, true);
    while (Wire.available() < 6);
    int16_t ax = Wire.read() << 8 | Wire.read();
    int16_t ay = Wire.read() << 8 | Wire.read();
    int16_t az = Wire.read() << 8 | Wire.read();

    *accX = (float)ax * accScale;
    *accY = (float)ay * accScale;
    *accZ = (float)az * accScale;
  }

  void getGyro(float *gytoX, float *gytoY, float *gytoZ){
    Wire.beginTransmission(MPU6886_ADDRESS);
    Wire.write(MPU6886_GYRO_DATA_START);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6886_ADDRESS, 6, true);
    while (Wire.available() < 6);
    int16_t gx = Wire.read() << 8 | Wire.read();
    int16_t gy = Wire.read() << 8 | Wire.read();
    int16_t gz = Wire.read() << 8 | Wire.read();

    *gytoX = (float)gx * gScale;
    *gytoY = (float)gy * gScale;
    *gytoZ = (float)gz * gScale;
  }

  void getTemperature(float *temp){
    Wire.beginTransmission(MPU6886_ADDRESS);
    Wire.write(MPU6886_TEMP_DATA_START);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6886_ADDRESS, 2, true);
    while (Wire.available() < 2);
    int16_t temp_data = Wire.read() << 8 | Wire.read();   
    // データシートP43より
    //High byte of the temperature sensor output 
    // TEMP_degC  = (TEMP_OUT[15:0]/Temp_Sensitivity) + RoomTemp_Offset 
    // where Temp_Sensitivity = 326.8 LSB/ºC and RoomTemp_Offset = 25ºC
    *temp = (float)temp_data / 326.8 + 25.0;
  }
};

#define RAD_TO_DEG 57.324

float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float pitch, roll, yaw;
float my_pitch, my_roll, my_yaw;
float Temp;

myIMU MPU6885;

void collectSensordata() {
    MPU6885.getGyro(&gyroX, &gyroY, &gyroZ);
    MPU6885.getAccel(&accX, &accY, &accZ);
}

#endif