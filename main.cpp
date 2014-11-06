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
    vector<Request> vRequest;   //Vector de canciones reproducidas
    
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
                Request r(cancion);
                
                vRequest.push_back(r);       
            }
            
        } while (pinchar);
    }

    void solicitarCanciones() {
        int cancion;
        
        // Pedir canciones hasta que se introduce "0"
        do {
            
            cin >> cancion;

            if (vRequest.size() != 0){
                int j = 0;
                bool reproducida = false;
                while (j < vRequest.size() && j < 100 && !reproducida) {
                    if (vRequest[j].getCod() == cancion) {
                        cout << "La canción " << cancion 
                                <<  " fue de las últimas 100 reproducidas" << endl;
                        reproducida=true;
                    }
                    j++;
                }
                if (!reproducida) {
                    semaforo.lock();
                    canciones.push_back(cancion);
                    semaforo.unlock();
                }
            } else {
                semaforo.lock();
                canciones.push_back(cancion);
                semaforo.unlock();
            }
            
            
            
            

        } while (cancion != 0);

        pinchar = false;
        threadReproducirCanciones.join();
    }
};

/**
 * 
 * @param lSongs    Lista con las canciones (por referencia)
 * @return          void
 * @description     Este procedimiento carga en la lista de canciones todas las
 *                  canciones que se encuentran en el archivo de canciones para
 *                  tal fin ("canciones.txt") en el directorio del proyecto.
 */
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
list<Request>::iterator BuscarDato(list< list<Request> > &lRequest, int codigo) {
    list< list<Request> >::iterator i;
    list<Request>::iterator j;
    Request *r;
    
    for (i = lRequest.begin(); i != lRequest.end(); ++i){
        for (j = i->begin(); j != i->end(); ++j) {
            if (j->getCod() == codigo) {
                return j;
            }
        }
    }
    return lRequest.end()->end();
}

/**
 * 
 * @param lRequest  Lista de listas de peticiones
 * @param peticion  Código de la canción que queremos añadir
 * @return          void
 * @description     Este procedimiento añade a la lista de peticiones la canción
 *                  que le indiquemos como segundo parámetro de entrada.
 */
void AnadePeticion(std::list< list<Request> > &lRequest, int peticion) {
    std::list<Request>::iterator data = BuscarDato(lRequest, peticion);
    bool insertado = false;
    
    if (data != lRequest.end()->end()) {
        Request r = *data;
        r.setNRequest(1);
        
        list< list<Request> >::iterator it = lRequest.begin();
        while (it != lRequest.end() && !insertado) {
            it->remove(*data);
            if (it->empty())
                it = lRequest.erase(it);
            if (it != lRequest.end() && it->begin()->getNRequest() == r.getNRequest()){
                it->push_back(r);
                insertado = true;
            }
            ++it;
        }
        if (!insertado) {
                list<Request> l;
                l.push_back(r);
                lRequest.push_back(l);
                insertado = true;
            }
    } else {
        Request r(peticion);
        
        if (!lRequest.empty() && lRequest.begin()->begin()->getNRequest() == 1)
            lRequest.begin()->push_back(r);
        else {
            list<Request> l;
            l.push_back(r);
            lRequest.push_back(l);
        }
    }
    lRequest.sort();
}

/**
 * 
 * @param lSongs    Lista de canciones del fichero canciones.txt
 * @param letra     String que identifica el tipo de búsqueda 
 * @return          void
 * @description     Este procedimiento busca mediante artista (A) o título (T)
 *                  los códigos de las canciones de la lista de canciones y los
 *                  muestra por pantalla.
 */
void BuscaCodigo(std::list<Song> &lSongs, string letra) {
    cout << "\nBuscando: " << endl;
    std::list<Song>::iterator it=lSongs.begin();
    string objetivo;
    
    cin >> objetivo;
    
    while (it!=lSongs.end()) {
        std::size_t found;
        if (letra == "A")
            found = it->GetArtist().find(objetivo);
        else
            found = it->GetTitle().find(objetivo);
        
        if (found!=std::string::npos) {
            cout << it->GetCode() << " - " << 
                    it->GetArtist() << " - " << 
                    it->GetTitle() << endl;
        }
        it++;
    }

}

/**
 * 
 * @param lRequest  Lista de listas de peticiones
 * @param vRequest  Vector con las canciones ya reproducidas
 * @return          Booleano que indica si se puede reproducir o no una petición
 * @description     Esta función mira las 100 últimas canciones reproducidas 
 *                  (se deben ir añadiendo al vector para dicha función) e
 *                  informa de si puede reproducirse una canción o no.
 */
bool PuedeReproducirPet (std::list< list<Request> > &lRequest, vector<Request> &vRequest,
        int peticion) {
    bool puede = true;
    
    //Ver si la petición es una de las últimas 100 reproducidas
    if (vRequest.size() != 0){
        int j = 0;
        while (j < vRequest.size() && j < 100) {
            if (vRequest[j].getCod() == peticion)
                puede = false;
            j++;
        }
    }
    
    return puede;
}


int main(int argc, char** argv) {
    list<Song> lSongs;
    list< list<Request> > lRequest;

    /////////////////
    //   PRUEBAS   //
    /////////////////
    

    AnadePeticion(lRequest, 4);
    AnadePeticion(lRequest, 4);
    AnadePeticion(lRequest, 4);
    AnadePeticion(lRequest, 4);
    AnadePeticion(lRequest, 2);
    AnadePeticion(lRequest, 2);
    AnadePeticion(lRequest, 2);
    AnadePeticion(lRequest, 1);
    AnadePeticion(lRequest, 5);
    AnadePeticion(lRequest, 5);
    AnadePeticion(lRequest, 3);

    //Mostrar lista peticiones
    list< list<Request> >::iterator i;
    list<Request>::iterator j;
    
    for (i = lRequest.begin(); i != lRequest.end(); ++i) {
        cout << "Prioridad " << i->begin()->getNRequest() << endl;
        for (j = i->begin(); j != i->end(); ++j) {
            cout << j->getCod() << " - " <<  j->getNRequest() << endl;
        }
    }

    // ========================================================================
        
    
    ////////////////////////
    // PROGRAMA PRINCIPAL //
    ////////////////////////
    
    int peticion;

    CargarListaCaciones(lSongs);
    
    //======//
    // MENU //
    //======//
    int opcion;         // opción seleccionada en el menú
    RadioApp app;
    
    cout << "\n¡Bienvenido a Radionauta v3!" << endl;
    cout << "Solicita aquí tu canción preferida. \n" << endl;
    while (opcion != 6) {
        
        cout << "Opciones:" << endl;
        cout << "1. Añadir petición." << endl;
        cout << "2. Reproducir canciones." << endl;
        cout << "3. Mostrar canciones reproducidas." << endl;
        cout << "4. Mostrar canciones disponibles." << endl;
        cout << "5. Mostrar lista de peticiones." << endl;
        cout << "6. Exit" << endl;
        cout << "Por favor, introduce el número deseado y pulsa 'Enter': ";
        
        // Evitar malas entradas de teclado
        cin >> opcion;
        while (opcion < 1 || opcion > 6) {
            cin.clear();
            cin.ignore(100, '\n');
            cout << "Por favor, introduce un número entre 1 y 6: ";
            cin >> opcion;
        }
        
        switch (opcion) {
            case 1: {
                cout << "\nAñadir o buscar canción." << endl;
                cout << "C - Código canción, A - Artista, T - Título." << endl;
                string letra;
    
                cin >> letra;
    
                while (letra != "A" && letra != "T" && letra != "C") {
                    cin.clear();
                    cin.ignore(100, '\n');
                    cout << "\nPor favor, 'C' para Código, 'A' para Artista o 'T' para Título: ";
                    cin >> letra;
                }
                
                if (letra == "C") {
                    cout << "\nIntroduce el código deseado: ";
                    cin >> peticion;
                    AnadePeticion(lRequest, peticion);
                } else {
                    BuscaCodigo(lSongs, letra);
                    cout << "\nIntroduce el código deseado: ";
                    cin >> peticion;
                    AnadePeticion(lRequest, peticion);
                } 
                break;
            }
            case 2: {
                //Reproducir canción
                cout << "\n\nIntroduce el código de la canción a reproducir." << endl;
                cout << "Introduce '0' en cualquier momento para interrumpir la "
                        "reproducción" << endl;
                app.solicitarCanciones();
                break;
            }
            case 3:
                cout << "Canciones reproducidas:" << endl;
                //Mostrar lista de canciones reproducidas  
                for (int i = 0; i < app.vRequest.size(); ++i) {
                    cout << app.vRequest[i].getCod() << endl;
                }
                break;
            case 4: 
                cout << "Canciones disponibles:" << endl;
                //Mostrar lista de canciones
                for (std::list<Song>::iterator it=lSongs.begin(); it!=lSongs.end(); ++it) {
                    cout << it->GetCode() << " - " << it->GetTitle() << endl;        
                }
                break;
            case 5: 
                cout << "Lista de peticiones:" << endl;
                //Mostrar lista peticiones
                list< list<Request> >::iterator i;
                list<Request>::iterator j;

                for (i = lRequest.begin(); i != lRequest.end(); ++i) {
                    cout << "Prioridad " << i->begin()->getNRequest() << endl;
                    for (j = i->begin(); j != i->end(); ++j) {
                        cout << j->getCod() << " - " <<  j->getNRequest() << endl;
                    }
                }
                break;
        }
    }
    
    return 0;
}