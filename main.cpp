#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map> //para analizar tradenetwork
#include <chrono> //para medir tiempo
#include "grafo.h"

/*
Función auxiliar que lee strings de manera inteligente y posee control de 
estados para discriminar entre un formato de lista de aristas (Yeast) y el formato Pajek (Tradenetwork)
*/
bool cargarDataset(Grafo& grafo, const std::string& rutaArchivo, bool tienePeso) {
    std::ifstream archivo(rutaArchivo);
    
    if (!archivo.is_open()) {
        std::cerr << "ERROR: No se pudo abrir el archivo en la ruta: " << rutaArchivo << std::endl;
        return false;
    }

    std::string linea;
    bool leyendo_aristas = !tienePeso; //Si no tiene peso (caso de Yeast), procesa directo

    //Diccionario temporal para guardar el mapeo numérico de Pajek utilizado por tradenetwotk
    std::unordered_map<std::string, std::string> traductor_pajek;

    while (std::getline(archivo, linea)) {
        //Ignorar líneas vacías o símbolos que pueden generar fallas
        if (linea.empty() || linea[0] == '#' || linea[0] == '%') {
            continue;
        }

        //Control de cabeceras de sección en formato Pajek (*Vertices o *Arcs/*Edges)
        if (linea[0] == '*') {
            if (linea.find("Vertices") != std::string::npos) {
                leyendo_aristas = false;
            } else {
                leyendo_aristas = true;
            }
            continue;
        }

        std::stringstream ss(linea);
        
        if (!leyendo_aristas && tienePeso) {
            //Para PAJEK: Leer catálogo de vértices y limpiar comillas (ej, 4 "ARG" guarda "ARG")
            std::string indice_num, codigo_pais;
            if (ss >> indice_num >> codigo_pais) {
                if (codigo_pais.front() == '"' && codigo_pais.back() == '"') {
                    codigo_pais = codigo_pais.substr(1, codigo_pais.length() - 2);
                }
                traductor_pajek[indice_num] = codigo_pais;
                grafo.agregar_vertice(codigo_pais); //registramos el país con sus 3 letras reales
            }
        } 
        else {
            //FASE ARISTAS, Carga de aristas 
            std::string u, v;
            double peso;

            if (tienePeso) {
                //Formato Trade Network - mapeo Pajek
                if (ss >> u >> v >> peso) {
                    std::string pais_u = traductor_pajek[u].empty() ? u : traductor_pajek[u];
                    std::string pais_v = traductor_pajek[v].empty() ? v : traductor_pajek[v];
                    grafo.agregar_arista(pais_u, pais_v, peso);
                }
            } else {
                //Formato Yeast estándar - nodos directos sin peso
                if (ss >> u >> v) {
                    grafo.agregar_arista(u, v); 
                }
            }
        }
    }

    archivo.close();
    return true;
}

int main() {
    std::cout << "===== PRUEBAS Y TEST COMPLETO =====\n\n";

    //Cargar Dataset Yeast 
    std::cout << "-> Cargando dataset Yeast (Proteinas): " << std::endl;
    Grafo yeast(false); //No dirigido

    auto inicioYeast = std::chrono::high_resolution_clock::now();
    if (cargarDataset(yeast, "datasets/yeast.edgelist", false)) { 
        auto finYeast = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoYeast = finYeast - inicioYeast;

        std::cout << "   [OK] Dataset Yeast cargado con exito." << std::endl;
        std::cout << "   [*] Numero de vertices (V): " << yeast.obtener_num_vertices() << std::endl;
        std::cout << "   [*] Numero de aristas (E) : " << yeast.obtener_num_aristas() << std::endl;
        std::cout << "   [*] Tiempo de construccion: " << tiempoYeast.count() << " ms\n" << std::endl;
    }

    //Cargar Dataset Trade Network 
    std::cout << "-> Cargando dataset Trade Network (Economia): " << std::endl;
    Grafo trade(true); // -> Dirigido

    auto inicioTrade = std::chrono::high_resolution_clock::now();
    if (cargarDataset(trade, "datasets/2000.net", true)) { 
        auto finTrade = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoTrade = finTrade - inicioTrade;

        std::cout << "   [OK] Dataset Trade Network cargado con exito." << std::endl;
        std::cout << "   [*] Numero de vertices (V): " << trade.obtener_num_vertices() << std::endl;
        std::cout << "   [*] Numero de aristas (E) : " << trade.obtener_num_aristas() << std::endl;
        std::cout << "   [*] Tiempo de construccion: " << tiempoTrade.count() << " ms\n" << std::endl;
    }

    std::cout << "===================================" << std::endl;
    return 0;
}

/*

Para compilar:
g++ -O3 main.cpp Grafo.cpp -o analisis_redes

Ejecutar:
./analisis_redes

*/