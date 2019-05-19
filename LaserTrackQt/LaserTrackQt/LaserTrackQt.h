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

bool Compare(QByteArray a, QByteArray b, int start, int len);

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

class LaserTrackQt : public QMainWindow
{
	Q_OBJECT

public:
	LaserTrackQt(QWidget *parent = Q_NULLPTR);
	void closeEvent(QCloseEvent *e);

private:
	Ui::LaserTrackQtClass ui;
	QLabel *dxBox;
	QLabel *dyBox;
	QLabel *dzBox;
	QLabel *dxLabel;
	QLabel *dyLabel;
	QLabel *dzLabel;
	QPushButton *sensorConnectButton;
	QPushButton *robotConnectButton;
	QPushButton *robotRunButton;
	void sensorConnectButtonEvent();
	void robotConnectButtonEvent();
	void robotRunButtonEvent();

	QLabel *sensorState;
	QLabel *robotState;

	QTcpSocket *opcClient;
	QUdpSocket *sensorClient;
	void robotConnectedEvent();
	void robotReceivedEvent();
	void sensorReceivedEvent();
	void robotRun();
	bool isRobotConnected = false;
	bool isSensorConnected = false;
	bool isRobotRun = false;

	float velocity = 4;
	int bufferLength = 300;

	float homeX = 0;
	float homeY = 0;
	float homeZ = 0;
	float posX = 0;
	float posY = 0;
	float posZ = 0;
	float kx = 0;
	float ky = 0;
	float kz = 0;
	float dx = 0;
	float dy = 0;
	float dz = 0;

	std::deque<iterpos> ipos;
	int frontTime = 0;
	int rearTime = 0;
};