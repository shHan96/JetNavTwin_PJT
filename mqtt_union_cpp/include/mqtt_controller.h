// mqtt_controller.h
#pragma once
#include <mqtt/async_client.h>
#include <string>
#include <functional>
#include <atomic>
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>

class MQTTController : public virtual mqtt::callback, 
                      public virtual mqtt::iaction_listener {
public:
    enum class Command {
        START,
        STOP,
        FORCE_QUIT,
        GO,
        HALT,
        LEFT,
        RIGHT,
        BACK
    };

    MQTTController(const std::string& address = "tcp://192.168.164.137:1883") 
        : client_(address, "agv_controller") {
        
        connectOptions_.set_keep_alive_interval(20);
        connectOptions_.set_clean_session(true);
        client_.set_callback(*this);
    }
    void on_failure(const mqtt::token& tok) override {}
    void on_success(const mqtt::token& tok) override {}
    void start() {
        try {
            mqtt::token_ptr conntok = client_.connect(connectOptions_);
            conntok->set_action_callback(*this);  // 비동기 연결 완료 콜백
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error: " << exc.what() << std::endl;
            throw;
        }
    }

    void stop() {
        if (client_.is_connected()) {
            try {
                publishLog(LogType::SYSTEM, "MQTT connection closing");
                client_.disconnect(); // 비동기 연결 해제
            }
            catch (const mqtt::exception& exc) {
                std::cerr << "Error disconnecting: " << exc.what() << std::endl;
            }
        }
    }

    enum class LogType {
        COMMAND,
        SYSTEM,
        SENSING,
        ERROR
    };

    void publishLog(LogType type, const std::string& message) {
        if (!client_.is_connected()) return;

        try {
            std::string logTypeStr;
            switch (type) {
                case LogType::COMMAND: logTypeStr = "command"; break;
                case LogType::SYSTEM: logTypeStr = "system"; break;
                case LogType::SENSING: logTypeStr = "sensing"; break;
                case LogType::ERROR: logTypeStr = "error"; break;
            }

            std::string timestamp = getCurrentTimestamp();
            std::string logMessage = timestamp + ";[" + logTypeStr + "];" + message;
            
            mqtt::message_ptr pubmsg = mqtt::make_message("AGV/log", logMessage);
            pubmsg->set_qos(1);
            client_.publish(pubmsg);  // 비동기 발행
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error publishing log: " << exc.what() << std::endl;
        }
    }
void publishImageData(const std::string& base64_image) {
        if (!client_.is_connected()) return;

        try {
            mqtt::message_ptr pubmsg = mqtt::make_message("AGV/camera", base64_image);
            pubmsg->set_qos(1);
            client_.publish(pubmsg);
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error publishing image data: " << exc.what() << std::endl;
        }
    }
void publishSensingData(float x, float y) {
    if (!client_.is_connected()) return;

    try {
        // 센싱 데이터를 직접 AGV/sensing 토픽으로 전송
        std::stringstream ss;
        ss << std::fixed << std::setprecision(4) << x << "," << y;
        std::string timestamp = getCurrentTimestamp();
        std::string logMessage = timestamp + ";[" + std::string("sensing") + "];" + ss.str();
        mqtt::message_ptr pubmsg = mqtt::make_message("AGV/sensing", logMessage);
        pubmsg->set_qos(1);
        client_.publish(pubmsg);
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "Error publishing sensing data: " << exc.what() << std::endl;
    }
}
    void publishMotorData(double left_speed, double right_speed) {
        if (!client_.is_connected()) return;

        try {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3);
            ss << left_speed << "," << right_speed;
            
            std::string timestamp = getCurrentTimestamp();
            std::string message = timestamp + ";[motor];" + ss.str();
            
            mqtt::message_ptr pubmsg = mqtt::make_message("AGV/sensing", message);
            pubmsg->set_qos(1);
            client_.publish(pubmsg);
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error publishing motor data: " << exc.what() << std::endl;
        }
    }

    void publishServoData(const std::vector<int16_t>& angles) {
        if (!client_.is_connected()) return;

        try {
            std::stringstream ss;
            for (size_t i = 0; i < angles.size(); ++i) {
                if (i > 0) ss << ",";
                ss << angles[i];
            }
            
            std::string timestamp = getCurrentTimestamp();
            std::string message = timestamp + ";[servo];" + ss.str();
            
            mqtt::message_ptr pubmsg = mqtt::make_message("AGV/sensing", message);
            pubmsg->set_qos(1);
            client_.publish(pubmsg);
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error publishing servo data: " << exc.what() << std::endl;
        }
    }
    void publishBatteryStatus(float capacity) {
        if (!client_.is_connected()) return;

        try {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << capacity;
            
            std::string timestamp = getCurrentTimestamp();
            std::string message = timestamp + ";[battery];" + ss.str();
            
            mqtt::message_ptr pubmsg = mqtt::make_message("AGV/sensing", message);
            pubmsg->set_qos(1);
            client_.publish(pubmsg);
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "Error publishing battery status: " << exc.what() << std::endl;
        }
    }
    void setCommandCallback(std::function<void(Command)> callback) {
        command_callback_ = callback;
    }

private:
    // mqtt::callback interface implementation
    void connected(const std::string& cause) override {
        client_.subscribe("AGV/command", 1);
        publishLog(LogType::SYSTEM, "MQTT connection established");
    }

    void connection_lost(const std::string& cause) override {
        std::cerr << "Connection lost: " << cause << std::endl;
        while (!client_.is_connected()) {
            try {
                client_.connect(connectOptions_);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            catch (const mqtt::exception& exc) {
                std::cerr << "Error reconnecting: " << exc.what() << std::endl;
            }
        }
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        if (msg->get_topic() == "AGV/command") {
            const std::string payload = msg->get_payload_str();
            Command cmd;
            
            if (payload == "start") cmd = Command::START;
            else if (payload == "stop") cmd = Command::STOP;
            else if (payload == "force_quit") cmd = Command::FORCE_QUIT;
            else if (payload == "go") cmd = Command::GO;
            else if (payload == "halt") cmd = Command::HALT;
            else if (payload == "left") cmd = Command::LEFT;
            else if (payload == "right") cmd = Command::RIGHT;
            else if (payload == "back") cmd = Command::BACK;
            else return;

            publishLog(LogType::COMMAND, payload);

            if (command_callback_) {
                command_callback_(cmd);
            }
        }
    }

    void delivery_complete(mqtt::delivery_token_ptr token) override {}

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        struct tm timeinfo;
        localtime_r(&time, &timeinfo);

        std::stringstream ss;
        ss << std::setfill('0')
           << timeinfo.tm_year + 1900 << ":"
           << std::setw(2) << timeinfo.tm_mon + 1 << ":"
           << std::setw(2) << timeinfo.tm_mday << ":"
           << std::setw(2) << timeinfo.tm_hour << ":"
           << std::setw(2) << timeinfo.tm_min << ":"
           << std::setw(2) << timeinfo.tm_sec;
        return ss.str();
    }

    mqtt::async_client client_;
    mqtt::connect_options connectOptions_;
    std::function<void(Command)> command_callback_;
};