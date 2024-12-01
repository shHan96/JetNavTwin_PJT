# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'form.ui'
##
## Created by: Qt User Interface Compiler version 6.8.0
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QFrame, QGridLayout, QHBoxLayout,
    QLabel, QLineEdit, QMainWindow, QMenuBar,
    QPlainTextEdit, QPushButton, QSizePolicy, QStatusBar,
    QTextEdit, QVBoxLayout, QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(1020, 710)
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.verticalLayout = QVBoxLayout(self.centralwidget)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setSpacing(7)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.horizontalLayout.setContentsMargins(-1, 0, -1, -1)
        self.startButton = QPushButton(self.centralwidget)
        self.startButton.setObjectName(u"startButton")
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Maximum, QSizePolicy.Policy.Maximum)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.startButton.sizePolicy().hasHeightForWidth())
        self.startButton.setSizePolicy(sizePolicy)
        self.startButton.setLayoutDirection(Qt.LayoutDirection.LeftToRight)
        icon = QIcon(QIcon.fromTheme(QIcon.ThemeIcon.MediaPlaybackStart))
        self.startButton.setIcon(icon)
        self.startButton.setIconSize(QSize(40, 40))

        self.horizontalLayout.addWidget(self.startButton)

        self.stopButton = QPushButton(self.centralwidget)
        self.stopButton.setObjectName(u"stopButton")
        sizePolicy1 = QSizePolicy(QSizePolicy.Policy.Maximum, QSizePolicy.Policy.Fixed)
        sizePolicy1.setHorizontalStretch(0)
        sizePolicy1.setVerticalStretch(0)
        sizePolicy1.setHeightForWidth(self.stopButton.sizePolicy().hasHeightForWidth())
        self.stopButton.setSizePolicy(sizePolicy1)
        font = QFont()
        font.setPointSize(20)
        self.stopButton.setFont(font)
        icon1 = QIcon(QIcon.fromTheme(QIcon.ThemeIcon.MediaPlaybackPause))
        self.stopButton.setIcon(icon1)
        self.stopButton.setIconSize(QSize(40, 40))

        self.horizontalLayout.addWidget(self.stopButton)

        self.initButton = QPushButton(self.centralwidget)
        self.initButton.setObjectName(u"initButton")
        sizePolicy1.setHeightForWidth(self.initButton.sizePolicy().hasHeightForWidth())
        self.initButton.setSizePolicy(sizePolicy1)
        self.initButton.setFont(font)
        icon2 = QIcon(QIcon.fromTheme(QIcon.ThemeIcon.SystemShutdown))
        self.initButton.setIcon(icon2)
        self.initButton.setIconSize(QSize(40, 40))

        self.horizontalLayout.addWidget(self.initButton)


        self.verticalLayout.addLayout(self.horizontalLayout)

        self.statuslbl = QLabel(self.centralwidget)
        self.statuslbl.setObjectName(u"statuslbl")
        self.statuslbl.setLayoutDirection(Qt.LayoutDirection.RightToLeft)
        self.statuslbl.setTextFormat(Qt.TextFormat.PlainText)
        self.statuslbl.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.verticalLayout.addWidget(self.statuslbl)

        self.gui_frame = QFrame(self.centralwidget)
        self.gui_frame.setObjectName(u"gui_frame")
        self.gui_frame.setFrameShape(QFrame.Shape.StyledPanel)
        self.gui_frame.setFrameShadow(QFrame.Shadow.Raised)
        self.gridLayout = QGridLayout(self.gui_frame)
        self.gridLayout.setObjectName(u"gridLayout")
        self.sensingText = QPlainTextEdit(self.gui_frame)
        self.sensingText.setObjectName(u"sensingText")
        sizePolicy2 = QSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)
        sizePolicy2.setHorizontalStretch(0)
        sizePolicy2.setVerticalStretch(0)
        sizePolicy2.setHeightForWidth(self.sensingText.sizePolicy().hasHeightForWidth())
        self.sensingText.setSizePolicy(sizePolicy2)
        self.sensingText.setReadOnly(True)

        self.gridLayout.addWidget(self.sensingText, 2, 1, 1, 2)

        self.label = QLabel(self.gui_frame)
        self.label.setObjectName(u"label")
        font1 = QFont()
        font1.setPointSize(9)
        self.label.setFont(font1)

        self.gridLayout.addWidget(self.label, 1, 0, 1, 1)

        self.label_4 = QLabel(self.gui_frame)
        self.label_4.setObjectName(u"label_4")
        self.label_4.setFont(font1)

        self.gridLayout.addWidget(self.label_4, 9, 0, 1, 2)

        self.promptSendButton = QPushButton(self.gui_frame)
        self.promptSendButton.setObjectName(u"promptSendButton")

        self.gridLayout.addWidget(self.promptSendButton, 3, 2, 1, 1)

        self.label_2 = QLabel(self.gui_frame)
        self.label_2.setObjectName(u"label_2")
        self.label_2.setFont(font1)

        self.gridLayout.addWidget(self.label_2, 1, 1, 1, 2)

        self.FirebaseSendButton = QPushButton(self.gui_frame)
        self.FirebaseSendButton.setObjectName(u"FirebaseSendButton")

        self.gridLayout.addWidget(self.FirebaseSendButton, 9, 2, 1, 1)

        self.logText = QTextEdit(self.gui_frame)
        self.logText.setObjectName(u"logText")
        self.logText.setReadOnly(True)

        self.gridLayout.addWidget(self.logText, 11, 1, 1, 2)

        self.promptOutput = QPlainTextEdit(self.gui_frame)
        self.promptOutput.setObjectName(u"promptOutput")
        sizePolicy2.setHeightForWidth(self.promptOutput.sizePolicy().hasHeightForWidth())
        self.promptOutput.setSizePolicy(sizePolicy2)
        self.promptOutput.setMinimumSize(QSize(450, 0))
        self.promptOutput.setReadOnly(True)

        self.gridLayout.addWidget(self.promptOutput, 5, 1, 1, 2)

        self.label_3 = QLabel(self.gui_frame)
        self.label_3.setObjectName(u"label_3")

        self.gridLayout.addWidget(self.label_3, 3, 1, 1, 1)

        self.promptInput = QLineEdit(self.gui_frame)
        self.promptInput.setObjectName(u"promptInput")

        self.gridLayout.addWidget(self.promptInput, 4, 1, 1, 2)

        self.commandText = QPlainTextEdit(self.gui_frame)
        self.commandText.setObjectName(u"commandText")
        self.commandText.setReadOnly(True)

        self.gridLayout.addWidget(self.commandText, 2, 0, 1, 1)

        self.gridLayout_2 = QGridLayout()
        self.gridLayout_2.setObjectName(u"gridLayout_2")
        self.backButton = QPushButton(self.gui_frame)
        self.backButton.setObjectName(u"backButton")
        sizePolicy3 = QSizePolicy(QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Minimum)
        sizePolicy3.setHorizontalStretch(0)
        sizePolicy3.setVerticalStretch(0)
        sizePolicy3.setHeightForWidth(self.backButton.sizePolicy().hasHeightForWidth())
        self.backButton.setSizePolicy(sizePolicy3)
        self.backButton.setFont(font)

        self.gridLayout_2.addWidget(self.backButton, 2, 1, 1, 1)

        self.midButton = QPushButton(self.gui_frame)
        self.midButton.setObjectName(u"midButton")
        sizePolicy3.setHeightForWidth(self.midButton.sizePolicy().hasHeightForWidth())
        self.midButton.setSizePolicy(sizePolicy3)
        self.midButton.setFont(font)

        self.gridLayout_2.addWidget(self.midButton, 1, 1, 1, 1)

        self.leftButton = QPushButton(self.gui_frame)
        self.leftButton.setObjectName(u"leftButton")
        sizePolicy3.setHeightForWidth(self.leftButton.sizePolicy().hasHeightForWidth())
        self.leftButton.setSizePolicy(sizePolicy3)
        self.leftButton.setFont(font)

        self.gridLayout_2.addWidget(self.leftButton, 1, 0, 1, 1)

        self.goButton = QPushButton(self.gui_frame)
        self.goButton.setObjectName(u"goButton")
        sizePolicy3.setHeightForWidth(self.goButton.sizePolicy().hasHeightForWidth())
        self.goButton.setSizePolicy(sizePolicy3)
        self.goButton.setFont(font)

        self.gridLayout_2.addWidget(self.goButton, 0, 1, 1, 1)

        self.rightButton = QPushButton(self.gui_frame)
        self.rightButton.setObjectName(u"rightButton")
        sizePolicy3.setHeightForWidth(self.rightButton.sizePolicy().hasHeightForWidth())
        self.rightButton.setSizePolicy(sizePolicy3)
        self.rightButton.setFont(font)

        self.gridLayout_2.addWidget(self.rightButton, 1, 2, 1, 1)


        self.gridLayout.addLayout(self.gridLayout_2, 3, 0, 3, 1)

        self.video_lbl = QLabel(self.gui_frame)
        self.video_lbl.setObjectName(u"video_lbl")
        sizePolicy2.setHeightForWidth(self.video_lbl.sizePolicy().hasHeightForWidth())
        self.video_lbl.setSizePolicy(sizePolicy2)
        self.video_lbl.setMaximumSize(QSize(240, 240))
        self.video_lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)

        self.gridLayout.addWidget(self.video_lbl, 10, 0, 2, 1)

        self.gridLayout.setColumnStretch(2, 1)

        self.verticalLayout.addWidget(self.gui_frame)

        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(MainWindow)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 1020, 26))
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(MainWindow)
        self.statusbar.setObjectName(u"statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        self.goButton.clicked.connect(MainWindow.go)
        self.leftButton.clicked.connect(MainWindow.left)
        self.rightButton.clicked.connect(MainWindow.right)
        self.midButton.clicked.connect(MainWindow.mid)
        self.backButton.clicked.connect(MainWindow.back)
        self.stopButton.clicked.connect(MainWindow.stop)
        self.initButton.clicked.connect(MainWindow.connect_agv)
        self.startButton.clicked.connect(MainWindow.start)
        self.FirebaseSendButton.clicked.connect(MainWindow.send_firebase)

        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"MainWindow", None))
        self.startButton.setText("")
        self.stopButton.setText("")
        self.initButton.setText("")
        self.statuslbl.setText(QCoreApplication.translate("MainWindow", u"Welcome!", None))
        self.label.setText(QCoreApplication.translate("MainWindow", u"command Table", None))
        self.label_4.setText(QCoreApplication.translate("MainWindow", u"log Table", None))
        self.promptSendButton.setText(QCoreApplication.translate("MainWindow", u"AI Help!", None))
        self.label_2.setText(QCoreApplication.translate("MainWindow", u"sensing Table", None))
        self.FirebaseSendButton.setText(QCoreApplication.translate("MainWindow", u"Send Firebase", None))
        self.label_3.setText(QCoreApplication.translate("MainWindow", u"openAI prompt", None))
        self.promptInput.setText("")
        self.backButton.setText(QCoreApplication.translate("MainWindow", u"Back", None))
        self.midButton.setText(QCoreApplication.translate("MainWindow", u"halt", None))
        self.leftButton.setText(QCoreApplication.translate("MainWindow", u"Left", None))
        self.goButton.setText(QCoreApplication.translate("MainWindow", u"Go", None))
        self.rightButton.setText(QCoreApplication.translate("MainWindow", u"Right", None))
        self.video_lbl.setText(QCoreApplication.translate("MainWindow", u"No signal...", None))
    # retranslateUi

