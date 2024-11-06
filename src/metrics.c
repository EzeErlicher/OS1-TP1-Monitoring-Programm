#include "../include/metrics.h"

#include <math.h>

double get_memory_usage(unsigned long long arr_mem_values[3])
{
    enum memory_type
    {
        TOTAL = 0,
        FREE = 1,
        USED = 2
    };

    FILE* fp;
    char buffer[BUFFER_SIZE];

    unsigned long long total_mem = 0, free_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/meminfo");
        return -1.0;
    }

    // Leer los valores de memoria total y disponible
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal encontrado
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable encontrado, podemos dejar de leer
        }
    }

    fclose(fp);

    // Verificar si se encontraron ambos valores
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error al leer la información de memoria desde /proc/meminfo\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de memoria
    double used_mem = total_mem - free_mem;
    double mem_usage_percent = (used_mem / total_mem) * 100.0;

    // Se convierte la información de uso de memoria a MB (megabytes)
    unsigned long long total_mem_in_MB = total_mem / 1024;
    unsigned long long free_mem_in_MB = free_mem / 1024;
    unsigned long long used_mem_in_MB = used_mem / 1024;
    arr_mem_values[TOTAL] = total_mem_in_MB;
    arr_mem_values[FREE] = free_mem_in_MB;
    arr_mem_values[USED] = used_mem_in_MB;

    return mem_usage_percent;
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    double cpu_usage_percent;

    // Abrir el archivo /proc/stat
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    char buffer[BUFFER_SIZE * 4];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer /proc/stat");
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Analizar los valores de tiempo de CPU
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < 8)
    {
        fprintf(stderr, "Error al parsear /proc/stat\n");
        return -1.0;
    }

    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long prev_idle_total = prev_idle + prev_iowait;
    unsigned long long idle_total = idle + iowait;

    unsigned long long prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;

    unsigned long long prev_total = prev_idle_total + prev_non_idle;
    unsigned long long total = idle_total + non_idle;

    totald = total - prev_total;
    idled = idle_total - prev_idle_total;

    if (totald == 0)
    {
        fprintf(stderr, "Totald es cero, no se puede calcular el uso de CPU!\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de CPU
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Actualizar los valores anteriores para la siguiente lectura
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent;
}

double get_context_switches_amount()
{
    FILE* ptr;

    ptr = fopen("/proc/stat", "r");

    char line[BUFFER_SIZE];

    double amount_of_context_switches = 0;

    while (fgets(line, sizeof(line), ptr))
    {
        if (strncmp(line, "ctxt ", 5) == 0)
        {
            char* endptr;
            amount_of_context_switches = strtod(line + 5, &endptr);
        }
    }

    fclose(ptr);

    return amount_of_context_switches;
}

int get_running_processes_amount()
{
    FILE* ptr;

    ptr = fopen("/proc/stat", "r");

    char line[BUFFER_SIZE];

    int amount_of_running_processes = 0;

    while (fgets(line, sizeof(line), ptr))
    {
        if (strncmp(line, "procs_running ", 14) == 0)
        {
            amount_of_running_processes = atoi(line + 14);
        }
    }

    fclose(ptr);

    return amount_of_running_processes;
}

unsigned long* get_reads_writes()
{

    FILE* file;
    char line[256];
    unsigned long* reads_writes = malloc(2 * sizeof(unsigned long));

    // reads
    reads_writes[0] = 0;

    // writes
    reads_writes[1] = 0;

    file = fopen("/proc/diskstats", "r");

    if (file == NULL)
    {
        perror("Error al abrir /proc/diskstats");
        free(reads_writes);
        return reads_writes;
    }

    while (fgets(line, sizeof(line), file))
    {
        if (strstr(line, "sda "))
        {
            sscanf(line, "%*d %*d sda %lu %*d %*d %*d %lu %*d %*d %*d %*d %*d %*d", &reads_writes[0], &reads_writes[1]);
            break;
        }
    }

    fclose(file);

    return reads_writes;
}

unsigned long* get_tx_rx_packets()
{

    FILE* fp = fopen("/proc/net/dev", "r");

    if (fp == NULL)
    {
        perror("Failed to open /proc/net/dev");
        return NULL;
    }

    char line[1024];
    int found = 0;

    unsigned long* packets = malloc(sizeof(unsigned long) * 2);

    unsigned long rx_bytes, rx_packets, rx_errs, rx_drop, rx_fifo, rx_frame, rx_compressed, rx_multicast, tx_bytes,
        tx_packets;

    // Read the file line by line
    while (fgets(line, sizeof(line), fp))
    {
        if (strstr(line, "wlp2s0"))
        {
            if (sscanf(line, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &rx_bytes, &rx_packets, &rx_errs, &rx_drop,
                       &rx_fifo, &rx_frame, &rx_compressed, &rx_multicast, &tx_bytes, &tx_packets) == 10)
            {
                packets[0] = rx_packets;
                packets[1] = tx_packets;
                found = 1;
            }

            break;
        }
    }

    if (found == 0)
    {
        fprintf(stderr, "No se encontró la Interfaz");
    }

    fclose(fp);

    return packets;
}
