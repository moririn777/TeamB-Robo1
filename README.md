# B_Team_Robo1
Bチームのロボット1のコード
### PlatformIOを使用
### 使用ライブラリ
-  `Arduino.h`
-  `CAN.h` 
-  `PS4_Controller`

### データのフォーマット
```RPMdata```のフォーマット：

| Bits  | RPMデータ          |
|-------|------------------  |
| 63-48 | frontLeft (16bit)  |
| 47-32 | frontRight (16bit) |
| 31-16 | rearLeft (16bit)   |
| 15-0  | rearRight (16bit)  |

```TxData[8]```のフォーマット

| TxData Index | Bits  | データ      |
|--------------|-------|-------------|
| TxData[0]    | 63-56 | frontLeft上位8ビット  |
| TxData[1]    | 55-48 | frontLeft下位8ビット  |
| TxData[2]    | 47-40 | frontRight上位8ビット |
| TxData[3]    | 39-32 | frontRight下位8ビット |
| TxData[4]    | 31-24 | rearLeft上位8ビット   |
| TxData[5]    | 23-16 | rearLeft下位8ビット   |
| TxData[6]    | 15-8  | rearRight上位8ビット  |
| TxData[7]    | 7-0   | rearRight下位8ビット  |
