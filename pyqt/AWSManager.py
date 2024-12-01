# AWSManager.py
from PySide6.QtCore import QThread, Signal
from queue import Queue
import asyncio
import websockets
import json
import time
from collections import deque
import os
from dotenv import load_dotenv

class AWSManager(QThread):
    connected_signal = Signal(bool)
    error_signal = Signal(str)
    
    def __init__(self):
        super().__init__()
        self.running = False
        load_dotenv()
        self.uri = os.getenv('AWS_WEBSOCKET_URI')
        self.loop = None
        self.last_send_time = 0
        self.send_interval = 0.25  # 4Hz (1/4 second)
        # Store latest values
        self.latest_motor = None
        self.latest_servo = None
        self.latest_battery = None
        self.latest_state = None

    def run(self):
        self.running = True
        self.loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.loop)

        try:
            self.loop.run_until_complete(self.send_control_data())
        except Exception as e:
            self.error_signal.emit(str(e))
        finally:
            self.loop.close()

    def stop(self):
        self.running = False
        if self.loop:
            self.loop.stop()

    async def send_control_data(self):
        while self.running:
            try:
                print("weksocket Connect...")
                async with websockets.connect(self.uri) as websocket:
                    self.connected_signal.emit(True)
                    while self.running:
                        current_time = time.time()
                        # Check if it's time to send data (4Hz)
                        if current_time - self.last_send_time >= self.send_interval:
                            # Prepare payload with buffered data
                            payload = {
                                "type": "control",
                                "jointRotations": self.latest_servo,
                                "trackSpeeds": self.latest_motor
                            }
                            await websocket.send(json.dumps(payload))
                            payload = {
                                "type": "status",
                                "battery": self.latest_battery,
                                "state": self.latest_state
                            }
                            self.last_send_time = current_time
                        
                        await asyncio.sleep(0.01)  # Small sleep to prevent CPU overload
                        
            except Exception as e:
                self.error_signal.emit(str(e))
                self.connected_signal.emit(False)
                await asyncio.sleep(5)  # Wait before reconnecting
