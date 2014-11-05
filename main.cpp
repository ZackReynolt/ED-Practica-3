#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <list>

#include "tinythread.h"
#include "millisleep.h"
#include "Request.h"
#include "Song.h"

using namespace std;
using namespace tthread;

class RadioApp {
    vector<int> canciones;
    
    thread threadReproducirCanciones;
    mutex semaforo;
    bool pinchar;

    static void hebraReproducirCanciones(void *arg) {
        RadioApp *radioApp = static_cast<RadioApp *> (arg);
        radioApp->reproducirCanciones();
    }

public:

    RadioApp() : threadReproducirCanciones(hebraReproducirCanciones, this) {
        pinchar = true;
    }

    void reproducirCanciones() {
        do {
            // Esperar 1 seg. hasta ver si hay alguna canción nueva en la lista de peticiones
            millisleep(1000); 
            
            while (pinchar && !canciones.empty()) {
                // Sacar una canción y reproducirla
                semaforo.lock();
                int cancion = canciones[0];
                canciones.erase(canciones.begin());
                semaforo.unlock();

                cout << "Reproduciendo canción " << cancion << " ..." << endl;
                // Simular el tiempo de reproducción de la canción (entre 2 y 12 seg.)
                millisleep(2000 + 1000 * (rand() % 10));
            }
            
        } while (pinchar);
    }

    void solicitarCanciones() {
        int cancion;
        
        // Pedir canciones hasta que se introduce "0"
        do {
            cin >> cancion;

            semaforo.lock();
            canciones.push_back(cancion);
            semaforo.unlock();

        } while (cancion != 0);

        pinchar = false;
        threadReproducirCanciones.join();
    }
};

void CargarListaCaciones(std::list<Song> &lSongs) {
    try { 
        fstream fi("canciones.txt");
        string line, atribute[3];

        while (!fi.eof()) {
            getline(fi, line);
            stringstream lineStream(line);

            int i = 0;
            while (getline(lineStream, atribute[i], '|')) {
                i++;
            };

            int cod = atoi(atribute[0].c_str());
            Song s(cod, atribute[1], atribute[2]);
            lSongs.push_back(s);
        }
        fi.close();
        lSongs.pop_back();
    } catch (std::exception &e) {
        cout << "No se ha podido abrir el archivo";
    }
}

int main(int argc, char** argv) {
    RadioApp app;
    list<Song> lSongs;
    
    CargarListaCaciones(lSongs);
    
    for (std::list<Song>::iterator it=lSongs.begin(); it!=lSongs.end(); ++it) {
        cout << it->GetCode() << " - " << it->GetTitle() << endl;        
    }
    cout << "Introduce el código de la canción deseada (1-500)." << 
            "\nIntroduce 0 para salir." << endl;
    
    app.solicitarCanciones();

    return 0;
}