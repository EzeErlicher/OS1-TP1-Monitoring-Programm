#include "../include/expose_metrics.h"
#include <cjson/cJSON.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define PIPE_PATH "/tmp/monitor_pipe"

bool cpu_enabled, memory_enabled, context_switches_enabled, running_processes_enabled, reads_writes_enabled,
    rx_tx_packets_enabled;

void load_config(const char* config_path)
{
    FILE* file = fopen(config_path, "r");
    if (!file)
    {
        perror("Could not open config file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* json_content = malloc(file_size + 1);
    fread(json_content, 1, file_size, file);
    json_content[file_size] = '\0';
    fclose(file);

    cJSON* config_json = cJSON_Parse(json_content);
    if (!config_json)
    {
        fprintf(stderr, "Error parsing config file\n");
        exit(EXIT_FAILURE);
    }

    cJSON* metrics = cJSON_GetObjectItem(config_json, "metrics");
    cpu_enabled = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "cpu"));
    memory_enabled = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "memory"));
    context_switches_enabled = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "context_switches"));
    running_processes_enabled = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "running_processes"));
    reads_writes_enabled = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "reads_writes"));
    rx_tx_packets_enabled = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "rx_tx_packets"));

    cJSON_Delete(config_json);
    free(json_content);
}

void handle_sigterm(int signum)
{
    printf("Monitor stopping...\n");
    exit(0);
}

#define SLEEP_TIME 1

int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <config_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    init_metrics();

    load_config(argv[1]);

    signal(SIGTERM, handle_sigterm);

    int pipe_fd = open(PIPE_PATH, O_WRONLY);
    if (pipe_fd == -1)
    {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    pthread_t tid;
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error creating HTTP server thread\n");
        return EXIT_FAILURE;
    }

    while (true)
    {
        if (cpu_enabled)
        {
            update_cpu_gauge();
        }
        if (memory_enabled)
        {
            update_memory_gauge();
        }
        if (context_switches_enabled)
        {
            update_context_switches_gauge();
        }
        if (running_processes_enabled)
        {
            update_running_processes_gauge();
        }
        if (reads_writes_enabled)
        {
            update_reads_writes_gauge();
        }
        if (rx_tx_packets_enabled)
        {
            update_rx_tx_packets_gauge();
        }

        sleep(SLEEP_TIME);
    }

    return EXIT_SUCCESS;
}
