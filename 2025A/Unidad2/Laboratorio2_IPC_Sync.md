# Sistemas Operativos
# Universidad Santiago de Cali, 2025A
# Laboratorio #2: Comunicación y Sincronización entre Procesos

##   **"Sistema de pedidos entre clientes y cocina"**

###  Descripción del problema

Desarrollar una aplicación en C que simule la interacción entre **múltiples procesos clientes** (que hacen pedidos) y un **proceso cocina** (que los atiende), mediante mecanismos de **comunicación entre procesos (IPC)**: FIFO o memoria compartida.

El objetivo es modelar un entorno de trabajo donde se presenten **problemas reales de sincronización**, como:
- Acceso concurrente a recursos compartidos,
- Espera activa o bloqueo,
- Pérdida o sobreescritura de datos si no se sincroniza correctamente.

---

##  Contexto del sistema simulado

- Varios procesos cliente (usuarios) envían pedidos de comida: `{"cliente_id": 1, "pedido": "Pizza"}`.
- Un proceso cocina (servidor) recibe los pedidos y los procesa en orden.
- El sistema debe funcionar como una **cola de pedidos**.
- El cliente recibe confirmación de que su pedido fue recibido o preparado.

Se debe garantizar:
- Que **los pedidos no se pierdan**.
- Que **la cocina atienda un pedido a la vez**.
- Que los clientes no sobrescriban el pedido del otro.
- Que no haya lectura de datos incompletos.

---

##  Opciones de implementación

###  Opción A: **FIFO bidireccional**

- Usar **dos archivos FIFO**: uno para que los clientes escriban pedidos (`fifo_pedidos`), otro para que la cocina envíe confirmaciones (`fifo_respuestas`).
- Requiere implementar control para que los procesos no interfieran al escribir simultáneamente.

###  Opción B: **Memoria compartida**

- Usar un segmento de memoria con una **estructura compartida tipo cola**.
- Si se necesita sincronización: usar **flags, turnos u otro tipo de variable** para gestionar el acceso de múltiples procesos cliente.

---

##  Requisitos técnicos

- El número de clientes puede variar (paralelos).
- La cocina procesa pedidos uno por uno, en el orden recibido (FIFO real).
- El cliente debe recibir confirmación de que su pedido fue **recibido** y luego que fue **preparado**.
- Implementar medidas de **sincronización explícitas**.

---

##  Posibles estructuras de datos

### Para memoria compartida:
```c
typedef struct {
    int cliente_id;
    char pedido[64];
    int estado; // 0: vacío, 1: recibido, 2: preparado
} Pedido;

Pedido pedidos[10]; // cola circular de pedidos
```

---

##  Bibliografía sugerida

1. Tanenbaum, A. S. – *Modern Operating Systems*.  
2. Silberschatz, A. – *Operating System Concepts*.  
3. [Linux Programmer’s Manual – shmget, shmat, mkfifo, select](https://man7.org/linux/man-pages/)  

---

## Preguntas de análisis y discusión

1. **¿Cómo evitar que dos procesos cliente escriban al mismo tiempo en la misma posición de la cola?**
2. **¿Qué pasaría si la cocina intenta leer un pedido mientras el cliente aún lo está escribiendo?**
3. **¿Qué método de sincronización implementarían para que los procesos cliente esperen su confirmación?**
4. **¿Cómo se comportaría el sistema si la cocina es más lenta que los clientes? ¿Qué riesgos hay?**
5. **¿Qué diferencias prácticas ven entre usar FIFO y memoria compartida para este problema?**
6. **¿Cómo podrían escalar este sistema para tener más de un proceso cocina trabajando en paralelo sin procesar el mismo pedido dos veces?**

---

##  Entregables

- Implementación en C usando:
  - Opción A: FIFO bidireccional
  - Opción B: Memoria compartida con sincronización
- Informe que:
  - Describa las decisiones de diseño.
  - Responda las preguntas propuestas.
  - Analice el rendimiento y la robustez del sistema.
