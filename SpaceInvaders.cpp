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