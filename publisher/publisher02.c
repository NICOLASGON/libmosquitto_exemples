#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

#include <mosquitto.h>

#define NUM_THREADS 10

struct mosquitto *mosq = NULL;

void *publisher_thread(void *parameters)
{
    char mqtt_message[200];
    char topic[200];
    int error = 0;

    sprintf(topic, "/gateway/%d", *(int *)parameters);
    sprintf(mqtt_message, "Hello world from %d", *(int *)parameters);

    printf("%s %s\n", topic, mqtt_message);
    if( mosquitto_publish(mosq, NULL, topic, strlen(mqtt_message), mqtt_message, 1, false) != MOSQ_ERR_SUCCESS )
        printf("ERROR\n");
}

int main(int argc, char **argv)
{
    int major = 0, minor = 0, revision = 0;
    bool clean_session = true;
    char *host = "localhost";
    int port = 1883;
    int keepalive = 60;
    pthread_t threads[NUM_THREADS];

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

    mosquitto_max_inflight_messages_set(mosq, 0);
    mosquitto_message_retry_set(mosq, 0);
    mosquitto_threaded_set(mosq, false);

    switch( mosquitto_connect(mosq, host, port, keepalive) )
    {
        case MOSQ_ERR_INVAL:
            fprintf(stderr, "Error : Invalid parameters\n"); 
            return EXIT_FAILURE;
            break;
        case MOSQ_ERR_ERRNO:
            fprintf(stderr, "Error : %s\n", mosquitto_strerror(errno));
            return EXIT_FAILURE;
            break;
    }
    
    printf("Mosquitto client started ...\n");

    int i = 0;
    int threads_id[NUM_THREADS];
    for(i=0; i < NUM_THREADS; i++)
    {
        threads_id[i] = i;
        pthread_create(&(threads[i]), NULL, &publisher_thread, &(threads_id[i]));
    }
 
    for(i=0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    /* Call to free resources associated with the library */
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return EXIT_SUCCESS;
}
