#include <iostream>
#include <cstring>
#include <mqtt/async_client.h>

const std::string SERVER_ADDRESS("tcp://localhost:1883");
const std::string CLIENT_ID("client2");

int main(int argc, char* argv[]) {
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);

    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        client.connect(connOpts)->wait();

        std::string topic = "test/topic";
        std::string payload = "Hello, World!";
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