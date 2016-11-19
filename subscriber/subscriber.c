#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <mosquitto.h>

/* Mosquitto callbacks functions */
void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
    printf("on_connect\n");
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
    printf("on_disconnect\n");
}

void on_publish(struct mosquitto *mosq, void *obj, int rc)
{
    printf("on_publish\n");
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    printf("on_message :\n");
    printf("\tmid         | %d\n", message->mid);
    printf("\ttopic       | %s\n", message->topic);
    printf("\tpayload     | %s\n", message->payload);
    printf("\tpayload_len | %d\n", message->payloadlen);
    printf("\tqos         | %d\n", message->qos);
    printf("\tretain      | %d\n", message->retain);
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
    printf("on_subscribe\n");
}

void on_unsubscribe(struct mosquitto *mosq, void *obj, int mid)
{
    printf("on_unsubscribe\n");
}

void on_log(struct mosquitto *mosq, void *obj, int level, const char *str)
{
    printf("[LOG] %s\n", str);
}

/* Main function  */
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
    int retval = 0;

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

    switch( retval = mosquitto_connect(mosq, host, port, keepalive) )
    {
        case MOSQ_ERR_INVAL:
            fprintf(stderr, "Error : %s\n", mosquitto_strerror(retval)); 
            return EXIT_FAILURE;
            break;
        case MOSQ_ERR_ERRNO:
            fprintf(stderr, "Error : %s\n", strerror(errno));
            return EXIT_FAILURE;
            break;
    }

    printf("Mosquitto client started ...\n");

    switch( retval = mosquitto_subscribe(mosq, NULL, "/test/", 0) )
    {
        case MOSQ_ERR_SUCCESS:
            printf("Subscription success.\n");
            break;
        case MOSQ_ERR_INVAL:
        case MOSQ_ERR_NOMEM:
        case MOSQ_ERR_NO_CONN:
            fprintf(stderr, "Error : %s\n", mosquitto_strerror(retval));
    }

    /* Define mosquitto callbacks */
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_publish_callback_set(mosq, on_publish);
    mosquitto_message_callback_set(mosq, on_message);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_unsubscribe_callback_set(mosq, on_unsubscribe);
    mosquitto_log_callback_set(mosq, on_log);

    /* Infinite network loop 
     * return on error or on call to mosquitto_disconnect()
     */
    retval = mosquitto_loop_forever(mosq, -1, 1);
    if( retval != MOSQ_ERR_SUCCESS )
    {
        fprintf(stderr, "Error : %s\n", mosquitto_strerror(retval));

        /* Call to free resources associated with the library */
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return EXIT_FAILURE;
    }

    /* Call to free resources associated with the library */
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return EXIT_SUCCESS;
}
