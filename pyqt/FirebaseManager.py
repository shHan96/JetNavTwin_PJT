from collections import defaultdict
from datetime import datetime, timedelta

# for Firebase
from config.firebase_config import FirebaseConfig
import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore
import pytz

from PySide6.QtCore import QThread, Signal

class FirebaseManager(QThread):
    statusSignal = Signal(str)

    def __init__(self):
        super().__init__()
        self.current_minute = None
        self.second_data = defaultdict(lambda: {'motor': None, 'servo': None, 'battery': None, 'state': None})
        self.latest_motor = [0, 0]  # Default values
        self.latest_servo = [0, 0, 0, 0, 0]  # Default values
        self.latest_state = "Unknown"
        self.db = None

    def connectFireBase(self):
        # 서비스 계정 키 파일 경로를 설정
        # key 이름은 각 팀마다 생성한 키 이름으로 변경한다.
        self.statusSignal.emit("Current find firestore-keys...")
        try:
            self.cred = credentials.Certificate('firebase-service-account.json')
        except FileNotFoundError:
            self.statusSignal.emit("Firebase 인증 키 파일이 없습니다.")
            return False
        # Firebase Admin SDK 초기화
        self.statusSignal.emit("initialize_firestore_app")
        firebase_admin.initialize_app(self.cred)
        # Firestore 클라이언트 초기화
        self.statusSignal.emit("Make db for firestore.client")
        self.db = firestore.client()
        self.statusSignal.emit("Success to connect FireStore")

    def store_sensing_data(self, data):
        try:
            current_time = datetime.now(pytz.timezone('Asia/Seoul'))
            current_minute = current_time.replace(second=0, microsecond=0)
            current_second = current_time.second
            
            if self.current_minute and current_minute > self.current_minute:
                self._save_minute_data(self.current_minute)
                self.second_data.clear()
            
            self.current_minute = current_minute

            parts = data.split(";")
            if len(parts) == 3:
                if parts[1] == '[motor]':
                    values = [float(x) for x in parts[2].split(',')]
                    self.latest_motor = values
                    self.second_data[current_second]['motor'] = values
                elif parts[1] == '[servo]':
                    values = [int(x) for x in parts[2].split(',')]
                    self.latest_servo = values
                    self.second_data[current_second]['servo'] = values
                elif parts[1] == '[battery]':
                    value = float(parts[2])
                    self.latest_battery = value
                    self.second_data[current_second]['battery'] = value
                elif parts[1] == '[state]':
                    value = float(parts[2])
                    self.latest_battery = value
                    self.second_data[current_second]['state'] = value

        except Exception as e:
            print(f"Error storing sensing data: {e}")

    def _save_minute_data(self, minute_time):
        try:
            per_second_data = []
            curr_motor = self.latest_motor
            curr_servo = self.latest_servo
            curr_battery = self.latest_battery
            curr_state = self.latest_state

            for second in range(60):
                second_data = self.second_data[second]
                motor_data = second_data['motor'] if second_data['motor'] is not None else curr_motor
                servo_data = second_data['servo'] if second_data['servo'] is not None else curr_servo
                battery_data = second_data['battery'] if second_data['battery'] is not None else curr_battery
                state_data = second_data['state'] if second_data['state'] is not None else curr_state

                if second_data['motor'] is not None:
                    curr_motor = second_data['motor']
                if second_data['servo'] is not None:
                    curr_servo = second_data['servo']
                if second_data['battery'] is not None:
                    curr_battery = second_data['battery']
                if second_data['state'] is not None:
                    curr_state = second_data['state']

                second_dict = {
                    'timestamp': (minute_time + timedelta(seconds=second)).strftime("%Y-%m-%d %H:%M:%S"),
                    'motor': motor_data,
                    'servo': servo_data,
                    'battery': battery_data,
                    'state': state_data
                }
                per_second_data.append(second_dict)

            doc_data = {
                'timestamp': minute_time.strftime("%Y-%m-%d %H:%M"),
                'data': per_second_data,
                'latest_motor': curr_motor,
                'latest_servo': curr_servo,
                'latest_battery': curr_battery,
                'latest_state': curr_state
            }

            time_doc = minute_time.strftime("%Y-%m-%d %H:%M")
            self.db.collection('sensingTable').document(time_doc).set(doc_data)
            print(f"Saved minute data: {time_doc}")

        except Exception as e:
            print(f"Error saving minute data: {e}")
