# Monitor de Sistemas en un Mundo Postapocalíptico

## Introducción

En un mundo devastado por la pandemia del Cordyceps, donde cada recurso cuenta para la supervivencia, es crucial mantener y monitorear los sistemas que aún funcionan. En esta guía, aprenderás a desarrollar un programa en C que permita a las comunidades sobrevivientes leer datos de uso de CPU desde el sistema de archivos `/proc`, exponer estos datos utilizando la librería `prometheus-client-c` y, finalmente, visualizarlos en Grafana. Este proceso te ayudará a monitorear y analizar en tiempo real el consumo de CPU de los sistemas críticos que mantienen en funcionamiento las pocas infraestructuras tecnológicas restantes.

## ¿Qué aprenderemos?

- **Conocimientos Básicos en C:** Manejo de archivos y entradas/salidas en C para sistemas en condiciones adversas.
- **Sistema Operativo Linux:** Uso del archivo `/proc` en sistemas Linux supervivientes.
- **Prometheus y Grafana:** Instalación y configuración en entornos con recursos limitados.
- **Librería `prometheus-client-c`:** Utilización para exponer métricas esenciales para la supervivencia tecnológica.

### Preparativos

Dado que los recursos son escasos y las conexiones a internet son prácticamente inexistentes, asumiremos que tienes acceso local a los paquetes necesarios o que los has rescatado de servidores abandonados.

### Instalación de Prometheus

Consulta el manual local [INSTALL.md](INSTALL.md) que recuperamos de los antiguos servidores para instalar Prometheus desde los recursos disponibles.

### Instalación de Grafana

Sigue las instrucciones en los documentos impresos que tenemos disponibles, equivalentes a [esta guía](https://grafana.com/docs/grafana/latest/setup-grafana/installation/debian/).

### Instalación de `prometheus-client-c`

Asegúrate de tener acceso al repositorio local o al medio de almacenamiento donde se encuentra la librería:

```bash
git clone https://github.com/digitalocean/prometheus-client-c.git
cd prometheus-client-c
make
sudo make install
```

## Paso 1: Lectura de Datos de Consumo de CPU desde `/proc/`

Incluso en estos tiempos, los sistemas Linux siguen siendo el pilar de nuestra infraestructura tecnológica. Los archivos del directorio `/proc/` nos permite acceder a estadísticas vitales del sistema, incluyendo el consumo de CPU, esencial para asegurar que nuestros sistemas no fallen en momentos críticos.

### Explicación de `/proc/stat`

El archivo `/proc/stat` contiene líneas que comienzan con "cpu", seguidas de valores que representan el tiempo que la CPU ha pasado en diferentes estados: `user`, `nice`, `system`, `idle`, etc.

Ejemplo de salida:

```
cpu  4705 0 3421 1204310 50 120 0 0 0 0
```

- `user`: Tiempo en modo usuario (procesos normales).
- `nice`: Tiempo en procesos con prioridad modificada.
- `system`: Tiempo ejecutando procesos del sistema.
- `idle`: Tiempo que la CPU ha estado inactiva (importante para gestionar recursos).

### Código en C para Leer `/proc/stat`

Consulta el programa de ejemplo `read_cpu_usage.c`, que hemos adaptado para funcionar en nuestras condiciones actuales.

#### Explicación del Código:

- **`fopen`:** Abre `/proc/stat` en modo lectura.
- **`fgets`:** Lee línea por línea hasta encontrar la información de la CPU.
- **`strncmp`:** Verifica que estamos analizando la línea correcta.
- **`fclose`:** Cierra el archivo para conservar recursos.

## Paso 2: Exponer los Datos a Prometheus

Es vital compartir estas métricas con los demás puestos de control. Al exponer estos datos, podemos mantener una vigilancia constante y coordinada de nuestros sistemas.

### Agregar `prometheus-client-c` a tu Proyecto

Compila tu código, por ejemplo, 'expose_metrics.c':

```bash
gcc expose_metrics.c -o metrics -lprom -pthread -lpromhttp
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
./metrics
```

### Acceder a `/metrics` de Prometheus

Este endpoint expone las métricas en el formato que Prometheus puede recolectar. Asegúrate de que los demás puestos puedan acceder a este endpoint para una monitorización colaborativa.

Si experimentas problemas como un "segfault", revisa cuidadosamente el manejo de las métricas en el código, ya que puede ser crítico para la estabilidad del sistema.

## Paso 3: Visualizar los Datos en Grafana

Con las métricas expuestas y recolectadas, utilizaremos Grafana para visualizarlas en nuestros monitores y así mantener una vigilancia constante.

### Configurar Grafana

1. Accede a Grafana desde el terminal seguro.
2. Configura Prometheus como fuente de datos:
   - URL: `http://localhost:9090` (o la dirección del servidor Prometheus en tu red local).
3. Crea un nuevo dashboard y añade un panel con la métrica `cpu_usage`.

### Ejemplo de Consulta

Utiliza la siguiente consulta en Grafana para visualizar el uso de CPU:

```
cpu_usage
```

## Actividad

### Mejorar tu Dashboard

Es esencial ampliar nuestro monitoreo para garantizar la estabilidad de nuestros sistemas. Agrega las siguientes métricas:

- **Uso de Memoria:** Lee estadísticas desde `/proc/meminfo` y expón métricas para la memoria total, usada y disponible.
- **I/O de Disco:** Rastrea estadísticas desde `/proc/diskstats` para prevenir fallos en el almacenamiento.
- **Estadísticas de Red:** Expon métricas de tráfico desde `/proc/net/dev` para asegurar la comunicación entre puestos.
- **Conteo de Procesos:** Calcula el número de procesos en ejecución para detectar sobrecargas.
- **Cambios de Contexto:** Rastrea desde `/proc/stat` para analizar el rendimiento del sistema.

### Preguntas que debes responder en tu informe

1. **¿Cuál es el propósito del sistema de archivos `/proc` en Linux, y cómo podemos usarlo para recopilar métricas del sistema?**
2. **¿Cómo interpretas los campos en `/proc/stat` relacionados con el uso de la CPU, y cómo se utilizan para calcular el porcentaje de utilización de la CPU?**
3. **Explica cómo Prometheus recopila y almacena métricas, y cómo Grafana visualiza estos datos. ¿Cuáles son los componentes clave de cada uno?**
4. **¿Cuál es la diferencia entre un _gauge_, un _counter_ y un _histograma_ en Prometheus? Proporciona un ejemplo de cuándo debería usarse cada uno.**
5. **¿Por qué sería necesario un _mutex_ al trabajar con métricas en un entorno multi-thread? ¿Qué podría salir mal si no se utiliza?**

## Conclusión

A pesar de las adversidades, hemos logrado crear un programa en C que lee el uso de la CPU y la memoria desde /proc, expone esos datos y los visualiza para mantener nuestros sistemas críticos en funcionamiento. Este conocimiento es vital para la supervivencia y el restablecimiento de nuestra sociedad.

## Recursos Adicionales

- **Documentación de `/proc`**: Consulta los manuales locales o documentos impresos que hemos recopilado.
- **Prometheus Client for C**: Revisa el código fuente disponible en nuestros repositorios locales.
- **Documentación de Grafana**: Utiliza las guías impresas que tenemos en nuestro centro de control.
