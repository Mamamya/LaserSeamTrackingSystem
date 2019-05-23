#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LaserTrackQt.h"
#include<QListView>
#include<QListWidget>
#include<QPushButton>
#include<QLabel>
#include<qtcpsocket.h>
#include<qudpsocket.h>
#include<QMessageBox>
#include<deque>
#include<Windows.h>

//该函数是用于比较第一个字节数组的子数组跟第二个字节数组是否相同
//输入：
//QByteArray a：为输入的第一个字节数组，其中部分内容用于跟第二个字节数组进行比较
//QByteArray b：为第二个字节数组
//int start：为第一个字节数组中子数组的索引开头
//int len：为第一个字节数组中子数组长度
//返回：
//若第一个字节数组的子数组与第二个数组相同则返回true,否则返回false；
bool Compare(QByteArray a, QByteArray b, int start, int len);

//用于保存坐标点偏差的类，便于利用队列进行缓存操作
class iterpos
{
public:
	float dx = 0;
	float dy = 0;
	float dz = 0;
	iterpos(float dx = 0, float dy = 0, float dz = 0)
	{
		this->dx = dx;
		this->dy = dy;
		this->dz = dz;
	}
};

//窗口类定义
class LaserTrackQt : public QMainWindow
{
	Q_OBJECT

public:
	LaserTrackQt(QWidget *parent = Q_NULLPTR);
        //函数为窗口关闭时所调用的事件函数，目的是关闭窗口时向传感器发送命令关闭传感器
	void closeEvent(QCloseEvent *e);

private:
	Ui::LaserTrackQtClass ui;
	//用于显示传感器所检测的xyz方向的偏差值
	QLabel *dxBox;
	QLabel *dyBox;
	QLabel *dzBox;
	//用于显示偏差值名字
	QLabel *dxLabel;
	QLabel *dyLabel;
	QLabel *dzLabel;
	//为连接传感器的按钮
	QPushButton *sensorConnectButton;
	//为连接机器人控制的按钮
	QPushButton *robotConnectButton;
	//为开始焊缝跟踪系统
	QPushButton *robotRunButton;
	//为点击sensorConnectButton按钮后所调用的事件函数，向IP：192.168.3.1，端口：59999
	//以UDP协议发送{0x00,0x00,0x03,0x00,0x01}以打开传感器，发送数据用char数组保存。
	void sensorConnectButtonEvent();
	//为点击robotConnectButton按钮后所调用的事件函数，函数内创建了一个QTcpSocket类，并连接
	//服务器，服务器IP为172.31.1.147，端口：8081
	void robotConnectButtonEvent();
	//为点击robotRunButton按钮后所调用的事件函数，函数主要功能为：
	//1.复位软件中用于计算机器人运动目标点的起始坐标，起始坐标为机器人HOME点坐标
	//2.将isRobotRun设为true，允许向机器人控制柜发送坐标数据
	void robotRunButtonEvent();

	//用于显示传感器是否已经连接成功
	QLabel *sensorState;
	//用于显示机器人是否已经连接成功
	QLabel *robotState;
	//用于与机器人控制柜通讯的QTcpSocket类
	QTcpSocket *opcClient;
	//用于与传感器通讯的QUdpSocket类
	QUdpSocket *sensorClient;
	//为与机器人控制柜的TCP服务器连接成功后所调用的事件函数，函数功能为：
	//1.将isRobotConnected设为true，并显示“机器人连接成功”
	//2.向控制柜服务器发送请求，获取机器人末端位置坐标
	void robotConnectedEvent();
	//为接收到来自控制柜数据后所调用的事件函数，函数功能为：
	//1.获得含有机器人末端坐标数据的以OPC标准为基础的XML数据，并从数据中提取出
	//机器人末端位置坐标。
	void robotReceivedEvent();
	//为接收到来自传感器数据后所调用的事件函数，函数功能为：
	//1.从获取的来自传感器的数据中提取出传感器检测的偏差数据
	//2.记录时间，若执行本次事件函数与上次记录事件差大于90ms，
	//则执行robotRun()函数。
	void sensorReceivedEvent();
	//被robotReceivedEvent()函数调用，用于向机器人控制柜发送机器人运动目标坐标数据
	void robotRun();
	//用于判断与控制柜服务器是否连接成功
	bool isRobotConnected = false;
	//用于判断与传感器是否连接成功
	bool isSensorConnected = false;
	//用于判断向控制柜发送数据是否被允许
	bool isRobotRun = false;
	//机器人运动速度，单位mm/s
	float velocity = 4;
	//偏差坐标缓冲队列长度
	int bufferLength = 300;
	//运动起始点坐标
	float homeX = 0;
	float homeY = 0;
	float homeZ = 0;
	//机器人运动目标坐标
	float posX = 0;
	float posY = 0;
	float posZ = 0;
	//机器人末端位置坐标
	float kx = 0;
	float ky = 0;
	float kz = 0;
	//运动偏差数据
	float dx = 0;
	float dy = 0;
	float dz = 0;
	//偏差数据缓冲队列
	std::deque<iterpos> ipos;
	//用于记录时间
	int frontTime = 0;
	int rearTime = 0;
};
