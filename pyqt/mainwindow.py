import sys

from Base64ImageProcessor import Base64ImageProcessor
from FirebaseManager import FirebaseManager
from AWSManager import AWSManager
from OpenAIManager import OpenAIManager

from PySide6.QtWidgets import QApplication, QMainWindow
from PySide6.QtCore import QTimer, QThread, Signal
import random as rd

from ui_form import Ui_MainWindow
from datetime import datetime
import pytz
from dotenv import load_dotenv
import os

# for MQTT
import paho.mqtt.client as mqtt

# Load environment variables from .env file
load_dotenv()

#Broker IP Address 와 Port
address = os.getenv("MQTT_ADDRESS", "localhost")
port = int(os.getenv("MQTT_PORT", 1883))

#MQTT command Topic
commandTopic = "AGV/command"

#MQTT log Topic
logTopic = "AGV/log"

#MQTT sensor Topic
sensingTopic = "AGV/sensing"

#MATT camera Topic
cameraTopic = "AGV/camera"

DEFAULT_QOS = 1

korea_timezone = pytz.timezone("Asia/Seoul")

class MainWindow(QMainWindow):
    base64ImageSignal = Signal(str)

    #MQTT로 들어온 log를 받아줄 list 생성
    logData = list()
    #logData 중 최신 15개 data만 저장할 list
    logDataList = list()

    #MQTT로 들어온 data를 받아줄 list 생성
    sensorData = list()
    #sensorData 중 최신 15개 data만 저장할 list
    sensingDataList = list()

    #MQTT로 보낼 command dict
    commandData = dict()
    #commandData 전체
    commandDataList = list()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.init()

    def init(self):
        self.firebaseManager = FirebaseManager()
        self.firebaseManager.statusSignal.connect(self.setStatusLbl)

        self.firebase_timer = QTimer(self)
        self.firebase_timer.timeout.connect(self.check_firebase_data)
        self.firebase_timer.start(1000)  # 1초마다 실행

        self.videoThread = Base64ImageProcessor()
        self.videoThread.image_signal.connect(self.show_camera)
        self.base64ImageSignal.connect(self.videoThread.update_image)
        self.ui.gui_frame.hide()

        self.awsManager = AWSManager()
        self.awsManager.connected_signal.connect(self.on_aws_connection)
        self.awsManager.error_signal.connect(self.on_aws_error)

        self.openaiManager = OpenAIManager()
        self.openaiManager.responseSignal.connect(self.handle_ai_response)
        self.openaiManager.errorSignal.connect(self.handle_ai_error)
        self.openaiManager.thinkingSignal.connect(self.show_thinking)
        self.ui.promptSendButton.clicked.connect(self.send_prompt)

        print('init')

    def check_firebase_data(self):
        # 현재 시간의 sensing 데이터를 Firebase로 전송
        if hasattr(self, 'latest_sensing_data'):
            self.firebaseManager.store_sensing_data(self.latest_sensing_data)


    def connect_agv(self):
        # Firestore 연결
        self.firebaseManager.connectFireBase()

        # MQTT 클라이언트 생성
        self.client = mqtt.Client(callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
        # 연결 시 콜백 함수 설정
        self.client.on_connect = self.on_connect
        # 메시지 수신 시 콜백 함수 설정
        self.client.message_callback_add(logTopic, self.on_message_log)
        self.client.message_callback_add(sensingTopic, self.on_message_sensing)
        self.client.message_callback_add(cameraTopic, self.on_message_camera)

        # Broker IP, port 연결
        self.client.connect(address, port)
        self.client.subscribe("AGV/#", qos=DEFAULT_QOS)
        self.client.loop_start()

        self.awsManager.start()

        # QTimer 설정 (0.5초마다 settingUI 메서드 호출)
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.settingUI)
        self.timer.start(500)  # 1000ms = 1초
        print('connect start')

    def setStatusLbl(self, msg):
        self.ui.statuslbl.setText(msg);

    def makeCommandData(self, str, arg, finish):
        current_time = datetime.now(korea_timezone)
        self.commandData = f'{current_time.strftime("%Y-%m-%d %H:%M:%S")};[command];{str}'
        return self.commandData

    def start(self):
        self.commandData = self.makeCommandData("start", 100, 1)
        self.client.publish(commandTopic, "start", qos=DEFAULT_QOS)
        self.commandDataList.append(self.commandData)

    def stop(self):
        self.commandData = self.makeCommandData("stop", 100, 1)
        self.client.publish(commandTopic, "stop", qos=DEFAULT_QOS)
        self.commandDataList.append("stop")

    def settingUI(self):
        self.ui.commandText.clear()
        for i in range(len(self.commandDataList)):
            self.commandData = self.commandDataList[i].split(";")
            msg = ""
            for j in range(len(self.commandData)):
                if j != 0: msg += " | "
                msg += self.commandData[j]

            self.ui.commandText.appendPlainText(msg)

        self.ui.sensingText.clear()
        for i in range(len(self.sensingDataList)):
            self.sensingData = self.sensingDataList[i].split(";")

            msg = ""
            for j in range(len(self.sensingData)):
                if j != 0: msg += " | "
                msg += self.sensingData[j]

            self.ui.sensingText.appendPlainText(msg)

        self.ui.logText.clear()
        for i in range(len(self.logDataList)):
            self.logText = self.logDataList[i].split(";")

            msg = ""
            for j in range(len(self.logText)):
                if j != 0: msg += " | "
                msg += self.logText[j]
            # msg = f'{self.sensingData[0]} | {self.sensingData[1]} | {self.sensingData[2]}\n'
            self.ui.logText.appendPlainText(msg)


    def go(self):
        self.commandData = self.makeCommandData("go", 100, 1)
        self.client.publish(commandTopic, "go", qos=DEFAULT_QOS)
        self.commandDataList.append(self.commandData)

    def mid(self):
        self.commandData = self.makeCommandData("halt", 100, 1)
        self.client.publish(commandTopic, "halt", qos=DEFAULT_QOS)
        self.commandDataList.append(self.commandData)

    def back(self):
        self.commandData = self.makeCommandData("back", 100, 1)
        self.client.publish(commandTopic, "back", qos=DEFAULT_QOS)
        self.commandDataList.append(self.commandData)

    def left(self):
        self.commandData = self.makeCommandData("left", 100, 1)
        self.client.publish(commandTopic, "left", qos=DEFAULT_QOS)
        self.commandDataList.append(self.commandData)

    def right(self):
        self.commandData = self.makeCommandData("right", 100, 1)
        self.client.publish(commandTopic, "right", qos=DEFAULT_QOS)
        self.commandDataList.append(self.commandData)

    def closeEvent(self, event):
        self.commandData = self.makeCommandData("exit", 100, 1)
        # self.client.publish(commandTopic, json.dumps(self.commandData), qos=1)

        self.commandDataList.append(self.commandData)
        self.commandData = dict()
        print(self.commandDataList)

        self.client.disconnect()

        current_time = datetime.now(korea_timezone)
        file_name = current_time.strftime("%Y-%m-%d") + "_logs.txt"
        with open(file_name, "w") as file:
            for value in self.logData:
                file.write(str(value) + "\n")

        file_name = current_time.strftime("%Y-%m-%d") + "_sensing.txt"
        with open(file_name, "w") as file:
            for value in self.sensorData:
                file.write(str(value) + "\n")

        print("save data")

        if self.timer.isActive(): self.timer.stop()
        if self.firebase_timer.isActive(): self.firebase_timer.stop()

        self.awsManager.stop()
        
        self.videoThread.stop()
        event.accept()

    def on_connect(self, client, userdata, flags, reason_code, properties):
        if reason_code == 0:
            print("connected OK")
            self.ui.gui_frame.show()
            self.ui.statuslbl.hide()
            self.videoThread.start()
        else:
            print("Bad connection Returned code=", reason_code)

    def on_message_log(self, client, userdata, msg):
        logMsg = msg.payload.decode("utf-8")
        # print(logMsg)

        [logMsg.split(';')[2]]

        self.logData.append(logMsg)
        self.logDataList = self.logData[-15:]

    def on_message_sensing(self, client, userdata, msg):
        try:
            sensingMsg = msg.payload.decode("utf-8")
            self.latest_sensing_data = sensingMsg  # 최신 데이터 저장
            self.sensorData.append(sensingMsg)
            self.sensingDataList = self.sensorData[-15:]
            
            # Firebase에 즉시 전송
            self.firebaseManager.store_sensing_data(sensingMsg)
            
            # Parse and update latest values
            msg_parts = sensingMsg.split(";")
            if len(msg_parts) == 3:  # Check valid format
                if msg_parts[1] == '[motor]':
                    values = [float(x) for x in msg_parts[2].split(',')]
                    # Send to AWS Manager
                    values = [values[1], values[0]]
                    self.awsManager.latest_motor = values
                elif msg_parts[1] == '[servo]':
                    values = [int(x) for x in msg_parts[2].split(',')]
                    # Send to AWS Manager
                    self.awsManager.latest_servo = values
                elif msg_parts[1] == '[battrey]':
                    values = int(msg_parts[2])
                    self.awsManager.latest_battey = values
                self.awsManager.latest_state = 1
        except Exception as e:
            print(f"Error processing sensing message: {e}")

    def on_message_camera(self, client, userdata, msg):
        self.base64ImageSignal.emit(msg.payload.decode("utf-8"))

    def show_camera(self, qpixmap):
        self.ui.video_lbl.setPixmap(qpixmap)

    def send_firebase(self):
        print("Send Firebase!")
        self.firebaseManager.test_send()
        pass

        # Add these methods to MainWindow:
    def on_aws_connection(self, connected):
        if connected:
            print("AWS WebSocket Connected")
        else:
            print("AWS WebSocket Disconnected")
            
    def on_aws_error(self, error_msg):
        print(f"AWS Error: {error_msg}")

    def send_prompt(self):
        user_prompt = self.ui.promptInput.text()
        self.question = user_prompt
        if not user_prompt:
            return
        self.ui.promptInput.clear()
        system_content = "You are an AI assistant for AGV control system."
        user_prompt += "log Data: " + str(self.logDataList)
        user_prompt += "sensing Data: " + str(self.sensingDataList)
        user_prompt += "command Data: " + str(self.commandDataList)
        self.openaiManager.send_prompt(system_content, user_prompt)

    def handle_ai_response(self, answer):
        self.ui.promptOutput.clear()
        self.ui.promptOutput.appendPlainText(f"Q: {self.question}\n")
        self.ui.promptOutput.appendPlainText(f"A: {answer}")

    def handle_ai_error(self, error_msg):
        self.ui.promptOutput.clear()
        self.ui.promptOutput.appendPlainText(f"Error: {error_msg}")

    def show_thinking(self):
        self.ui.promptOutput.clear()
        self.ui.promptOutput.appendPlainText("Thinking...")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    widget = MainWindow()
    widget.show()
    sys.exit(app.exec())
