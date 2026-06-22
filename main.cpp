#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map> //para analizar tradenetwork
#include <chrono> //para medir tiempo
#include <algorithm>
#include <iomanip>
#include "grafo.h"
#include "MedidasCentralidad.h"

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

// TEST DEGREE CENTRALITY: imprime los 5 nodos con mayor centralidad
void DCentralityTop(const Grafo& grafo, const std::vector<double>& centralidad, const std::string& titulo) {
    std::cout << "\n-  TOP: " << titulo << "   -\n";
    int V = grafo.obtener_num_vertices();
    if (V == 0) return;

    //Crear un vector de pares (índice_interno, valor_centralidad)
    std::vector<std::pair<int, double>> ranking;
    for (int i = 0; i < V; ++i) {
        ranking.push_back({i, centralidad[i]});
    }

    //Ordenar de mayor a menor según el valor de centralidad
    std::sort(ranking.begin(), ranking.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second; //compara los valores double
    });

    //Imprime
    int limite = std::min(5, V);
    for (int i = 0; i < limite; ++i) {
        std::string nombre_nodo = grafo.mapear_a_externo(ranking[i].first);
        std::cout << i + 1 << ". Nodo: " << nombre_nodo 
                  << " | Puntaje: " << std::fixed << std::setprecision(4) << ranking[i].second << "\n";
    }
}

int main() {
    std::cout << "===== PRUEBAS Y TEST COMPLETO =====\n\n";

    //Cargar Dataset Yeast 
    std::cout << "===================================" << std::endl;
    std::cout << "-> Cargando dataset Yeast (Proteinas): " << std::endl;
    Grafo yeast(false); //No dirigido

    auto inicioYeast = std::chrono::high_resolution_clock::now();
    if (cargarDataset(yeast, "datasets/yeast.edgelist", false)) { 
        MedidasCentralidad analizadorYeast(yeast);
        auto finYeast = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoYeast = finYeast - inicioYeast;

        std::cout << "   [OK] Dataset Yeast cargado con exito." << std::endl;
        std::cout << "   [*] Numero de vertices (V): " << yeast.obtener_num_vertices() << std::endl;
        std::cout << "   [*] Numero de aristas (E) : " << yeast.obtener_num_aristas() << std::endl;
        std::cout << "   [*] Tiempo de construccion: " << tiempoYeast.count() << " ms\n" << std::endl;

        std::cout << "-- D E G R E E  C E N T R A L I T Y -- " << std::endl;

        // CALCULA DEGREE CENTRALITY
        std::vector<double> degree_yeast = analizadorYeast.calcular_degree_centrality();
        DCentralityTop(yeast, degree_yeast, "Degree Centrality (Hubs Proteicos)"); //imprime
    }

    //Cargar Dataset Trade Network 
    std::cout << "===================================" << std::endl;
    std::cout << "\n-> Cargando dataset Trade Network (Economia): " << std::endl;
    Grafo trade(true); // -> Dirigido

    auto inicioTrade = std::chrono::high_resolution_clock::now();
    if (cargarDataset(trade, "datasets/2000.net", true)) { 
        MedidasCentralidad analizadorTrade(trade);
        auto finTrade = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoTrade = finTrade - inicioTrade;

        std::cout << "   [OK] Dataset Trade Network cargado con exito." << std::endl;
        std::cout << "   [*] Numero de vertices (V): " << trade.obtener_num_vertices() << std::endl;
        std::cout << "   [*] Numero de aristas (E) : " << trade.obtener_num_aristas() << std::endl;
        std::cout << "   [*] Tiempo de construccion: " << tiempoTrade.count() << " ms\n" << std::endl;

        std::cout << "-- D E G R E E  C E N T R A L I T Y -- " << std::endl;

        // TEST    DEGREE   CENTRALITY
        // Calcular In-Degree (Importadores) y Out-Degree (Exportadores)
        std::vector<double> in_degree_trade = analizadorTrade.calcular_in_degree_centrality();
        std::vector<double> out_degree_trade = analizadorTrade.calcular_out_degree_centrality();

        DCentralityTop(trade, in_degree_trade, "In-Degree (Mayores Importadores/Destinos)");
        DCentralityTop(trade, out_degree_trade, "Out-Degree (Mayores Exportadores/Origenes)");
    }

    std::cout << "===================================" << std::endl;

    return 0;
}

/*

Para compilar:
g++ -O3 main.cpp grafo.cpp MedidasCentralidad.cpp -o analisis_redes

Ejecutar:
./analisis_redes

*/