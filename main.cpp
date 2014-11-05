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

/**
 * 
 * @param lRequest  Lista de listas de peticiones
 * @param codigo    Código de la canción que queremos buscar
 * @return          Puntero al dato encontrado o nulo si no es el caso
 * @description     Este método busca en la lista la canción pedida y devuelve un 
 *                  puntero que apunta al dato de la petición solicitada.
 */
Request* BuscarDato(list< list<Request> > &lRequest, int codigo) {
    list< list<Request> >::iterator i;
    list<Request>::iterator j;
    Request *r;
    
    for (i = lRequest.begin(); i != lRequest.end(); ++i){
        for (j = i->begin(); j != i->end(); ++j) {
            if (j->getCod() == codigo) {
                return r = j.operator ->();
            }
        }
    }
    return 0;
}

void AnadePeticion(std::list< list<Request> > &lRequest, int peticion) {
    Request *req = BuscarDato(lRequest, peticion);
    std::list<list <Request> >::iterator it=lRequest.begin();
    std::list<Request>::iterator jt=it->begin();
    Request aux;
    bool encontrado = false;
    
    // Si se encuentra entre las peticiones existentes
    if (req) {
        //aumentar número de peticiones
        req->setNRequest(1);
        
        //cambiar prioridad
        aux=*req;
        
        while (it!=lRequest.end() && !encontrado) {
            while (jt != it->end() && !encontrado) {
                if (jt->getCod() == req->getCod()) {
                    it->erase(jt);
                    encontrado = true;
                }
                ++jt;
            }
            ++it;
        }
        if (encontrado) {
            //Buscar si hay una lista con prioridad n+1
            if (it!=lRequest.end()) {
                if (it->begin()->getNRequest()==aux.getNRequest()) {
                    it->push_back(aux);
                } else {
                    list<Request> l;
                    l.push_back(aux);
                    lRequest.insert(it,l);
                }
            } else {
                list<Request> l;
                l.push_back(aux);
                lRequest.push_back(l);
            }
        }
    } else {
        // Crea el objeto Request con 1 petición por defecto
        Request Req(peticion);
        
        // Hay que saber si hay lista de prioriad 1
        if (it->begin()->getNRequest() == 1) {
            it->push_back(Req);
        } else {
            list<Request> l;
            l.push_back(Req); 
            lRequest.push_front(l);         // Al meterla al principio ya 
                                            // tiene prioridad 1 (baja)
        }

    }
      
}

int main(int argc, char** argv) {
    RadioApp app;
    list<Song> lSongs;
    list< list<Request> > lRequest;
    
    list<Request> liR;
    
//    for (int i = 0; i < 5; ++i) {
//        for (int j = 0; j < 5; ++j) {
//            Request r(j);
//            liR.push_back(r);
//        }
//        lRequest.push_back(liR);
        AnadePeticion(lRequest, 4);
        AnadePeticion(lRequest, 4);
        AnadePeticion(lRequest, 2);
        AnadePeticion(lRequest, 2);
        AnadePeticion(lRequest, 1);
        AnadePeticion(lRequest, 5);
        
//    }
    
    Request *r = BuscarDato(lRequest, 8);
    if (r)
        cout << "He encontrado la canción con código: " << r->getCod() << " con " << r->getNRequest() << " peticiones" << endl;
    else
        cout << "El dato no se ha encontrado..." << endl;
    
    //Mostrar lista peticiones
    list< list<Request> >::iterator i;
    list<Request>::iterator j;
    
    for (i = lRequest.begin(); i != lRequest.end(); ++i) {
        cout << "Prioridad " << i->begin()->getNRequest() << endl;
        for (j = i->begin(); j != i->end(); ++j) {
            cout << j->getCod() << " - " <<  j->getNRequest() << endl;
        }
    }
    
    
    
    int peticion;

    CargarListaCaciones(lSongs);
    
    //Mostrar lista de canciones
//    for (std::list<Song>::iterator it=lSongs.begin(); it!=lSongs.end(); ++it) {
//        cout << it->GetCode() << " - " << it->GetTitle() << endl;        
//    }
    
    //Añadir peticiones
    cout << "Introduce el código de la canción deseada (1-500)." << 
            "\nIntroduce 0 para salir." << endl;
//    
//    cin >> peticion;
//    
//    AnadePeticion (lRequest, peticion);
    
    
    app.solicitarCanciones();
    
    return 0;
}