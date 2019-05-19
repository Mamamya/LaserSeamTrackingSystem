# 激光焊缝跟踪系统
## 1 系统说明
>激光焊缝跟踪系统是由：1、ScanSonic的TH6I型号的激光传感器；2、KUKA的KR16机械臂和KRC4控制柜；3、工控机  
>系统执行流程为：激光传感器=====焊接偏差数据=====>工控机=====发送控制信号=====>KRC4控制柜  
>工控机软件的开发环境为：  
>1.Visual Studio 2013 C++  
>2.Qt5.8.0
  
### 1.1 与激光传感器的通讯
>激光传感器内部有多种通讯方式，本系统的工控机采用基于UDP协议的以太网方式与传感器进行通讯。激光传感器是作为广播源将焊接偏差数据进行广播，工控机只需要
实例化QUdpSocket类，并监听59998端口即可。
>```cpp
>this->isSensorConnected = this->sensorClient->bind(59998, QUdpSocket::ShareAddress);
>```  
>在焊接偏差数据接收和处理上，是通过事件的异步接收方式进行。  
>将QUdpSocket的ReadyRead信号槽与处理函数void LaserTrackQt::sensorReceivedEvent()进行连接。
>```cpp
>QObject::connect(this->sensorClient, &QUdpSocket::readyRead, this, &LaserTrackQt::sensorReceivedEvent);
>```
>当工控机接收到激光传感器的广播数据时，就会执行void LaserTrackQt::sensorReceivedEvent()，用户可以在函数内对数据进行处理。  
>接收的数据包含传感器状态、关于xyz方向和abc角度的偏差值等数据。执行焊缝跟踪任务只需要获取关于xyz方向的焊接偏差数据即可。  
  
>数据格式如下：  
>xyz方向偏差为int16类型数据，x方向偏差处在数据字节数组的第9位、第10位，第10位为高8位，第9位为低8位。y方向偏差为11、12位，z方向偏差为13、14位。
计算得到的int16偏差数据若大于32768，则需要原数据-65536。最后，再除以100即为xyz方向的焊接偏差数据。  
  
>具体实现方式如下：
>```cpp
>QByteArray buffer;
>buffer.resize(this->sensorClient->pendingDatagramSize());
>this->sensorClient->readDatagram(buffer.data(), this->sensorClient->pendingDatagramSize());
>float x = buffer[9] * 256 + buffer[8];
>float y = buffer[11] * 256 + buffer[10];
>float z = buffer[13] * 256 + buffer[12];
>if (x >= 32768)
>	x = x - 65536;
>if (y >= 32768)
>	y = y - 65536;
>if (z >= 32768)
>	z = z - 65536;
>x = x*0.01;
>y = y*0.01;
>z = z*0.01;
>```  
  
### 1.2 与机器人控制柜的通讯
>机器人控制柜内置基于OPC标准的以太网通讯方式，用户可以根据OPC标准向KRC4控制柜发送和接收数据。  
>OPC标准有多种具体的实现形式，本系统采用OPC-Xml方式与控制柜进行通讯。OPC-Xml是以TCP协议为基础进行通讯的，因此，在具体实现上，需要实例化QTcpSocket类，并监听本地IP和端口。KRC4的IP为172.31.1.147，端口随意。 
连接成功事件和接收到数据事件。
>```cpp
>QObject::connect(this->opcClient, &QTcpSocket::connected, this, &LaserTrackQt::robotConnectedEvent);
>QObject::connect(this->opcClient, &QTcpSocket::readyRead, this, &LaserTrackQt::robotReceivedEvent);
>```  
>监听地址与端口。  
>```cpp
>this->opcClient->connectToHost(tr("172.31.1.147"), 8081);
>```  
