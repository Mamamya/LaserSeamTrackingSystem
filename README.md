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
>从控制柜接收数据是为了获取当前机械臂末端位置，并初始化程序原点坐标。接收数据的处理由void LaserTrackQt::robotReceivedEvent()执行。  
>处理完激光传感器数据后，需要向控制柜发送控制信号控制机械臂运动。由于激光条纹与焊枪尖端存在一定距离，控制上属于超前控制，本系统采用缓冲队列来解决超前
控制问题。  
>```cpp
>std::deque<iterpos> ipos;
>this->ipos.resize(this->bufferLength);
>```  
>工控机向控制柜发送控制信号的周期为90ms，并将坐标数据与队列第一位数据相加发送给控制柜。
>以下为周期控制和缓冲队列数据的进出：  
>```cpp
>this->rearTime = GetTickCount();
>	if ((this->rearTime - this->frontTime) > 90)
>	{
>		this->ipos.push_back(iterpos(x, y, z));
>		if (this->ipos.size() == 1)
>		{
>			this->ipos.resize(this->bufferLength);
>			for (int i = 1; i < this->ipos.size(); i++)
>			{
>				ipos[i].dx = ipos[0].dx;
>				ipos[i].dy = ipos[0].dy;
>				ipos[i].dz = ipos[0].dz;
>			}
>		}
>		this->ipos.pop_front();
>		this->frontTime = this->rearTime;
>		this->robotRun();
>	}
>```  
以下为向控制柜发送控制信号：  
>```cpp
>void LaserTrackQt::robotRun()
>{
>	if (this->isRobotConnected == true && this->isRobotRun == true)
>	{
>		iterpos tmpos;
>		tmpos.dx = ipos[0].dx;
>		tmpos.dy = 0.025 * ipos[0].dy;
>		tmpos.dz = ipos[0].dz;
>		this->dy = -(float)(this->velocity / 10 * tmpos.dy / sqrt(tmpos.dy*tmpos.dy + tmpos.dx*tmpos.dx));
>		float step = this->velocity / 10;
>		this->dx = sqrtf(step*step - this->dy*this->dy);
>		posX += dx;
>		posY += dy;
>		std::string content = "<?xml version=\"1.0\" encoding=\"utf-8\"?><soap:Envelope >xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" >xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"http://opcfoundation.org/webservices/XMLDA/1.0/\"><soap:Body><Write >ReturnValuesOnReply=\"false\"><Options/><ItemList><Items ItemPath=\"\" ItemName=\"RobotVar.KUKAX\"><Value xsi:type=\"xsd:float\">" + >std::to_string(posX) + "</Value></Items><Items ItemPath=\"\" ItemName=\"RobotVar.KUKAY\"><Value xsi:type=\"xsd:float\">" + >std::to_string(posY) + "</Value></Items><Items ItemPath=\"\" ItemName=\"RobotVar.KUKAZ\"><Value xsi:type=\"xsd:float\">" + >std::to_string(posZ) + "</Value></Items><Items ItemPath=\"\" ItemName=\"RobotVar.KUKAVEL\"><Value xsi:type=\"xsd:float\">" + >std::to_string(this->velocity) + "</Value></Items></ItemList></Write></soap:Body></soap:Envelope>";
>		std::string enter = "\r\n\r\n";
>		std::string head = "POST /DA HTTP/1.1\r\nUser-Agent: Softing OPC Toolbox\r\nHost: 172.31.1.147:8081\r\nDate: Thu, 6 Apr 2017 >01:47:57 GMT\r\nConnection: Keep-Alive\r\nSOAPAction: \"http://opcfoundation.org/webservices/XMLDA/1.0/Write\"\r\nContent-Type: ??>text/xml; charset=utf-8\r\nContent-Length: ";
>		std::string write = head + std::to_string(content.size()) + enter + content;
>		QString sendData = tr(write.c_str());
>		this->opcClient->write(sendData.toUtf8().data());
>	}
>}
>```  
