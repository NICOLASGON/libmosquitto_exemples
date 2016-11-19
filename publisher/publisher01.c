#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <mosquitto.h>

int main(int argc, char **argv)
{
    int major = 0, minor = 0, revision = 0;
    struct mosquitto *mosq = NULL;
    bool clean_session = true;
    char *host = "localhost";
    int port = 1883;
    int keepalive = 60;
    char mqtt_message[200];
    char topic[200];

    /* Show library version */
    mosquitto_lib_version(&major, &minor, &revision);
    printf("Mosquitto library version : %d.%d.%d\n", major, minor, revision);    

    /* Init mosquitto library */
    mosquitto_lib_init();

    /* Create a new mosquitto client instance */
    mosq = mosquitto_new(NULL, clean_session, NULL);
    if( mosq == NULL )
    {
        switch(errno){
            case ENOMEM:
                fprintf(stderr, "Error: Out of memory.\n");
                break;
            case EINVAL:
                fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
                break;
        }
        mosquitto_lib_cleanup();
        return EXIT_FAILURE;
    }

    switch( mosquitto_connect(mosq, host, port, keepalive) )
    {
        case MOSQ_ERR_INVAL:
            fprintf(stderr, "Error : Invalid parameters\n"); 
            return EXIT_FAILURE;
            break;
        case MOSQ_ERR_ERRNO:
            fprintf(stderr, "Error : %s\n", strerror(errno));
            return EXIT_FAILURE;
            break;
    }

    printf("Mosquitto client started ...\n");

    sprintf(topic, "/gateway/1");
    sprintf(mqtt_message, "Hello world!");
    switch( mosquitto_publish(mosq, NULL, topic, strlen(mqtt_message), mqtt_message, 0, false) )
    {
        case MOSQ_ERR_SUCCESS:
            printf("Publish success\n");
            break;
        case MOSQ_ERR_INVAL:
        case MOSQ_ERR_NOMEM:
        case MOSQ_ERR_NO_CONN:
        case MOSQ_ERR_PROTOCOL:
        case MOSQ_ERR_PAYLOAD_SIZE:
            fprintf(stderr, "Error : %s\n", mosquitto_strerror(errno));
            break;
    }

    /* Call to free resources associated with the library */
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return EXIT_SUCCESS;
}
