# for ImageProcess
import base64
import cv2
import numpy as np
from queue import Queue
import time

from PySide6.QtCore import QThread, Signal
from PySide6.QtGui import QImage, QPixmap

# This Python file uses the following encoding: utf-8

# if __name__ == "__main__":
#     pass

class Base64ImageProcessor(QThread):
    image_signal = Signal(QPixmap)
    running = False

    def __init__(self):
        super().__init__()
        self.running = True
        self.image_queue = Queue(maxsize=2) # Limit queue size
        self.last_update_time = 0
        self.min_update_interval = 1/60 # ~60 FPS max

    def run(self):
        while self.running:
            try:
                if not self.image_queue.empty():
                    # Get latest image data
                    base64_data = self.image_queue.get_nowait()

                    # Check if enough time has passed since last update
                    current_time = time.time()
                    if (current_time - self.last_update_time) >= self.min_update_interval:
                        # Process image
                        self._process_image(base64_data)
                        self.last_update_time = current_time

                    self.image_queue.task_done()
                else:
                    # Sleep briefly to prevent busy waiting
                    time.sleep(0.001)

            except Exception as e:
                print(f"Error in image processing thread: {e}")

    def update_image(self, base64_data):
        try:
            # Add new image data to queue, skip if queue is full
            self.image_queue.put_nowait(base64_data)
        except:
            pass

    def _process_image(self, base64_data):
        try:
            # base64 디코딩
            image_data = base64.b64decode(base64_data)
            # 바이트 데이터를 NumPy 배열로 변환
            np_arr = np.frombuffer(image_data, np.uint8)
            # NumPy 배열을 OpenCV 이미지로 디코딩
            image = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
            # Resize for better performance (adjust size as needed)
            image = cv2.resize(image, (240, 240))
            # OpenCV 이미지를 QImage로 변환
            height, width, channel = image.shape
            bytes_per_line = 3 * width
            qimage = QImage(image.data, width, height, bytes_per_line, QImage.Format_RGB888)
            # QImage를 QPixmap으로 변환
            pixmap = QPixmap.fromImage(qimage)
            # Emit signal with new image
            self.image_signal.emit(pixmap)

        except Exception as e:
            print(f"Error processing image: {e}")

    def stop(self):
        self.running = False
