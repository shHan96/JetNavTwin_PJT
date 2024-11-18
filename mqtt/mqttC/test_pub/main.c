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

int main(int argc, char* argv[]) {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    rc = mosquitto_connect(mosq, ADDRESS, PORT, KEEPALIVE);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect (%d).\n", rc);
        return 1;
    }

    rc = mosquitto_publish(mosq, NULL, TOPIC, strlen(PAYLOAD), PAYLOAD, QOS, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to publish (%d).\n", rc);
        return 1;
    }

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}