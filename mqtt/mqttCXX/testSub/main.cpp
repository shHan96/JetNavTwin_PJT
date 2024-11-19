#include <iostream>
#include <cstring>
#include <mqtt/async_client.h>

const std::string SERVER_ADDRESS("tcp://localhost:1883");
const std::string CLIENT_ID("mqtt_client1");
const std::string TOPIC("test/topic");

class mqtt_callback : public virtual mqtt::callback {
    void connection_lost(const std::string& cause) override {
        std::cout << "\nConnection lost: " << cause << std::endl;
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        std::cout << "Message arrived: " << msg->get_payload_str() << std::endl;
    }

    void delivery_complete(mqtt::delivery_token_ptr token) override {}
};

int main() {
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);

    mqtt::connect_options conn_opts;
    conn_opts.set_clean_session(true);

    mqtt_callback cb;
    client.set_callback(cb);

    try {
        client.connect(conn_opts)->wait();
        client.subscribe(TOPIC, mqtt::GRANTED_QOS_0)->wait();

        std::cout << "Subscribed to topic: " << TOPIC << std::endl;

        while (true) {
            // Keep the main thread alive
        }
    } catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}