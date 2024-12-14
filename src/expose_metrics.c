#include "../include/expose_metrics.h"
#include "memory.h"

/** Mutex para sincronización de hilos */

pthread_mutex_t lock;

/** Métrica de Prometheus para el uso de CPU */

static prom_gauge_t* cpu_usage_metric;

/** Métrica de Prometheus para el uso de memoria */

static prom_gauge_t* memory_usage_metric;

/** Métricas de Prometheus para memoria total, usada y disponible */

static prom_gauge_t* total_mem_metric;
static prom_gauge_t* free_mem_metric;
static prom_gauge_t* used_mem_metric;

/** Métrica de Prometheus para cantidad de cambios de contexto */

static prom_gauge_t* context_switches_metric;

/** Métrica de Prometheus para cantidad de procesos en ejecución */

static prom_gauge_t* running_processes_metric;

/** Métricas de lectura y escrituras exitosas a disco */

static prom_gauge_t* reads_metric;
static prom_gauge_t* writes_metric;

/** Métricas de paquetes transmitidos y recibidos exitosamente */

static prom_gauge_t* rx_packets_metric;
static prom_gauge_t* tx_packets_metric;

// Fragmentation Gauge for memory fragmentation rate
static prom_gauge_t* first_fit_fragmentation;
static prom_gauge_t* best_fit_fragmentation;
static prom_gauge_t* worst_fit_fragmentation;

// Counters for tracking the usage of allocation policies
static prom_counter_t* first_fit_counter;
static prom_counter_t* best_fit_counter;
static prom_counter_t* worst_fit_counter;

void update_cpu_gauge()
{
    double usage = get_cpu_usage();

    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }

    else
    {
        fprintf(stderr, "Error al obtener el uso de CPU\n");
    }
}

void update_memory_gauge()
{
    unsigned long long mem_usage_info[3];

    double usage = get_memory_usage(mem_usage_info);

    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_usage_metric, usage, NULL);
        prom_gauge_set(total_mem_metric, mem_usage_info[0], NULL);
        prom_gauge_set(free_mem_metric, mem_usage_info[1], NULL);
        prom_gauge_set(used_mem_metric, mem_usage_info[2], NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de memoria\n");
    }
}

void update_context_switches_gauge()
{
    int context_switches_amount = get_context_switches_amount();

    if (context_switches_amount >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(context_switches_metric, context_switches_amount, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la cantidad de cambios de contexto\n");
    }
}

void update_running_processes_gauge()
{
    int running_processes_amount = get_running_processes_amount();

    if (running_processes_amount >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(running_processes_metric, running_processes_amount, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la cantidad de procesos en ejecución \n");
    }
}

void update_reads_writes_gauge()
{
    unsigned long* read_writes_metrics = get_reads_writes();

    if (read_writes_metrics != NULL)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(reads_metric, read_writes_metrics[0], NULL);
        prom_gauge_set(writes_metric, read_writes_metrics[1], NULL);
        pthread_mutex_unlock(&lock);
        free(read_writes_metrics);
    }
    else
    {
        fprintf(stderr, "Error al obtener lecturas y escrituras a disco\n");
    }
}

void update_rx_tx_packets_gauge()
{
    unsigned long* rx_tx_packets_metric = get_tx_rx_packets();

    if (rx_tx_packets_metric != NULL)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(rx_packets_metric, rx_tx_packets_metric[0], NULL);
        prom_gauge_set(tx_packets_metric, rx_tx_packets_metric[1], NULL);
        pthread_mutex_unlock(&lock);
        free(rx_tx_packets_metric);
    }
    else
    {
        fprintf(stderr, "Error al obtener los paquetes recibido y enviados\n");
    }
}

void update_adv_memory_management_metrics()
{
    double first_fit_frag = test_efficiency(FIRST_FIT);
    double best_fit_frag = test_efficiency(BEST_FIT);
    double worst_fit_frag = test_efficiency(WORST_FIT);

    if (first_fit_frag >= 0 && best_fit_frag >= 0 && worst_fit_frag >= 0)
    {
        if (pthread_mutex_lock(&lock) == 0)
        {
            // Update fragmentation metrics
            prom_gauge_set(first_fit_fragmentation, first_fit_frag, NULL);
            prom_gauge_set(best_fit_fragmentation, best_fit_frag, NULL);
            prom_gauge_set(worst_fit_fragmentation, worst_fit_frag, NULL);

            // Increment counters for allocation policies
            prom_counter_inc(first_fit_counter, NULL);
            prom_counter_inc(best_fit_counter, NULL);
            prom_counter_inc(worst_fit_counter, NULL);

            pthread_mutex_unlock(&lock);
        }
        else
        {
            fprintf(stderr, "Error locking mutex for fragmentation metrics update\n");
        }
    }
    else
    {
        fprintf(stderr, "Error calculating fragmentation values\n");
    }
}

void* expose_metrics(void* arg)
{
    (void)arg; // Argumento no utilizado

    // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
    promhttp_set_active_collector_registry(NULL);

    // Iniciamos el servidor HTTP en el puerto 8000
    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error al iniciar el servidor HTTP\n");
        return NULL;
    }

    // Mantenemos el servidor en ejecución
    while (1)
    {
        sleep(1);
    }

    // Nunca debería llegar aquí
    MHD_stop_daemon(daemon);
    return NULL;
}

int init_metrics()
{
    // Inicializamos el mutex
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error al inicializar el mutex\n");
        return EXIT_FAILURE;
    }

    // Inicializamos el registro de coleccionistas de Prometheus
    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error al inicializar el registro de Prometheus\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de CPU
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "Porcentaje de uso de CPU", 0, NULL);
    if (cpu_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de CPU\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica para el porcentjae de uso de memoria
    memory_usage_metric = prom_gauge_new("memory_usage_percentage", "Porcentaje de uso de memoria", 0, NULL);
    if (memory_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de memoria\n");
        return EXIT_FAILURE;
    }

    // Creamos las métricas de memoria total,libre y en uso
    total_mem_metric = prom_gauge_new("total_memory", "Memoria Total", 0, NULL);
    free_mem_metric = prom_gauge_new("free_memory", "Memoria Disponible", 0, NULL);
    used_mem_metric = prom_gauge_new("used_memory", "Memoria en Uso", 0, NULL);

    if (total_mem_metric == NULL || free_mem_metric == NULL || used_mem_metric == NULL)
    {
        fprintf(stderr, "Error al crear las métricas memoria total,libre y en uso\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica de cambios de contexto
    context_switches_metric = prom_gauge_new("context_switches", "Cantidad de Cambios de Contexto", 0, NULL);
    if (context_switches_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de cantidad de cambios de contexto\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica de cambios de contexto
    running_processes_metric = prom_gauge_new("running_processes", "Cantidad de procesos en ejecución", 0, NULL);
    if (running_processes_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de cantidad de procesos en ejecución\n");
        return EXIT_FAILURE;
    }

    // Creamos las métricas de escritura y lectura a disco
    reads_metric = prom_gauge_new("reads_metric", "Lecturas a disco completadas", 0, NULL);
    writes_metric = prom_gauge_new("writes_metric", "Escrituras a disco completadas", 0, NULL);
    if (reads_metric == NULL || writes_metric == NULL)
    {
        fprintf(stderr, "Error al crear las métricas de escritura y lecturas (I/O) de disco\n");
        return EXIT_FAILURE;
    }

    // Creamos las métricas de paquetes recibidos y transitidos
    rx_packets_metric = prom_gauge_new("rx_packets_metric", "Paquetes recibidos", 0, NULL);
    tx_packets_metric = prom_gauge_new("tx_packets_metric", "Paquetes transmitidos", 0, NULL);
    if (rx_packets_metric == NULL || tx_packets_metric == NULL)
    {
        fprintf(stderr, "Error al crear las métricas de escritura y lecturas (I/O) de disco\n");
        return EXIT_FAILURE;
    }

    first_fit_fragmentation =
        prom_gauge_new("first_fit_fragmentation", "Porcentaje de fragmentacion - First fit", 0, NULL);
    best_fit_fragmentation =
        prom_gauge_new("best_fit_fragmentation", "Porcentaje de fragmentacion - Best fit", 0, NULL);
    worst_fit_fragmentation =
        prom_gauge_new("worst_fit_fragmentation", "Porcentaje de fragmentacion - Worst fit", 0, NULL);
    if (!first_fit_fragmentation || !best_fit_fragmentation || !worst_fit_fragmentation)
    {
        fprintf(stderr, "Error al crear las metricas de fragmentacion de memoria\n");
        return EXIT_FAILURE;
    }

    first_fit_counter = prom_counter_new("allocation_policy_first_fit_count",
                                         "Numero de veces que se utilizo First Fit", 0, NULL);
    best_fit_counter = prom_counter_new("allocation_policy_best_fit_count",
                                        "Numero de veces que se utilizo Best Fit", 0, NULL);
    worst_fit_counter = prom_counter_new("allocation_policy_worst_fit_count",
                                         "Numero de veces que se utilizo Worst Fit", 0, NULL);
    if (!first_fit_counter || !best_fit_counter || !worst_fit_counter)
    {
        fprintf(stderr, "Error al crear las métricas de políticas de asignación de memoria\n");
        return EXIT_FAILURE;
    }


    if (prom_collector_registry_must_register_metric(cpu_usage_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de uso de CPU\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(memory_usage_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de uso de memoria\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(total_mem_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de memoria total \n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(free_mem_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de memoria libre \n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(used_mem_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de memoria usada\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(context_switches_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de cambios de contexto\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(running_processes_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de procesos en ejecución\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(reads_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de lecturas\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(writes_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de escrituras\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(rx_packets_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de paquetes recibidos\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(tx_packets_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de paquetes transmitidos\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(first_fit_fragmentation) == NULL ||
        prom_collector_registry_must_register_metric(best_fit_fragmentation) == NULL ||
        prom_collector_registry_must_register_metric(worst_fit_fragmentation) == NULL)
    {
        fprintf(stderr, "Error registering fragmentation metrics\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_must_register_metric(first_fit_counter) == NULL ||
        prom_collector_registry_must_register_metric(best_fit_counter) == NULL ||
        prom_collector_registry_must_register_metric(worst_fit_counter) == NULL)
    {
        fprintf(stderr, "Error registering memory managment policies counters\n");
        return EXIT_FAILURE;
    }
}

void destroy_mutex()
{
    pthread_mutex_destroy(&lock);
}