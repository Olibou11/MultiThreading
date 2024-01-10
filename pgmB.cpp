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
#include <iomanip>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <semaphore>
#include <chrono>
#include <mutex>
#include <sstream>
#include <fstream>

// Définition du namespace
using namespace std;

// Déclaration des variables afin de produire les threads pour les ressources

const int ITEMS = 6;							// Nombre de ressources différentes
bool resources[ITEMS] = {1, 1, 1, 1, 1, 1}; 	// Tableau avec les disponibilités de chacune des ressources
mutex tbMutex[ITEMS]; 							// Tableau de mutex. Chaque case[i] représente un mutex pour chacune des ressources

// Déclaration des variables utile pour la génération du fichier .txt

ofstream file;
mutex fileMutex;                                // Permet de protéger la ressource "file"

// Déclaration des variables surveillant les quantitées

int coupes = 0;
int epees = 0;
int chandeliers = 0;
int bagues = 0;
int tables = 0;
int portes = 0;

atomic_int total = 0;

// Déclaration de la variable responsable de compter les threads actifs
// Dans ce laboratoire, cette dernière ne devrait pas dépasser 6
// atomic est protégé, donc il n'a pas besoin de mutex

atomic_int threadCounter;

// Création d'une struct qui permet de stocker toutes les données d'un thread

string timePointToString(chrono::time_point<std::chrono::high_resolution_clock> time);

struct ThreadInfo{

    // Attributs
    string objectName;
    thread::id TID;
    int resource1;
    int resource2;
    chrono::time_point<std::chrono::high_resolution_clock> startTime;
    chrono::time_point<std::chrono::high_resolution_clock> endTime;
    int time; // (En ms)

    // Méthodes

    void calculateTime(){ // Permet de calculer la durée totale d'un thread
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
        time = static_cast<int>(duration.count());
    }

    string getInfoThread(){ // .toString() pour faciliter l'écriture dans le fichier .txt
        stringstream ss;
        ss << setw(20) << left << TID << setw(20) << left << objectName << setw(20) << left << timePointToString(startTime) << setw(20) << left << timePointToString(endTime) << time << "ms";
        return ss.str();
    }
};

// -- Méthodes permettant l'écriture dans le fichier .txt -- //

string timePointToString(chrono::time_point<std::chrono::high_resolution_clock> time){
    using sc = std::chrono::system_clock;
    std::time_t t = sc::to_time_t(time);
    char buf[20];
    strftime(buf, 20, "%H:%M:%S", localtime(&t)); // (Heure:Minutes:Secondes)
    return string(buf);
}

void writeThreadInfo(ThreadInfo t){
    fileMutex.lock();
    file << t.getInfoThread() << "\n";
    fileMutex.unlock();
}

// -- Méthode de création des objets -- //

void buildObject(ThreadInfo t){

    // Setup des infos sur le thread

    t.TID = this_thread::get_id();
    t.startTime = chrono::high_resolution_clock::now();

    // Affichage s'il nous manque une resource ou des ressources

    if(!resources[t.resource1] && !resources[t.resource2])
        std::cout << "[" << t.TID << "] " << t.objectName << " en attente de " << t.resource1 << " et " << t.resource2 << endl;
    else if(!resources[t.resource1])
        std::cout << "[" << t.TID << "] " << t.objectName << " en attente de " << t.resource1 << endl;
    else if(!resources[t.resource2])
        std::cout << "[" << t.TID << "] " << t.objectName << " en attente de " << t.resource2 << endl;

    // Possibilité 1 : le mutex de la ressource 1 est bloqué, donc le programme attend
    // Possibilité 2 : le mutex de la ressource 1 est libre, mais le 2e ne l'est pas, donc le programme attend
    // Possibilité 3 : les deux mutex sont libres, donc le programme ne s'arrête pas

    tbMutex[t.resource1].lock();
    tbMutex[t.resource2].lock();

    // Lorsque le thread obtient le droit de poursuivre son execution, il prend les 2 ressources

    resources[t.resource1] = false;
    resources[t.resource2] = false;

    // On procède à la fabrication

    std::cout << "[" << t.TID << "] " << t.objectName << " en création!" << endl;

    this_thread::sleep_for(chrono::milliseconds(200));

    // Ajustement des quantités

    if(t.objectName == "coupe"){
        coupes++;
    }
    else if (t.objectName == "epee"){
        epees++;
    }
    else if (t.objectName == "chandelier"){
        chandeliers++;
    }
    else if (t.objectName == "bague"){
        bagues++;
    }
    else if (t.objectName == "table"){
        tables++;
    }
    else if (t.objectName == "porte"){
        portes++;
    }

    total++;

    // On remet à disposition les 2 ressources

    resources[t.resource1] = true;
    resources[t.resource2] = true;

    // On unlock les mutex afin de débloquer les possibles thread en attente

    tbMutex[t.resource1].unlock();
    tbMutex[t.resource2].unlock();

    // Set de la valeur de fin du thread et calcul de sa durée totale

    t.endTime = chrono::high_resolution_clock::now();
    t.calculateTime();

    // Écriture du nouvel objet dans le .txt
    writeThreadInfo(t);

    // Décrémentation du compteur
    threadCounter--;
}

int main(){

    // Initialisation du fichier .txt

    file.open("resultat.txt", ios::trunc);
    file << setw(20) << left << "[TID]" << setw(20) << left << "[Nom]" << setw(20) << left << "[Début]" << setw(20) << left << "[Fin]" << "[Durée Totale]\n";

    // Déclaration des variables

    int randomObject;
    ThreadInfo t;

    stringstream details;

    srand(time(0));// Seed random

    auto start = chrono::high_resolution_clock::now();

    // Boucle tant et aussi longtemps que les conditions ne sont pas respectées

    do {
        if (threadCounter < 6) { // Assure que 6 threads sont actifs

            randomObject = 1 + (rand() % (ITEMS)); // (1 à 6)

            switch (randomObject){
                case 1:
                    t.objectName = "coupe";
                    t.resource1 = 0;
                    t.resource2 = 1;
                    break;

                case 2:
                    t.objectName = "epee";
                    t.resource1 = 1;
                    t.resource2 = 2;
                    break;

                case 3:
                    t.objectName = "chandelier";
                    t.resource1 = 2;
                    t.resource2 = 3;
                    break;

                case 4:
                    t.objectName = "bague";
                    t.resource1 = 3;
                    t.resource2 = 4;
                    break;

                case 5:
                    t.objectName = "table";
                    t.resource1 = 4;
                    t.resource2 = 5;
                    break;

                case 6:
                    t.objectName = "porte";
                    t.resource1 = 0;
                    t.resource2 = 5;
                    break;
            }

            // Lancement du thread

            thread th(buildObject, t);
            th.detach();

            // Incrémentation du compteur de thread
            threadCounter++;
        }
    } while (coupes < 100 || epees < 100 || chandeliers < 100 || bagues < 100 || tables < 100 || portes < 100);

    auto end = chrono::high_resolution_clock::now();

    // Setup de l'output : sommaire

    details << "\nOn a créé " << total << " objets au total!\n";
    details << "\tCoupes :\t" << coupes << "\n";
    details << "\tEpees : \t" << epees << "\n";
    details << "\tChandeliers :\t" << chandeliers << "\n";
    details << "\tBagues :\t" << bagues << "\n";
    details << "\tTables :\t" << tables << "\n";
    details << "\tPortes :\t" << portes << "\n";
    details << "Le temps d'execution total des threads est de " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms!\n";

    // Écriture du sommaire dans la console et le fichier .txt

    std::cout << details.str();
    file << details.str();

    file.close();
}