/**
 * @file metrics.h
 * @brief Funciones para obtener el uso de CPU y memoria desde el sistema de archivos /proc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/** Tamaño del buffer */
#define BUFFER_SIZE 256

/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Uso de memoria como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_memory_usage();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();

/**
 *@brief Devuelve la cantidad de cambios de contexto del CPU
 *
 *
 */
double get_context_switches_amount();

/**
 * @brief Devuelve la cantidad de procesos en ejecución
 *
 *
 */
int get_running_processes_amount();

/**
 * @brief Devuelve la cantidad de escrituras y lecturas de disco exitosas
 *
 *
 */

unsigned long* get_reads_writes();

/**
 * @brief Devulve la cantidad de paquetes recibidos y transmitidos
 */

unsigned long* get_tx_rx_packets();
