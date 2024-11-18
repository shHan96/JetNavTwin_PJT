#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>

#define ADDRESS     "192.168.164.137"
#define PORT        1883
#define KEEPALIVE   60
#define TOPIC       "test/topic"
#define PAYLOAD     "Hello World"
#define QOS         1

// 메시지 콜백 함수
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    printf("Received message on topic '%s': %s\n", msg->topic, (char *)msg->payload);
}

int main(int argc, char* argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Out of memory.\n");
        mosquitto_lib_cleanup();
        return 1;
    }

    // 메시지 콜백 함수 등록
    mosquitto_message_callback_set(mosq, on_message);

    rc = mosquitto_connect(mosq, ADDRESS, PORT, KEEPALIVE);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect (%d).\n", rc);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    // 토픽 구독
    if (mosquitto_subscribe(mosq, NULL, TOPIC, 0)) {
        fprintf(stderr, "Failed to subscribe to topic '%s'.\n", TOPIC);
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }

    printf("Subscribed to topic '%s'. Waiting for messages...\n", TOPIC);

    // 메시지 루프 실행
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}