#include <iostream>
#include <windows.h>
#include <vector>
#include <conio.h>
#include <cstdlib>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

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

pthread_mutex_t screen_mutex;
sem_t invader_sem;

struct GameData {
    int x, y;
    vector<pair<int, int>> balas;
    vector<pair<int, int>> enemigos;
    vector<pair<int, int>> disparosEnemigos;
    int direccion;
    int puntuacion;
    bool jugadorMuerto;
    bool modoComputadora;
    int velocidadInvader;
    int velocidadDisparosEnemigos;
    bool FinalizarHilos; // Flag to terminate threads
};

void* playerInput(void* arg) {
    GameData* data = (GameData*)arg;
    while (!data->jugadorMuerto) {
        if (!data->modoComputadora && _kbhit()) {
            char tecla = _getch();
            pthread_mutex_lock(&screen_mutex);
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
            pthread_mutex_unlock(&screen_mutex);
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
        Sleep(50);
    }
    return NULL;
}

void* moveInvaders(void* arg) {
    GameData* data = (GameData*)arg;
    while (!data->jugadorMuerto) {
        sem_wait(&invader_sem);
        pthread_mutex_lock(&screen_mutex);
        moverEnemigos(data->enemigos, data->direccion, 5, 50, 20);
        pthread_mutex_unlock(&screen_mutex);
        Sleep(data->velocidadInvader);  // Ajuste de velocidad con el nivel de dificultad
    }
    return NULL;
}

void* moveBullets(void* arg) {
    GameData* data = (GameData*)arg;
    while (!data->jugadorMuerto) {
        pthread_mutex_lock(&screen_mutex);
        moverBalas(data->balas, data->enemigos, 5, data->puntuacion);
        pthread_mutex_unlock(&screen_mutex);
        Sleep(100);
    }
    return NULL;
}

void* enemyFire(void* arg) {
GameData* data = (GameData*)arg;
while (!data->jugadorMuerto) {
    pthread_mutex_lock(&screen_mutex);
    dispararEnemigos(data->disparosEnemigos, data->enemigos);
    moverDisparosEnemigos(data->disparosEnemigos, data->x, data->y, data->jugadorMuerto);
    pthread_mutex_unlock(&screen_mutex);
    Sleep(data->velocidadDisparosEnemigos); // Ajuste de velocidad con el nivel de dificultad
    }
return NULL;
}

void moverBalas(vector<pair<int, int>>& balas, vector<pair<int, int>>& enemigos, int y_min, int& puntuacion) {
    for (size_t i = 0; i < balas.size(); i++) {
        int x = balas[i].first;
        int y = balas[i].second;
        gotoxy(x, y); cout << " ";
        if (y > y_min) {
            balas[i].second--;
            gotoxy(x, balas[i].second); cout << " ▲ ";
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

bool colision(int x1, int y1, int x2, int y2) {
    return (x1 == x2 && y1 == y2);
}

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

void dibujarNave(int x, int y) {
    gotoxy(x, y); cout << "🛦";
}

void borrarNave(int x, int y) {
    gotoxy(x, y); cout << " ";
}

void disparar(vector<pair<int, int>>& balas, int x, int y) {
    balas.push_back({x, y - 1});
}

void dibujarEnemigos(vector<pair<int, int>>& enemigos) {
    for (auto& enemigo : enemigos) {
        gotoxy(enemigo.first, enemigo.second); cout << "M";
    }
}

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

void mostrarPuntuacion(int puntuacion) {
    setColor(10);  // 10 es el código para verde en la consola
    gotoxy(5, 2);  // Mostrar en una posición fija
    cout << "Puntuacion: " << puntuacion;
    setColor(7);   // Restaurar a blanco (el color por defecto)
}

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

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

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

void mostrarTituloParpadeante() {
    string titulo = R"(
   _____ ____  ___   ____________   _____   ___    _____    ____  __________  _____
  / ___// __ \/   | / ____/ ____/  /  _/ | / / |  / /   |  / __ \/ ____/ __ \/ ___/
  \__ \/ /_/ / /| |/ /   / __/     / //  |/ /| | / / /| | / / / / __/ / /_/ /\__ \ 
 ___/ / ____/ ___ / /___/ /___   _/ // /|  / | |/ / ___ |/ /_/ / /___/ _, _/___/ / 
/____/_/   /_/  |_\____/_____/  /___/_/ |_/  |___/_/  |_/_____/_____/_/ |_|/____/   
)";

    int colores[] = {12, 14, 10, 11, 13}; // Colores: rojo, amarillo, verde, cian, púrpura
    int numColores = sizeof(colores) / sizeof(colores[0]);

    for (int i = 0; i < 10; i++) {  // Hace que el título parpadee 10 veces
        setColor(colores[i % numColores]);
        system("cls");
        dibujarMarco(85, 6);
        setColor(colores[i % numColores]);
        cout << titulo << endl;
        Sleep(200);  // Ajustar la velocidad del parpadeo para hacerlo más fluido
    }

    setColor(7);
}


