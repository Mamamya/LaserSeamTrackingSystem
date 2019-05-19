#include "LaserTrackQt.h"

LaserTrackQt::LaserTrackQt(QWidget *parent)
: QMainWindow(parent)
{
	ui.setupUi(this);
	this->setFixedWidth(600);
	this->setFixedHeight(400);
	this->dxBox = new QLabel(this);
	this->dyBox = new QLabel(this);
	this->dzBox = new QLabel(this);
	this->dxBox->setGeometry(100, 50, 100, 50);
	this->dyBox->setGeometry(100, 150, 100, 50);
	this->dzBox->setGeometry(100, 250, 100, 50);
	this->dxLabel = new QLabel(this);
	this->dyLabel = new QLabel(this);
	this->dzLabel = new QLabel(this);
	this->dxLabel->setGeometry(20, 50, 100, 50);
	this->dyLabel->setGeometry(20, 150, 100, 50);
	this->dzLabel->setGeometry(20, 250, 100, 50);
	QFont textFont;
	textFont.setPointSize(16);
	this->dxLabel->setFont(textFont);
	this->dxLabel->setText(QString::fromLocal8Bit("Δx"));
	this->dyLabel->setFont(textFont);
	this->dyLabel->setText(QString::fromLocal8Bit("Δy"));
	this->dzLabel->setFont(textFont);
	this->dzLabel->setText(QString::fromLocal8Bit("Δz"));
	this->sensorConnectButton = new QPushButton(this);
	this->robotConnectButton = new QPushButton(this);
	this->robotRunButton = new QPushButton(this);
	this->sensorConnectButton->setGeometry(300, 50, 120, 50);
	this->robotConnectButton->setGeometry(300, 150, 120, 50);
	this->robotRunButton->setGeometry(300, 250, 120, 50);
	this->sensorConnectButton->setFont(textFont);
	this->sensorConnectButton->setText(QString::fromLocal8Bit("连接传感器"));
	this->robotConnectButton->setFont(textFont);
	this->robotConnectButton->setText(QString::fromLocal8Bit("连接机器人"));
	this->robotRunButton->setFont(textFont);
	this->robotRunButton->setText(QString::fromLocal8Bit("开始运行"));
	QObject::connect(this->sensorConnectButton, &QPushButton::clicked, this, &LaserTrackQt::sensorConnectButtonEvent);
	QObject::connect(this->robotConnectButton, &QPushButton::clicked, this, &LaserTrackQt::robotConnectButtonEvent);
	QObject::connect(this->robotRunButton, &QPushButton::clicked, this, &LaserTrackQt::robotRunButtonEvent);

	this->sensorState = new QLabel(this);
	this->robotState = new QLabel(this);
	this->sensorState->setFont(textFont);
	this->robotState->setFont(textFont);
	this->sensorState->setText(QString::fromLocal8Bit("未连接传感器"));
	this->robotState->setText(QString::fromLocal8Bit("未连接机器人"));
	this->sensorState->setGeometry(20, 350, 150, 50);
	this->robotState->setGeometry(300, 350, 150, 50);

	this->opcClient = new QTcpSocket(this);
	QObject::connect(this->opcClient, &QTcpSocket::connected, this, &LaserTrackQt::robotConnectedEvent);
	QObject::connect(this->opcClient, &QTcpSocket::readyRead, this, &LaserTrackQt::robotReceivedEvent);
	this->sensorClient = new QUdpSocket(this);
	QObject::connect(this->sensorClient, &QUdpSocket::readyRead, this, &LaserTrackQt::sensorReceivedEvent);

}

void LaserTrackQt::sensorConnectButtonEvent()
{
	this->isSensorConnected = this->sensorClient->bind(59998, QUdpSocket::ShareAddress);
	if (this->isSensorConnected == true)
	{
		this->sensorState->setText(QString::fromLocal8Bit("传感器已连接"));
		char sendData[] = { 0, 0, 3, 0, 1 };
		this->sensorClient->writeDatagram(QByteArray::fromRawData(sendData, 5), QHostAddress("192.168.3.1"), 59999);
		this->frontTime = GetTickCount();
	}

}

void LaserTrackQt::robotConnectButtonEvent()
{
	this->opcClient->connectToHost(tr("172.31.1.147"), 8081);
}

void LaserTrackQt::robotRunButtonEvent()
{
	if (this->isRobotRun == false)
		this->isRobotRun = true;
	else
		this->isRobotRun = false;
	this->homeX = this->kx;
	this->homeY = this->ky;
	this->homeZ = this->kz;
	this->posX = this->kx;
	this->posY = this->ky;
	this->posZ = this->kz;
}

void LaserTrackQt::robotConnectedEvent()
{
	this->isRobotConnected = true;
	if (this->isRobotConnected == true)
	{
		this->robotState->setText(QString::fromLocal8Bit("机器人已连接"));
		std::string content = "<?xml version=\"1.0\" encoding=\"utf-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"http://opcfoundation.org/webservices/XMLDA/1.0/\"><soap:Body><Read><Options ReturnItemName=\"true\"/><ItemList MaxAge=\"0\"><Items ItemPath=\"\" ItemName=\"RobotVar.KX\"/><Items ItemPath=\"\" ItemName=\"RobotVar.KY\"/><Items ItemPath=\"\" ItemName=\"RobotVar.KZ\"/></ItemList></Read></soap:Body></soap:Envelope>";
		std::string enter = "\r\n\r\n";
		std::string head = "POST /DA HTTP/1.1\r\nUser-Agent: Softing OPC Toolbox\r\nHost: 172.31.1.147:8081\r\nDate: Wed, 26 Apr 2017 07:00:32 GMT\r\nConnection: Keep-Alive\r\nSOAPAction: \"http://opcfoundation.org/webservices/XMLDA/1.0/Read\"\r\nContent-Type: text/xml; charset=utf-8\r\nContent-Length: ";
		std::string write = head + std::to_string(content.size()) + enter + content;
		QString sendData = tr(write.c_str());
		this->opcClient->write(sendData.toUtf8().data());
	}
}

void LaserTrackQt::robotRun()
{
	if (this->isRobotConnected == true && this->isRobotRun == true)
	{
		iterpos tmpos;
		tmpos.dx = ipos[0].dx;
		tmpos.dy = 0.025 * ipos[0].dy;
		tmpos.dz = ipos[0].dz;
		this->dy = -(float)(this->velocity / 10 * tmpos.dy / sqrt(tmpos.dy*tmpos.dy + tmpos.dx*tmpos.dx));
		float step = this->velocity / 10;
		this->dx = sqrtf(step*step - this->dy*this->dy);
		posX += dx;
		posY += dy;
		std::string content = "<?xml version=\"1.0\" encoding=\"utf-8\"?><soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"http://opcfoundation.org/webservices/XMLDA/1.0/\"><soap:Body><Write ReturnValuesOnReply=\"false\"><Options/><ItemList><Items ItemPath=\"\" ItemName=\"RobotVar.KUKAX\"><Value xsi:type=\"xsd:float\">" + std::to_string(posX) + "</Value></Items><Items ItemPath=\"\" ItemName=\"RobotVar.KUKAY\"><Value xsi:type=\"xsd:float\">" + std::to_string(posY) + "</Value></Items><Items ItemPath=\"\" ItemName=\"RobotVar.KUKAZ\"><Value xsi:type=\"xsd:float\">" + std::to_string(posZ) + "</Value></Items><Items ItemPath=\"\" ItemName=\"RobotVar.KUKAVEL\"><Value xsi:type=\"xsd:float\">" + std::to_string(this->velocity) + "</Value></Items></ItemList></Write></soap:Body></soap:Envelope>";
		std::string enter = "\r\n\r\n";
		std::string head = "POST /DA HTTP/1.1\r\nUser-Agent: Softing OPC Toolbox\r\nHost: 172.31.1.147:8081\r\nDate: Thu, 6 Apr 2017 01:47:57 GMT\r\nConnection: Keep-Alive\r\nSOAPAction: \"http://opcfoundation.org/webservices/XMLDA/1.0/Write\"\r\nContent-Type: text/xml; charset=utf-8\r\nContent-Length: ";
		std::string write = head + std::to_string(content.size()) + enter + content;
		QString sendData = tr(write.c_str());
		this->opcClient->write(sendData.toUtf8().data());
	}
}

void LaserTrackQt::sensorReceivedEvent()
{
	QByteArray buffer;
	buffer.resize(this->sensorClient->pendingDatagramSize());
	this->sensorClient->readDatagram(buffer.data(), this->sensorClient->pendingDatagramSize());
	float x = buffer[9] * 256 + buffer[8];
	float y = buffer[11] * 256 + buffer[10];
	float z = buffer[13] * 256 + buffer[12];
	if (x >= 32768)
		x = x - 65536;
	if (y >= 32768)
		y = y - 65536;
	if (z >= 32768)
		z = z - 65536;
	x = x*0.01;
	y = y*0.01;
	z = z*0.01;
	this->dxBox->setText(tr(std::to_string(x).c_str()));
	this->dyBox->setText(tr(std::to_string(y).c_str()));
	this->dzBox->setText(tr(std::to_string(z).c_str()));

	this->rearTime = GetTickCount();
	if ((this->rearTime - this->frontTime) > 90)
	{
		this->ipos.push_back(iterpos(x, y, z));
		if (this->ipos.size() == 1)
		{
			this->ipos.resize(this->bufferLength);
			for (int i = 1; i < this->ipos.size(); i++)
			{
				ipos[i].dx = ipos[0].dx;
				ipos[i].dy = ipos[0].dy;
				ipos[i].dz = ipos[0].dz;
			}
		}
		this->ipos.pop_front();
		this->frontTime = this->rearTime;
		this->robotRun();
	}
}

void LaserTrackQt::robotReceivedEvent()
{
	QByteArray buffer;

	buffer = opcClient->readAll();
	int index = 0;
	if (!buffer.isEmpty())
	{
		QByteArray key = QByteArray::fromRawData("double", 6);
		for (int i = 0; i < buffer.size() - key.size(); i++)
		{
			if (Compare(buffer, key, i, key.size()))
			{
				int numLen = 0;
				while (buffer[i + numLen + 8] != '<')
					numLen++;
				char *number = new char[numLen + 1];
				for (int j = 0; j < numLen; j++)
					number[j] = buffer[i + j + 8];
				number[numLen] = '\0';
				switch (index)
				{
				case 0:
					this->kx = atof(number);
					break;
				case 1:
					this->ky = atof(number);
					break;
				case 2:
					this->kz = atof(number);
					break;
				default:
					break;
				}
				index++;
			}
		}
	}
}

void LaserTrackQt::closeEvent(QCloseEvent *e)
{
	if (this->isSensorConnected == true)
	{
		char sendData[] = { 0, 0, 0, 0, 0 };
		this->sensorClient->writeDatagram(QByteArray::fromRawData(sendData, 5), QHostAddress("192.168.3.1"), 59999);
	}
}

bool Compare(QByteArray a, QByteArray b, int start, int len)
{
	for (int i = start; i < start + len; i++)
	{
		if (a[i] != b[i - start])
			return false;
	}
	return true;
}
