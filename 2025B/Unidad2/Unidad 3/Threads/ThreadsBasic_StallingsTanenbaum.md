# Problemas comunes de sincronización de hilos y sus soluciones

| **Problema de sincronización** | **Descripción** | **Ejemplo típico** | **Solución / Mecanismo** | **Comentarios** |
|---------------------------------|------------------|--------------------|--------------------------|-----------------|
| **Condiciones de carrera (Race Conditions)** | Dos o más hilos acceden y modifican un recurso compartido al mismo tiempo, provocando resultados inconsistentes. | Dos hilos incrementan una variable global. | `Mutex (pthread_mutex_t)`, `spinlock`, `atomic operations`. | El mutex asegura exclusión mutua: solo un hilo puede acceder a la sección crítica. |
| **Interbloqueo (Deadlock)** | Dos o más hilos quedan bloqueados indefinidamente esperando recursos que el otro posee. | Hilo A bloquea recurso 1 y espera recurso 2, mientras hilo B hace lo opuesto. | **Evitar bloqueo circular**: orden global de adquisición de locks; usar `trylock()` con timeout. | Se debe liberar siempre los recursos en orden inverso al que se adquirieron. |
| **Inanición (Starvation)** | Un hilo nunca obtiene acceso al recurso porque otros hilos lo acaparan constantemente. | Hilo de baja prioridad que nunca ejecuta. | Planificación justa (`pthread_cond_signal`), prioridad adecuada o semáforos con política FIFO. | Causado por políticas de scheduling injustas. |
| **Inversión de prioridad (Priority Inversion)** | Un hilo de baja prioridad bloquea a otro de alta prioridad al retener un recurso compartido. | Un hilo lento posee un mutex que necesita un hilo más importante. | **Herencia de prioridad** (Priority Inheritance). | Sistemas embebidos o de tiempo real implementan esta política. |
| **Busy waiting (Espera activa)** | Un hilo revisa continuamente una condición sin liberar CPU, desperdiciando recursos. | Uso de un bucle `while(flag == 0);`. | Condiciones y señales (`pthread_cond_wait`, `pthread_cond_signal`). | Evitar bucles activos, usar mecanismos de sincronización del sistema. |
| **Lectores–Escritores (Readers–Writers Problem)** | Múltiples hilos leen un recurso, pero las escrituras deben ser exclusivas. | Acceso concurrente a un archivo o estructura de datos. | `pthread_rwlock_t` (bloqueo de lectura/escritura). | Permite múltiples lectores o un solo escritor, nunca ambos. |
| **Acceso a variables no atómicas** | Las operaciones aparentemente simples (como `x++`) no son atómicas. | Dos hilos incrementan `x` al mismo tiempo. | Tipos atómicos (`stdatomic.h`) o mutex. | Evita resultados impredecibles o corrupción de datos. |
| **Despertar perdido (Lost Wakeup)** | Un hilo pierde una señal porque no estaba esperando en el momento correcto. | Hilo A hace `signal()` antes de que hilo B haga `wait()`. | Usar siempre `pthread_cond_wait` dentro de un bucle `while(condición)`. | Verificar la condición bajo el mismo mutex. |
| **Sección crítica demasiado amplia** | Bloquear más de lo necesario reduce el paralelismo y el rendimiento. | Mutex cubre código no crítico. | Limitar la sección crítica al mínimo. | Más locks → más latencia, menos eficiencia. |
| **Fugas de recursos (Resource leaks)** | Mutex, semáforos o hilos no liberados adecuadamente. | `pthread_mutex_lock()` sin `pthread_mutex_unlock()`. | Usar estructuras RAII o bloques `finally`/`cleanup`. | Especialmente importante en aplicaciones largas o servidores. |

---

## Resumen de mecanismos comunes

| **Mecanismo** | **Uso principal** | **Ejemplo (POSIX)** |
|----------------|------------------|---------------------|
| **Mutex (Mutual Exclusion Lock)** | Excluir acceso concurrente a secciones críticas. | `pthread_mutex_lock`, `pthread_mutex_unlock` |
| **Semáforo** | Controlar acceso a recursos limitados (n contadores). | `sem_init`, `sem_wait`, `sem_post` |
| **Condición (Condition Variable)** | Sincronizar eventos entre hilos. | `pthread_cond_wait`, `pthread_cond_signal` |
| **RWLock (Reader–Writer Lock)** | Permitir múltiples lectores o un escritor. | `pthread_rwlock_rdlock`, `pthread_rwlock_wrlock` |
| **Atomic Operations** | Evitar mutexes en operaciones simples. | `atomic_fetch_add`, `atomic_store` |
| **Barrier** | Sincronizar hilos en un punto común antes de continuar. | `pthread_barrier_wait` |


