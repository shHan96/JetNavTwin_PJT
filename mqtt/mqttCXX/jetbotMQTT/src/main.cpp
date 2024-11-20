#include <iostream>
#include <cstring>
#include <mqtt/async_client.h>
#include <Jetank/Servos.h>
#include <cstdint>

const std::string SERVER_ADDRESS("192.168.164.137:1883");
const std::string CLIENT_ID("jetbot");

struct TransferData {
    uint8_t servo_angle[6];
};

int main(int argc, char* argv[]) {
    Servos servos;
    TransferData transfer_data;
    std::string payload;
    for(uint8_t i = 1; i <= 5; i++) {
        payload += "Servo";
        payload += std::to_string(i);
        payload += ":";
        payload += std::to_string(servos.getServoData(i).current_angle);
        payload += ";";
    }

    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);

    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        client.connect(connOpts)->wait();

        std::string topic = "jetbot/info";
        int qos = 1;
        bool retained = false;

        mqtt::message_ptr pubmsg = mqtt::make_message(topic, payload, qos, retained);
        client.publish(pubmsg)->wait();

        client.disconnect()->wait();
    } catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}