///////////////////////////////////////////////

//Créé par Olivier Bouchard et Nicolas Cinq-Mars
//Cours 8INF342	
//Laboratoire #2
//Date: 2 novembre 2023

///////////////////////////////////////////////

// Librairies à inclure

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <semaphore>
#include <chrono>
#include <mutex>

// Définition du namespace
using namespace std;

// Déclaration des variables

const int ITEMS = 6;                            // Nombre de ressources différentes
bool resources[ITEMS] = {1, 1, 1, 1, 1, 1}; 	// Tableau avec les disponibilités de chacune des ressources
mutex tbMutex[ITEMS]; 						    // Tableau de mutex. Chaque case[i] représente un mutex pour chacune des ressources

// Déclaration de la variable de quantitée

atomic_int total = 0;

// Création d'une struct qui permettera de stocker les données de chacun des threads

struct ThreadInfo{

    // Attributs
    string objectName;
    thread::id TID;
    int resource1;
    int resource2;
    chrono::time_point<std::chrono::high_resolution_clock> startTime;
    chrono::time_point<std::chrono::high_resolution_clock> endTime;
    int time; // en milliseconds

    // Méthode

    int calculateTime(){ // Permet de calculer la durée totale d'un thread
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
        time = static_cast<int>(duration.count());
        return time;
    }
};

// -- Méthode de création des objets -- //

void buildObject(string objectName, int resource1, int resource2){

    // Création de l'objet

    ThreadInfo t;
    t.objectName = objectName;
    t.TID = this_thread::get_id();
    t.resource1 = resource1;
    t.resource2 = resource2;
    t.startTime = chrono::high_resolution_clock::now();

    // Affichage s'il nous manque une resource ou des ressources

    if(!resources[resource1] && !resources[resource2])
        cout << "[" << t.TID << "] " << objectName << " en attente de " << resource1 << " et " << resource2 << endl;
    else if(!resources[resource1])
        cout << "[" << t.TID << "] " << objectName << " en attente de " << resource1 << endl;
    else if(!resources[resource2])
        cout << "[" << t.TID << "] " << objectName << " en attente de " << resource2 << endl;

    // Possibilité 1 : le mutex de la ressource 1 est bloqué, donc le programme attend
    // Possibilité 2 : le mutex de la ressource 1 est libre, mais le 2e ne l'est pas, donc le programme attend
    // Possibilité 3 : les deux mutex sont libres, donc le programme ne s'arrête pas

    tbMutex[resource1].lock();
    tbMutex[resource2].lock();

    // Lorsque le thread obtient le droit de poursuivre son execution, il prend les 2 ressources

    resources[resource1] = false;
    resources[resource2] = false;

    // On procède à la fabrication

    this_thread::sleep_for(chrono::milliseconds(200));

    t.endTime = chrono::high_resolution_clock::now();

    cout << "[" << t.TID << "] " << objectName << " a été construit!" << "(" << t.calculateTime() << " ms)" << endl;

    // Ajustement de la quantité
    total++;

    // On remet à disposition les 2 ressources

    resources[resource1] = true;
    resources[resource2] = true;

    // On unlock les mutex afin de débloquer les possibles thread en attente

    tbMutex[resource1].unlock();
    tbMutex[resource2].unlock();
}

int main(){

    auto start = chrono::high_resolution_clock::now();

    thread th1(buildObject,"coupe", 0, 1);
    thread th2(buildObject,"epee", 1, 2);
    thread th3(buildObject,"chandelier", 2, 3);
    thread th4(buildObject, "bague", 3, 4);
    thread th5(buildObject,"table", 4, 5);
    thread th6(buildObject,"porte", 0, 5);

    th1.join();
    th2.join();
    th3.join();
    th4.join();
    th5.join();
    th6.join();

    auto end = chrono::high_resolution_clock::now();

    cout << "On a créer " << total << " objets au total!" << endl;
    cout << "Le temps d'execution total est de " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms!" << endl;
}