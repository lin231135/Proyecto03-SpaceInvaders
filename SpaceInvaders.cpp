/*
*----------------------------------------
* Proyecto3 - Space Invaders
* ---------------------------------------
* UNIVERSIDAD DEL VALLE DE GUATEMALA
* CC3086 - Programacion de Microprocesadores
* Autor: Dulce Ambrosio - 231143
* Autor: Javier Linares - 231135
* Autor: Marcos Ambrocio - 231140
* Autor: Daniel Chet - 231177
* Autor: Cindy Gualim - 21226
* ---------------------------------------
* Haciendo uso de pthreads y mutexes se 
* realizó una version a consola del 
* videojuego Space Invaders.
*----------------------------------------
*/

#include <iostream> // Biblioteca para entrada y salida estándar
#include <windows.h> // Biblioteca para funciones específicas de Windows
#include <vector> // Biblioteca para utilizar vectores
#include <conio.h> // Biblioteca para entrada de teclado
#include <cstdlib> // Biblioteca para funciones generales como rand()
#include <pthread.h> // Biblioteca para manejo de hilos
#include <semaphore.h> // Biblioteca para manejo de semáforos

using namespace std; // Para evitar escribir std:: antes de cada función de la biblioteca estándar

// Declaraciones de funciones
void gotoxy(int x, int y);
void dibujarBordes(int x_min, int x_max, int y_min, int y_max);
void dibujarNave(int x, int y);
void borrarNave(int x, int y);
void disparar(vector<pair<int, int>>& balas, int x, int y);
void moverBalas(vector<pair<int, int>>& balas, vector<pair<int, int>>& enemigos, int y_min, int& puntuacion);
void dispararEnemigos(vector<pair<int, int>>& disparosEnemigos, vector<pair<int, int>>& enemigos);
void moverDisparosEnemigos(vector<pair<int, int>>& disparosEnemigos, int x, int y, bool& jugadorMuerto);
void dibujarEnemigos(vector<pair<int, int>>& enemigos);
void moverEnemigos(vector<pair<int, int>>& enemigos, int& direccion, int x_min, int x_max, int y_max);
bool colision(int x1, int y1, int x2, int y2);
void mostrarPuntuacion(int puntuacion);

// Variables globales para sincronización
pthread_mutex_t screen_mutex; // Mutex para proteger la pantalla
sem_t invader_sem; // Semáforo para controlar el movimiento de los invasores

// Estructura para almacenar datos del juego
struct GameData {
    int x, y; // Posición de la nave
    vector<pair<int, int>> balas; // Vector de balas disparadas
    vector<pair<int, int>> enemigos; // Vector de posiciones de enemigos
    vector<pair<int, int>> disparosEnemigos; // Vector de disparos enemigos
    int direccion; // Dirección de movimiento de los enemigos
    int puntuacion; // Puntuación del jugador
    bool jugadorMuerto; // Estado del jugador
    bool modoComputadora; // Modo de juego (manual o automático)
    int velocidadInvader; // Velocidad de los invasores
    int velocidadDisparosEnemigos; // Velocidad de los disparos enemigos
    bool FinalizarHilos; // Bandera para finalizar los hilos
};

// Función para manejar la entrada del jugador
void* playerInput(void* arg) {
    GameData* data = (GameData*)arg;
    while (!data->jugadorMuerto && !data->FinalizarHilos) {
        if (!data->modoComputadora && _kbhit()) { // Si no está en modo automático y hay una tecla presionada
            char tecla = _getch(); // Captura la tecla presionada
            pthread_mutex_lock(&screen_mutex); // Bloquea el mutex para proteger la pantalla
            if (tecla == 'a' && data->x > 6) { // Mover a la izquierda con 'A'
                borrarNave(data->x, data->y);
                data->x--;
                dibujarNave(data->x, data->y);
            }
            if (tecla == 'd' && data->x < 49) { // Mover a la derecha con 'D'
                borrarNave(data->x, data->y);
                data->x++;
                dibujarNave(data->x, data->y);
            }
            if (tecla == ' ') { // Disparar
                disparar(data->balas, data->x, data->y);
            }
            pthread_mutex_unlock(&screen_mutex); // Desbloquea el mutex
        }
        // Modo computadora
        if (data->modoComputadora) {
            pthread_mutex_lock(&screen_mutex);
            borrarNave(data->x, data->y);
            if (data->x > 6 && rand() % 2 == 0) data->x--;
            else if (data->x < 49) data->x++;
            dibujarNave(data->x, data->y);
            if (rand() % 3 == 0) disparar(data->balas, data->x, data->y);
            pthread_mutex_unlock(&screen_mutex);
        }
        Sleep(50); // Espera 50 ms
    }
    return NULL;
}

// Función para mover las balas
void* moveBullets(void* arg) {
    GameData* data = (GameData*)arg;
    while (!data->jugadorMuerto && !data->FinalizarHilos) {
        pthread_mutex_lock(&screen_mutex);
        moverBalas(data->balas, data->enemigos, 5, data->puntuacion);
        pthread_mutex_unlock(&screen_mutex);
        Sleep(100); // Espera 100 ms
    }
    return NULL;
}

// Función para mover los invasores
void* moveInvaders(void* arg) {
    GameData* data = (GameData*)arg;
    while (!data->jugadorMuerto && !data->FinalizarHilos) {
        sem_wait(&invader_sem); // Espera en el semáforo
        pthread_mutex_lock(&screen_mutex);
        moverEnemigos(data->enemigos, data->direccion, 5, 50, 20);
        pthread_mutex_unlock(&screen_mutex);
        Sleep(data->velocidadInvader); // Ajuste de velocidad con el nivel de dificultad
    }
    return NULL;
}

// Función para disparar enemigos
void* enemyFire(void* arg) {
    GameData* data = (GameData*)arg;
    while (!data->jugadorMuerto && !data->FinalizarHilos) {
        pthread_mutex_lock(&screen_mutex);
        dispararEnemigos(data->disparosEnemigos, data->enemigos);
        moverDisparosEnemigos(data->disparosEnemigos, data->x, data->y, data->jugadorMuerto);
        pthread_mutex_unlock(&screen_mutex);
        Sleep(data->velocidadDisparosEnemigos); // Ajuste de velocidad con el nivel de dificultad
    }
    return NULL;
}

// Función para establecer el color de texto en la consola
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Función para dibujar un marco en la consola
void dibujarMarco(int ancho, int alto) {
    for (int i = 0; i < alto; i++) {
        for (int j = 0; j < ancho; j++) {
            if (i == 0 || i == alto - 1 || j == 0 || j == ancho - 1) {
                cout << "#";  // Marco del menú
            } else {
                cout << " ";
            }
        }
        cout << endl;
    }
}

// Función para mostrar el título con parpadeo y efectos arcade
void mostrarTituloParpadeante() {
    string titulo = R"(
   _____ ____  ___   ____________   _____   ___    _____    ____  __________  _____
  / ___// __ \/   | / ____/ ____/  /  _/ | / / |  / /   |  / __ \/ ____/ __ \/ ___/
  \__ \/ /_/ / /| |/ /   / __/     / //  |/ /| | / / /| | / / / / __/ / /_/ /\__ \ 
 ___/ / ____/ ___ / /___/ /___   _/ // /|  / | |/ / ___ |/ /_/ / /___/ _, _/___/ / 
/____/_/   /_/  |_\____/_____/  /___/_/ |_/  |___/_/  |_/_____/_____/_/ |_|/____/   
)";

    int colores[] = {12, 14, 10, 11, 13}; // Colores: rojo, amarillo, verde, cian, púrpura
    int numColores = sizeof(colores) / sizeof(colores[0]);

    for (int i = 0; i < 10; i++) {  // Hace que el título parpadee 10 veces
        setColor(colores[i % numColores]);
        system("cls");
        dibujarMarco(85, 6);
        setColor(colores[i % numColores]);
        cout << titulo << endl;
        Sleep(200);  // Ajustar la velocidad del parpadeo para hacerlo más fluido
    }

    setColor(7);
}

// Función para mostrar las instrucciones del juego
void mostrarInstrucciones() {
    mostrarTituloParpadeante();
    setColor(11);  // Color cian brillante
    dibujarMarco(85, 6);
    // Mostrar instrucciones centradas
    cout << "\n\n\t\t     Bienvenido a Space Invaders!\n\n";
    setColor(10); // Verde
    cout << "\t\tInstrucciones del juego:\n";
    setColor(15); // Blanco brillante
    cout << "\t\t1. Usa las teclas [A] y [D] para mover la nave.\n";
    cout << "\t\t2. Usa 'espacio' para disparar.\n";
    cout << "\t\t3. Elimina a los invasores antes de que lleguen al final.\n";
    cout << "\t\t4. ¡Evita los disparos enemigos!\n";
    cout << "\t\t5. Juega en modo 1 jugador o computadora vs invasores.\n";
    setColor(12); // Rojo
    cout << "\n\t\tPresiona cualquier tecla para continuar...\n";
    setColor(7);  // Restaurar color a blanco predeterminado
    _getch();
}

// Función para seleccionar el modo de juego
int seleccionarModo() {
    system("cls");
    setColor(14); // Amarillo
    dibujarMarco(80, 10);
    setColor(15); // Blanco brillante
    cout << "\n\n\t\tSelecciona el modo de juego:\n";
    cout << "\t\t1. 1 jugador vs invasores.\n";
    cout << "\t\t2. Computadora vs invasores.\n";
    cout << "\t\tSelecciona 1 o 2: ";
    char seleccion = _getch();
    return seleccion == '2';
}

// Función para seleccionar la dificultad del juego
void seleccionarDificultad(int &velocidadInvader, int &velocidadDisparosEnemigos) {
    system("cls");
    setColor(14); // Amarillo para el marco
    dibujarMarco(80, 12);
    setColor(15); // Blanco brillante para el texto
    cout << "\n\n\t\tSelecciona el nivel de dificultad:\n";

    setColor(10); // Verde para 'Fácil'
    cout << "\t\t1. Facil\n";

    setColor(14); // Amarillo para 'Medio'
    cout << "\t\t2. Medio\n";

    setColor(12); // Rojo para 'Difícil'
    cout << "\t\t3. Dificil\n";

    setColor(13); // Púrpura para 'Lluvia de Fuego'
    cout << "\t\t4. Lluvia de Fuego\n";

    setColor(15); // Restaurar blanco para la entrada
    cout << "\t\tSelecciona 1, 2, 3 o 4: ";
    char seleccion = _getch();

    // Asignar las velocidades en función de la dificultad seleccionada
    switch (seleccion) {
        case '1': // Facil
            velocidadInvader = 1000;
            velocidadDisparosEnemigos = 800;
            break;
        case '2': // Medio
            velocidadInvader = 800;
            velocidadDisparosEnemigos = 700;
            break;
        case '3': // Dificil
            velocidadInvader = 500;
            velocidadDisparosEnemigos = 100;
            break;
        case '4': // Lluvia de Fuego
            velocidadInvader = 100;
            velocidadDisparosEnemigos = 50;
            break;
        default:
            velocidadInvader = 1000;
            velocidadDisparosEnemigos = 800;
            break;
    }
}

// Función para mover el cursor a una posición específica en la consola
void gotoxy(int x, int y) {
    HANDLE consola = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(consola, pos);
}

// Función para dibujar los bordes del área de juego
void dibujarBordes(int x_min, int x_max, int y_min, int y_max) {
    for (int x = x_min; x <= x_max; x++) {
        gotoxy(x, y_min - 1); cout << "░";
        gotoxy(x, y_max); cout << "░";
    }
    for (int y = y_min; y <= y_max; y++) {
        gotoxy(x_min, y); cout << "░";
        gotoxy(x_max, y); cout << "░";
    }
}

// Función para dibujar la nave en la posición especificada
void dibujarNave(int x, int y) {
    gotoxy(x, y); cout << "🛦";
}

// Función para borrar la nave de la posición especificada
void borrarNave(int x, int y) {
    gotoxy(x, y); cout << " ";
}

// Función para disparar balas desde la nave
void disparar(vector<pair<int, int>>& balas, int x, int y) {
    balas.push_back({x, y - 1});
}

// Función para mover las balas y verificar colisiones
void moverBalas(vector<pair<int, int>>& balas, vector<pair<int, int>>& enemigos, int y_min, int& puntuacion) {
    for (size_t i = 0; i < balas.size(); i++) {
        int x = balas[i].first;
        int y = balas[i].second;
        gotoxy(x, y); cout << " ";
        if (y > y_min) {
            balas[i].second--;
            gotoxy(x, balas[i].second); cout << "▲";
        } else {
            balas.erase(balas.begin() + i);
            i--;
        }
        for (size_t j = 0; j < enemigos.size(); j++) {
            if (colision(x, balas[i].second, enemigos[j].first, enemigos[j].second)) {
                gotoxy(enemigos[j].first, enemigos[j].second); cout << " ";
                enemigos.erase(enemigos.begin() + j);
                puntuacion += 10;
                mostrarPuntuacion(puntuacion);
                break;
            }
        }
    }
}

// Función para disparar desde los enemigos
void dispararEnemigos(vector<pair<int, int>>& disparosEnemigos, vector<pair<int, int>>& enemigos) {
    if (!enemigos.empty()) {
        // Limitar el número de disparos activos
        if (disparosEnemigos.size() < 5) { // Limita a 5 disparos activos
            // Aumentar el valor del módulo para reducir la frecuencia de disparo
            if (rand() % 50 == 0) { // Cambia 50 a un número mayor para disparar menos frecuentemente
                int index = rand() % enemigos.size();
                disparosEnemigos.push_back({enemigos[index].first, enemigos[index].second + 1});
            }
        }
    }
}

// Función para mover los disparos enemigos y verificar colisiones
void moverDisparosEnemigos(vector<pair<int, int>>& disparosEnemigos, int x, int y, bool& jugadorMuerto) {
    for (size_t i = 0; i < disparosEnemigos.size(); i++) {
        int ex = disparosEnemigos[i].first;
        int ey = disparosEnemigos[i].second;
        gotoxy(ex, ey); cout << " ";
        if (ey < 19) {
            disparosEnemigos[i].second++;
            gotoxy(ex, disparosEnemigos[i].second); cout << "!";
        } else {
            disparosEnemigos.erase(disparosEnemigos.begin() + i);
            i--;
        }
        if (colision(ex, disparosEnemigos[i].second, x, y)) {
            jugadorMuerto = true;
        }
    }
}

// Función para dibujar los enemigos en sus posiciones actuales
void dibujarEnemigos(vector<pair<int, int>>& enemigos) {
    for (auto& enemigo : enemigos) {
        gotoxy(enemigo.first, enemigo.second); cout << "M";
    }
}

// Función para mover los enemigos y cambiar de dirección si es necesario
void moverEnemigos(vector<pair<int, int>>& enemigos, int& direccion, int x_min, int x_max, int y_max) {
    bool cambiarDireccion = false;
    for (auto& enemigo : enemigos) {
        gotoxy(enemigo.first, enemigo.second); cout << " ";
        enemigo.first += direccion;
        if (enemigo.first <= x_min+1 || enemigo.first >= x_max-1) {
            cambiarDireccion = true;
        }
    }
    if (cambiarDireccion) {
        direccion = -direccion;
        for (auto& enemigo : enemigos) {
            enemigo.second++;
            if (enemigo.second >= y_max) return;
        }
    }
    dibujarEnemigos(enemigos);
}

// Función para verificar colisiones entre dos objetos
bool colision(int x1, int y1, int x2, int y2) {
    return (x1 == x2 && y1 == y2);
}

// Función para mostrar la puntuación en la pantalla
void mostrarPuntuacion(int puntuacion) {
    setColor(10);  // 10 es el código para verde en la consola
    gotoxy(5, 2);  // Mostrar en una posición fija
    cout << "Puntuacion: " << puntuacion;
    setColor(7);   // Restaurar a blanco (el color por defecto)
}

// Función principal del juego
int main() {
    system("chcp 65001"); // Configurar la consola para UTF-8
    dibujarMarco(10, 5);

    while (true) {
        mostrarInstrucciones();
        bool modoComputadora = seleccionarModo();

        // Seleccionar la dificultad
        int velocidadInvader, velocidadDisparosEnemigos;
        seleccionarDificultad(velocidadInvader, velocidadDisparosEnemigos);

        // Inicializar datos del juego
        GameData data = {20, 19, {}, {{10, 7}, {15, 7}, {20, 7}, {25, 7}, {30, 7}, {35, 7}}, {}, 1, 0, false, modoComputadora, velocidadInvader, velocidadDisparosEnemigos, false};

        // Crear hilos para manejar diferentes aspectos del juego
        pthread_t input_thread, bullet_thread, invader_thread, enemy_fire_thread;
        pthread_mutex_init(&screen_mutex, NULL);
        sem_init(&invader_sem, 0, 1);

        system("cls");
        dibujarBordes(5, 50, 5, 20);
        dibujarNave(data.x, data.y);
        dibujarEnemigos(data.enemigos);

        pthread_create(&input_thread, NULL, playerInput, &data);
        pthread_create(&bullet_thread, NULL, moveBullets, &data);
        pthread_create(&invader_thread, NULL, moveInvaders, &data);
        pthread_create(&enemy_fire_thread, NULL, enemyFire, &data);

        while (!data.jugadorMuerto) {
            sem_post(&invader_sem);
            Sleep(100);

            // Verificar si todos los enemigos han sido eliminados
            pthread_mutex_lock(&screen_mutex);
            if (data.enemigos.empty()) {
                data.jugadorMuerto = true;  // Terminar el juego
                pthread_mutex_unlock(&screen_mutex);
                break; // Salir del bucle si no hay enemigos
            }
            pthread_mutex_unlock(&screen_mutex);
        }

        // Señal de hilos para finalizarlos
        data.FinalizarHilos = true;

        pthread_join(input_thread, NULL);
        pthread_join(bullet_thread, NULL);
        pthread_join(invader_thread, NULL);
        pthread_join(enemy_fire_thread, NULL);

        gotoxy(5, 21);
        if (data.jugadorMuerto) {
            if (data.enemigos.empty()) {
                cout << "¡Victoria! Has eliminado a todos los enemigos." << endl;
            } else {
                cout << "¡Game Over! Los enemigos te han alcanzado." << endl;
            }
        }

        pthread_mutex_destroy(&screen_mutex);
        sem_destroy(&invader_sem);

        cout << "Quieres jugar de nuevo? (s/n): ";
        char respuesta = _getch();
        if (respuesta != 's' && respuesta != 'S') {
            break;
        }

        // Reiniciar variables de estado
        data.puntuacion = 0;
        data.jugadorMuerto = false;
        data.balas.clear();  // Limpiar las balas anteriores
        data.disparosEnemigos.clear();  // Limpiar disparos enemigos
        data.enemigos = {{10, 7}, {15, 7}, {20, 7}, {25, 7}, {30, 7}, {35, 7}};  // Reestablecer enemigos
        data.x = 20;  // Reestablecer posición inicial de la nave
        data.y = 19;
        data.FinalizarHilos = false; // Resetear la bandera de finalizacion de los hilos
    }
    return 0;
}