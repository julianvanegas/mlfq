# Multi-Level Feedback Queue - MLFQ

## Uso del simulador

Desde la raíz del proyecto ejecutar los siguientes comandos:

**1. Compilación**

```
gcc src/main.c src/application/*.c src/domain/*.c src/infrastructure/*.c -o mlfq_exe
```

**2. Ejecución**

```
./mlfq_exe
```

## Análisis
### 1. ¿Qué ocurre si el boost es muy frecuente?

Un boost de ciclos cortos, lo que lo hace frecuente, provoca que el algoritmo MLFQ se comporte similar al Round Robin porque ningún proceso permanece sufieciente tiempo en colas de menor prioridad, por lo que el MLFQ no cumple su propósito que es distinguir entre procesos intensivos en CPU (CPU-bound) y procesos limitados por E/S (I/O-bound); por lo que procesos largos no se acumulan lo suficiente en colas de baja prioridad antes de volver a la cola de máxima prioridad.
Por ejemplo, si tenemos tres colas Q0, Q1, Q2 con quantums de 1, 2, 4 ciclos respectivamente; y un Priority Boost de 1 ciclo (igual al quantum de Q0); ningún proceso llega estar en las colas Q1 o Q2 antes de volver a Q0.

### 2. ¿Qué ocurre si no existe boost?

Si no exite un tiempo de Priority Boost, no existiría ningún mecanismo de promoción hacia colas de mayor prioridad en el sistema y solo habría democión a colas cada vez de menor prioridad y eventualmente se quedaría en la cola de menor prioridad.
Por ejemplo, supongamos que llega un proceso que es intensivo en CPU por lo que desenderá a Q2 (cola de menor prioridad) y permanecerá allí, sin importar si después empieza a comportarce como un proceso interactivo con ráfagas de E/S. Si luego llegan nuevos procesos continuamente y entran a Q0 (cola de mayor prioridad) siempre se ejecutan por lo que el proceso en Q2 se va a quedar esperando.

### 3. ¿Cómo afecta un quantum pequeño en la cola de mayor prioridad?

Con un quantum corto en la cola de mayor prioridad los procesos cortos, interactivos o intensivos en E/S terminan rapidamente o no sufren democión rápidamente a colas de menor prioridad, generando un buen Response Time.
La gran dificultad surge cuando cualquier proceso intensivo en CPU (CPU-bound) se degrada rápidamente a colas de menor prioridad tras un cortos ciclos de ejecución multiplicando la cantidad de decisiones de scheduling y movimientos de cola; incluso con Priority Boost puede ser degradado a colas de menor prioridad rapidamente al superar el quantum.

### 4. ¿Puede haber starvation?

Si no hay Priority Boost o con intervalo muy largo el starvation se vuelve permanente o casi si ese mecanismo de rescate tiene una ventana de tiempo muy grande. Y como el scheduler siempre revisa en orden de prioridad, primero la cola de mayor prioridad hasta la de menor prioridad, por lo que si llegan procesos nuevos constantemente que inician en la cola de mayor prioridad, los procesos en las demás colas pueden no recibir CPU provacando starvation.


## Diseño

### Arquitectura

<img src="https://github.com/julianvanegas/mlfq/blob/main/arquitectura.png" alt="Descripción de la imagen">

El sistema se divide en tres capas: dominio, aplicación e infraestructura; en lugar de tener todo mezclado en único módulo. La capa dominio contiene las entidades del sistema Proceso (Process) y Cola (Queue); mientras la capa aplicación contiene la capa lógica del MLFQ con el Scheduler y el gestión de E/S (I/O); mientras la capa de infraestructura contiene todos los mecanismos de salida tanto en consola y exportacion de resultados. Esta separación entre componentes se hace así:

- Un proceso solo debe ser responsable de su propio estado (saber si terminó o si necesita I/O) mientras el Scheduler es responsable de decidir quién tiene el turno de jecución en CPU. Por lo que si el algoritmo MLFQ cambia, no es necesario que el Proceso cambie.

- Para evitar el código duplicado no se crea un código para manejar la cola Q0, otro para Q1 y otro para Q2, se construyó un componente genérico Queue. El Scheduler simplemente instancia la veces que necesita esa misma cola, reutilizando la lógica de listas enlazadas de manera limpia.

- En la capa apliación separar entre Scheduler y Gestor de E/S (I/O Manager) garantiza la mantenibilidad del proyecto; dado que en la implementación solo un proceso a la vez puede acceder a la CPU, mientras que los dispositivos de E/S son paralelos. Si se combina ambos en el Scheduler, genera "Código Espagueti", mezclando reglas de prioridades de CPU con tiempos de espera de E/S. 

- En la capa de infraestructura el simulador permite que el sistema evolucione a nuevas interfaces o comunicaciones solo es necesario crear nuevos adaptadores.

### Patrones y principios

| Patrón o principio | ¿Dónde se aplica? | ¿Por qué se aplicó? |
|---|---|---|
| **Arquitectura en capas (Layered Architecture)** | Separación `domain/` → `application/` → `infrastructure/` | Establece una regla de dependencia unidireccional: las capas externas conocen a las internas, nunca al revés. Esto reduce el acoplamiento y permite modificar una capa sin afectar a las demás. |
| **Principio de responsabilidad única (SRP)** | Cada archivo tiene un único motivo de cambio: `process.c` cambia solo si cambia el modelo de proceso; `csv_adapter.c` cambia solo si cambia el formato de exportación | Facilita el mantenimiento pues un cambio en el formato de salida no obliga a tocar el algoritmo de planificación, y viceversa. |
| **Ocultamiento de información / encapsulamiento (Information Hiding, Parnas)** | Tipos opacos `Process`, `Queue`, `MLFQScheduler`, `IOManager` | Protege la representación interna de cada estructura. Permite modificar la implementación interna (por ejemplo, cambiar la lista enlazada de `Queue` por un arreglo) sin romper el código cliente, ya que este solo conoce la cabecera pública declarada en el `.h`. |
| **Principio de inversión de dependencias (DIP), aplicado parcialmente** | La infraestructura (`csv_adapter`, `console_adapter`) depende de las abstracciones del dominio y la aplicación (`Queue`, `MLFQScheduler`), y no al revés | El algoritmo de planificación no incluye ningún encabezado de `infrastructure/`. Esto significa que el "core" del sistema es independiente del mecanismo de entrega de resultados, cumpliendo la idea de que los detalles (I/O, formato) dependen de las políticas de negocio, no viceversa. |
| **Principio abierto/cerrado (OCP), aplicado en el borde del sistema** | Adaptadores independientes en `infrastructure/` | Agregar un nuevo formato de exportación implica *crear* un adaptador nuevo, no modificar el existente ni el scheduler. El sistema está parcialmente "cerrado a modificación, abierto a extensión" en su capa externa. |
| **Patrón Adapter (GoF)** | `csv_adapter.c`, `console_adapter.c` | Ambos módulos traducen una estructura interna (`Queue` de procesos terminados, `SchedulerTraceEntry`) a un formato externo específico (texto CSV, tabla de consola), sin que el resto del sistema conozca ese formato. |
| **Composition Root** | `main.c` | Concentrar en un único punto la creación e interconexión de todos los objetos evita que las dependencias se resuelvan de forma implícita y dispersa por el código, mejorando la trazabilidad de qué depende de qué. |
| **Máquina de estados finita (FSM)** | `enum ProcessState` y sus transiciones controladas por `process_set_state` | Modelar el ciclo de vida de un proceso como estados discretos con transiciones explícitas es la forma estándar de representar el comportamiento de un proceso en teoría de sistemas operativos, y facilita verificar que no ocurran transiciones inválidas. |
| **Inyección de dependencias (parcial, vía parámetros)** | `FILE* output` en `export_execution_trace_console` | Aunque el proyecto no usa un contenedor de inyección de dependencias, el hecho de pasar el destino de salida como parámetro (en vez de una constante global `stdout`) es la forma más simple de DI en C, y habilita pruebas sin depender de la consola real. |
