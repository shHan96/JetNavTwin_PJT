    # def get_response(self, system_content, user_prompt):
# OpenAIManager.py
from PySide6.QtCore import QThread, Signal
from openai import OpenAI
import time
import json
import os
from dotenv import load_dotenv

class OpenAIManager(QThread):
    responseSignal = Signal(str)  # For (question, answer)
    errorSignal = Signal(str)  # For error messages
    thinkingSignal = Signal()  # To show thinking status

    def __init__(self):
        super().__init__()
        load_dotenv()
        self.api_key = os.getenv('OPENAI_API_KEY')
        self.assistant_id = os.getenv('OPENAI_ASSISTANT_ID')

        self.client = OpenAI(api_key=self.api_key)
        self.assistant = self.client.beta.assistants.retrieve(assistant_id=self.assistant_id)
        self.assistant_thread = self.client.beta.threads.create()
        self.prompt_queue = []
        self.running = False

    def send_prompt(self, system_content, user_prompt):
        self.prompt_queue.append((system_content, user_prompt))
        if not self.isRunning():
            self.start()

    def run(self):
        self.running = True
        while self.running and self.prompt_queue:
            system_content, user_prompt = self.prompt_queue.pop(0)
            try:
                self.thinkingSignal.emit()
                response = self.get_response(system_content, user_prompt)

                # response = self.get_response(system_content, user_prompt)
                self.responseSignal.emit(response)
            except Exception as e:
                self.errorSignal.emit(str(e))

    def get_response(self, system_content, user_prompt):
        message = self.client.beta.threads.messages.create(thread_id=self.assistant_thread.id, role="user", content=user_prompt)
        run = self.client.beta.threads.runs.create(thread_id=self.assistant_thread.id, assistant_id=self.assistant_id)
        run = self.wait_on_run(run)
        messages = self.client.beta.threads.messages.list(
            thread_id=self.assistant_thread.id, order="asc", after=message.id
        )
        return messages.data[0].content[0].text.value

    def wait_on_run(self, run):
        while run.status == "queued" or run.status == "in_progress":
            run = self.client.beta.threads.runs.retrieve(
                thread_id= self.assistant_thread.id,
                run_id=run.id,
            )
            time.sleep(0.5)
        return run
    

    # def get_response(self, system_content, user_prompt):
    #     try:
    #         response = openai.ChatCompletion.create(
    #             model="gpt-4",
    #             messages=[
    #                 {"role": "system", "content": system_content},
    #                 {"role": "user", "content": user_prompt}
    #             ],
    #             max_tokens=256,
    #             temperature=1.0,
    #             top_p=1.0,
    #             frequency_penalty=0,
    #             presence_penalty=0
    #         )
    #         return response.choices[0].message['content']
    #     except Exception as e:
    #         raise e

    def stop(self):
        self.running = False