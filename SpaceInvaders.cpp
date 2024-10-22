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