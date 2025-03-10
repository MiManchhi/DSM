/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *widgetCentral;
    QVBoxLayout *layoutCentral;
    QLabel *labFullscreen;
    QFrame *frmScreen;
    QGridLayout *layoutScreen;
    QFrame *frmVideo;
    QFrame *frmControl;
    QVBoxLayout *layoutControl;
    QHBoxLayout *layoutURL;
    QLabel *labURL;
    QLineEdit *editURL;
    QHBoxLayout *layoutProgress;
    QSlider *sliderProgress;
    QLabel *labProgress;
    QHBoxLayout *layoutButtons;
    QPushButton *btnPlay;
    QPushButton *btnPause;
    QPushButton *btnStop;
    QPushButton *btnFastForward;
    QPushButton *btnFastBackward;
    QSpacerItem *spacerVolume;
    QLabel *labVolume;
    QSlider *sliderVolume;
    QSpacerItem *spacerAbout;
    QPushButton *btnAbout;
    QPushButton *btnQuit;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(826, 609);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/images/QtPlayer.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        MainWindow->setWindowIcon(icon);
        widgetCentral = new QWidget(MainWindow);
        widgetCentral->setObjectName("widgetCentral");
        layoutCentral = new QVBoxLayout(widgetCentral);
        layoutCentral->setObjectName("layoutCentral");
        labFullscreen = new QLabel(widgetCentral);
        labFullscreen->setObjectName("labFullscreen");
        labFullscreen->setStyleSheet(QString::fromUtf8("color:red"));

        layoutCentral->addWidget(labFullscreen);

        frmScreen = new QFrame(widgetCentral);
        frmScreen->setObjectName("frmScreen");
        frmScreen->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frmScreen->sizePolicy().hasHeightForWidth());
        frmScreen->setSizePolicy(sizePolicy);
        frmScreen->setMinimumSize(QSize(0, 0));
        frmScreen->setMouseTracking(false);
        frmScreen->setToolTipDuration(-1);
        frmScreen->setFrameShape(QFrame::Box);
        frmScreen->setFrameShadow(QFrame::Raised);
        frmScreen->setLineWidth(2);
        layoutScreen = new QGridLayout(frmScreen);
        layoutScreen->setSpacing(0);
        layoutScreen->setObjectName("layoutScreen");
        layoutScreen->setContentsMargins(0, 0, 0, 0);
        frmVideo = new QFrame(frmScreen);
        frmVideo->setObjectName("frmVideo");
        frmVideo->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(frmVideo->sizePolicy().hasHeightForWidth());
        frmVideo->setSizePolicy(sizePolicy1);
        frmVideo->setMinimumSize(QSize(0, 64));
        frmVideo->setMouseTracking(false);
        frmVideo->setAutoFillBackground(false);
        frmVideo->setStyleSheet(QString::fromUtf8("border-image: url(:/images/QtPlayer.bmp);"));
        frmVideo->setFrameShape(QFrame::NoFrame);
        frmVideo->setFrameShadow(QFrame::Plain);
        frmVideo->setLineWidth(0);

        layoutScreen->addWidget(frmVideo, 0, 0, 1, 1);


        layoutCentral->addWidget(frmScreen);

        frmControl = new QFrame(widgetCentral);
        frmControl->setObjectName("frmControl");
        frmControl->setFrameShape(QFrame::Box);
        frmControl->setFrameShadow(QFrame::Sunken);
        layoutControl = new QVBoxLayout(frmControl);
        layoutControl->setObjectName("layoutControl");
        layoutURL = new QHBoxLayout();
        layoutURL->setObjectName("layoutURL");
        labURL = new QLabel(frmControl);
        labURL->setObjectName("labURL");

        layoutURL->addWidget(labURL);

        editURL = new QLineEdit(frmControl);
        editURL->setObjectName("editURL");
        editURL->setEnabled(false);
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(editURL->sizePolicy().hasHeightForWidth());
        editURL->setSizePolicy(sizePolicy2);
        editURL->setReadOnly(false);

        layoutURL->addWidget(editURL);


        layoutControl->addLayout(layoutURL);

        layoutProgress = new QHBoxLayout();
        layoutProgress->setObjectName("layoutProgress");
        sliderProgress = new QSlider(frmControl);
        sliderProgress->setObjectName("sliderProgress");
        sliderProgress->setEnabled(false);
        sliderProgress->setMaximum(10000);
        sliderProgress->setPageStep(100);
        sliderProgress->setOrientation(Qt::Horizontal);
        sliderProgress->setTickPosition(QSlider::TicksBelow);
        sliderProgress->setTickInterval(100);

        layoutProgress->addWidget(sliderProgress);

        labProgress = new QLabel(frmControl);
        labProgress->setObjectName("labProgress");
        labProgress->setStyleSheet(QString::fromUtf8("color:blue"));
        labProgress->setFrameShape(QFrame::Panel);
        labProgress->setFrameShadow(QFrame::Sunken);
        labProgress->setAlignment(Qt::AlignCenter);
        labProgress->setMargin(1);

        layoutProgress->addWidget(labProgress);


        layoutControl->addLayout(layoutProgress);

        layoutButtons = new QHBoxLayout();
        layoutButtons->setObjectName("layoutButtons");
        btnPlay = new QPushButton(frmControl);
        btnPlay->setObjectName("btnPlay");
        btnPlay->setEnabled(false);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/images/Play.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnPlay->setIcon(icon1);
        btnPlay->setFlat(false);

        layoutButtons->addWidget(btnPlay);

        btnPause = new QPushButton(frmControl);
        btnPause->setObjectName("btnPause");
        btnPause->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/images/Pause.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnPause->setIcon(icon2);

        layoutButtons->addWidget(btnPause);

        btnStop = new QPushButton(frmControl);
        btnStop->setObjectName("btnStop");
        btnStop->setEnabled(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/images/Stop.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnStop->setIcon(icon3);

        layoutButtons->addWidget(btnStop);

        btnFastForward = new QPushButton(frmControl);
        btnFastForward->setObjectName("btnFastForward");
        btnFastForward->setEnabled(false);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/images/FastForward.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnFastForward->setIcon(icon4);

        layoutButtons->addWidget(btnFastForward);

        btnFastBackward = new QPushButton(frmControl);
        btnFastBackward->setObjectName("btnFastBackward");
        btnFastBackward->setEnabled(false);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/images/FastBackward.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnFastBackward->setIcon(icon5);

        layoutButtons->addWidget(btnFastBackward);

        spacerVolume = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutButtons->addItem(spacerVolume);

        labVolume = new QLabel(frmControl);
        labVolume->setObjectName("labVolume");

        layoutButtons->addWidget(labVolume);

        sliderVolume = new QSlider(frmControl);
        sliderVolume->setObjectName("sliderVolume");
        sliderVolume->setEnabled(true);
        sliderVolume->setMinimumSize(QSize(64, 0));
        sliderVolume->setMaximum(100);
        sliderVolume->setValue(80);
        sliderVolume->setOrientation(Qt::Horizontal);
        sliderVolume->setTickPosition(QSlider::TicksBelow);
        sliderVolume->setTickInterval(10);

        layoutButtons->addWidget(sliderVolume);

        spacerAbout = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        layoutButtons->addItem(spacerAbout);

        btnAbout = new QPushButton(frmControl);
        btnAbout->setObjectName("btnAbout");
        btnAbout->setEnabled(true);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/images/About.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnAbout->setIcon(icon6);

        layoutButtons->addWidget(btnAbout);

        btnQuit = new QPushButton(frmControl);
        btnQuit->setObjectName("btnQuit");
        btnQuit->setEnabled(true);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/images/Quit.ico"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        btnQuit->setIcon(icon7);

        layoutButtons->addWidget(btnQuit);


        layoutControl->addLayout(layoutButtons);


        layoutCentral->addWidget(frmControl);

        MainWindow->setCentralWidget(widgetCentral);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\345\235\217\350\200\201\345\244\264\345\204\277\346\222\255\346\224\276\345\231\250", nullptr));
#if QT_CONFIG(tooltip)
        MainWindow->setToolTip(QCoreApplication::translate("MainWindow", "\345\235\217\350\200\201\345\244\264\345\204\277\346\222\255\346\224\276\345\231\250", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        labFullscreen->setToolTip(QCoreApplication::translate("MainWindow", "\345\205\250\345\261\217", nullptr));
#endif // QT_CONFIG(tooltip)
        labFullscreen->setText(QCoreApplication::translate("MainWindow", "\346\270\251\351\246\250\346\217\220\347\244\272\357\274\232\345\234\250\350\247\206\351\242\221\345\214\272\345\237\237\345\217\214\345\207\273\351\274\240\346\240\207\344\273\245\345\210\207\346\215\242\345\205\250\345\261\217\345\222\214\347\252\227\345\217\243\346\250\241\345\274\217\342\225\256(\342\225\257\311\233\342\225\260)\342\225\255", nullptr));
#if QT_CONFIG(tooltip)
        frmScreen->setToolTip(QCoreApplication::translate("MainWindow", "\350\247\206\351\242\221", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        frmVideo->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        frmControl->setToolTip(QCoreApplication::translate("MainWindow", "\346\216\247\345\210\266", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(accessibility)
        frmControl->setAccessibleName(QString());
#endif // QT_CONFIG(accessibility)
#if QT_CONFIG(tooltip)
        labURL->setToolTip(QCoreApplication::translate("MainWindow", "URL", nullptr));
#endif // QT_CONFIG(tooltip)
        labURL->setText(QCoreApplication::translate("MainWindow", "URL:", nullptr));
#if QT_CONFIG(tooltip)
        editURL->setToolTip(QCoreApplication::translate("MainWindow", "URL", nullptr));
#endif // QT_CONFIG(tooltip)
        editURL->setText(QCoreApplication::translate("MainWindow", "http://127.0.0.1:8080/files/", nullptr));
#if QT_CONFIG(tooltip)
        sliderProgress->setToolTip(QCoreApplication::translate("MainWindow", "\350\277\233\345\272\246", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        labProgress->setToolTip(QCoreApplication::translate("MainWindow", "\350\277\233\345\272\246", nullptr));
#endif // QT_CONFIG(tooltip)
        labProgress->setText(QCoreApplication::translate("MainWindow", "00:00:00.000/00:00:00.000", nullptr));
#if QT_CONFIG(tooltip)
        btnPlay->setToolTip(QCoreApplication::translate("MainWindow", "\346\222\255\346\224\276", nullptr));
#endif // QT_CONFIG(tooltip)
        btnPlay->setText(QCoreApplication::translate("MainWindow", "\346\222\255\346\224\276", nullptr));
#if QT_CONFIG(tooltip)
        btnPause->setToolTip(QCoreApplication::translate("MainWindow", "\346\232\202\345\201\234", nullptr));
#endif // QT_CONFIG(tooltip)
        btnPause->setText(QCoreApplication::translate("MainWindow", "\346\232\202\345\201\234", nullptr));
#if QT_CONFIG(tooltip)
        btnStop->setToolTip(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
#endif // QT_CONFIG(tooltip)
        btnStop->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
#if QT_CONFIG(tooltip)
        btnFastForward->setToolTip(QCoreApplication::translate("MainWindow", "\345\277\253\350\277\233", nullptr));
#endif // QT_CONFIG(tooltip)
        btnFastForward->setText(QCoreApplication::translate("MainWindow", "\345\277\253\350\277\233", nullptr));
#if QT_CONFIG(tooltip)
        btnFastBackward->setToolTip(QCoreApplication::translate("MainWindow", "\345\277\253\351\200\200", nullptr));
#endif // QT_CONFIG(tooltip)
        btnFastBackward->setText(QCoreApplication::translate("MainWindow", "\345\277\253\351\200\200", nullptr));
#if QT_CONFIG(tooltip)
        labVolume->setToolTip(QCoreApplication::translate("MainWindow", "\351\237\263\351\207\217", nullptr));
#endif // QT_CONFIG(tooltip)
        labVolume->setText(QCoreApplication::translate("MainWindow", "\351\237\263\351\207\217:", nullptr));
#if QT_CONFIG(tooltip)
        sliderVolume->setToolTip(QCoreApplication::translate("MainWindow", "\351\237\263\351\207\217", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        btnAbout->setToolTip(QCoreApplication::translate("MainWindow", "\345\205\263\344\272\216", nullptr));
#endif // QT_CONFIG(tooltip)
        btnAbout->setText(QCoreApplication::translate("MainWindow", "\345\205\263\344\272\216...", nullptr));
#if QT_CONFIG(tooltip)
        btnQuit->setToolTip(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272", nullptr));
#endif // QT_CONFIG(tooltip)
        btnQuit->setText(QCoreApplication::translate("MainWindow", "\351\200\200\345\207\272", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
