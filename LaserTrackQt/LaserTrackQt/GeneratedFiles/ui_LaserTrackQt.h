/********************************************************************************
** Form generated from reading UI file 'LaserTrackQt.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LASERTRACKQT_H
#define UI_LASERTRACKQT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LaserTrackQtClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *LaserTrackQtClass)
    {
        if (LaserTrackQtClass->objectName().isEmpty())
            LaserTrackQtClass->setObjectName(QStringLiteral("LaserTrackQtClass"));
        LaserTrackQtClass->resize(600, 400);
        menuBar = new QMenuBar(LaserTrackQtClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        LaserTrackQtClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(LaserTrackQtClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        LaserTrackQtClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(LaserTrackQtClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        LaserTrackQtClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(LaserTrackQtClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        LaserTrackQtClass->setStatusBar(statusBar);

        retranslateUi(LaserTrackQtClass);

        QMetaObject::connectSlotsByName(LaserTrackQtClass);
    } // setupUi

    void retranslateUi(QMainWindow *LaserTrackQtClass)
    {
        LaserTrackQtClass->setWindowTitle(QApplication::translate("LaserTrackQtClass", "LaserTrackQt", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class LaserTrackQtClass: public Ui_LaserTrackQtClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LASERTRACKQT_H
