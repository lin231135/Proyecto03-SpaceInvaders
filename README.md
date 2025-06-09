# Space Invaders

![image1](https://github.com/user-attachments/assets/e21e80f8-9e25-47a2-8163-c9143b59324a)
![image2](https://github.com/user-attachments/assets/a2c96152-4769-41f7-aef5-c9245648a78f)


## Descripción
Una versión de consola del clásico videojuego Space Invaders, implementada utilizando hilos (pthreads) y mecanismos de sincronización (mutexes y semáforos) para crear una experiencia interactiva y concurrente.

## Desarrolladores
- Dulce Ambrosio - 231143
- Javier Linares - 231135
- Marcos Ambrocio - 231140
- Daniel Chet - 231177
- Cindy Gualim - 21226

## Características
- **Interfaz ASCII colorida**: Gráficos basados en caracteres con efectos visuales.
- **Multihilo**: Implementación concurrente para el movimiento de la nave, invasores y disparos.
- **Dos modos de juego**:
  - 1 jugador vs invasores: Control manual de la nave.
  - Computadora vs invasores: La IA controla la nave automáticamente.
- **Niveles de dificultad configurables**:
  - Fácil
  - Medio
  - Difícil
  - Lluvia de Fuego
- **Sistema de puntuación**: Gana puntos por cada invasor eliminado.
- **Efectos visuales**: Títulos parpadeantes y marcos coloridos.

## Requisitos del sistema
- Sistema operativo Windows
- Compilador C++ con soporte para pthreads
- Consola con soporte para caracteres UTF-8

## Cómo jugar
1. Usa las teclas **A** y **D** para mover la nave horizontalmente.
2. Presiona **Espacio** para disparar.
3. Elimina a todos los invasores antes de que lleguen al fondo de la pantalla.
4. Evita los disparos de los invasores.
5. El juego termina cuando eliminas a todos los invasores (victoria) o cuando un invasor te alcanza (derrota).

## Implementación técnica
El juego utiliza un enfoque concurrente con 4 hilos principales:
- **Hilo de entrada del jugador**: Gestiona los controles y movimientos de la nave.
- **Hilo de movimiento de balas**: Maneja los disparos del jugador y las colisiones.
- **Hilo de invasores**: Controla el movimiento sincronizado de los invasores.
- **Hilo de disparos enemigos**: Gestiona los ataques aleatorios de los invasores.

### Sincronización
- Se utiliza un **mutex** para proteger el acceso a la pantalla y evitar condiciones de carrera.
- Un **semáforo** controla el movimiento coordinado de los invasores.

## Compilación y ejecución
Para compilar el juego:
```bash
g++ -o SpaceInvaders SpaceInvaders.cpp -lpthread
```

Para ejecutar:
```bash
./SpaceInvaders
```

## Estructura del proyecto
```
├── SpaceInvaders.cpp  # Archivo principal del juego
└── .vscode/           # Configuración de VS Code
    └── launch.json    # Configuración de depuración
```

## Niveles de dificultad
- **Fácil**: Invasores y disparos lentos.
- **Medio**: Velocidad moderada.
- **Difícil**: Invasores y disparos rápidos.
- **Lluvia de Fuego**: ¡Velocidad extrema! Para jugadores experimentados.

## Licencia
Este proyecto fue desarrollado como parte del curso CC3086 - Programación de Microprocesadores en la Universidad del Valle de Guatemala.

---

*"Space Invaders - Defiende la Tierra de la invasión alienígena"*
