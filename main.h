#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include "grafo.h"
#include "MedidasCentralidad.h"

// ==========================================
//    TELEMETRÍA DE MEMORIA (Hardware-Aware)
// ==========================================
#ifdef __linux__
#include <sys/resource.h>
long obtener_memoria_kb() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    // En Linux, ru_maxrss se reporta en Kilobytes
    return usage.ru_maxrss; 
}
#else
long obtener_memoria_kb() {
    return 0; 
}
#endif

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
    bool leyendo_aristas = !tienePeso; 

    std::unordered_map<std::string, std::string> traductor_pajek;

    while (std::getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#' || linea[0] == '%') continue;

        if (linea[0] == '*') {
            leyendo_aristas = (linea.find("Vertices") == std::string::npos);
            continue;
        }

        std::stringstream ss(linea);
        
        if (!leyendo_aristas && tienePeso) {
            std::string indice_num, codigo_pais;
            if (ss >> indice_num >> codigo_pais) {
                if (codigo_pais.front() == '"' && codigo_pais.back() == '"') {
                    codigo_pais = codigo_pais.substr(1, codigo_pais.length() - 2);
                }
                traductor_pajek[indice_num] = codigo_pais;
                grafo.agregar_vertice(codigo_pais); 
            }
        } 
        else {
            std::string u, v;
            double peso;

            if (tienePeso) {
                if (ss >> u >> v >> peso) {
                    std::string pais_u = traductor_pajek[u].empty() ? u : traductor_pajek[u];
                    std::string pais_v = traductor_pajek[v].empty() ? v : traductor_pajek[v];
                    grafo.agregar_arista(pais_u, pais_v, peso);
                }
            } else {
                if (ss >> u >> v) {
                    grafo.agregar_arista(u, v); 
                }
            }
        }
    }
    archivo.close();
    return true;
}

// Imprime los 5 nodos con mayor centralidad
void DCentralityTop(const Grafo& grafo, const std::vector<double>& centralidad, const std::string& titulo) {
    std::cout << "\n-  TOP: " << titulo << "   -\n";
    int V = grafo.obtener_num_vertices();
    if (V == 0) return;

    std::vector<std::pair<int, double>> ranking;
    ranking.reserve(V);
    for (int i = 0; i < V; ++i) ranking.push_back({i, centralidad[i]});

    std::sort(ranking.begin(), ranking.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second;
    });

    int limite = std::min(5, V);
    for (int i = 0; i < limite; ++i) {
        std::string nombre_nodo = grafo.mapear_a_externo(ranking[i].first);
        std::cout << i + 1 << ". Nodo: " << nombre_nodo 
                  << " | Puntaje: " << std::fixed << std::setprecision(4) << ranking[i].second << "\n";
    }
}

// Imprime el TOP 5 de nodos ordenados por su K-Shell (coreness).
void DCoreTop(const Grafo& grafo, const std::vector<int>& coreness, const std::string& titulo) {
    std::cout << "\n-  TOP: " << titulo << "   -\n";
    int V = grafo.obtener_num_vertices();
    if (V == 0) return;

    std::vector<std::pair<int, int>> ranking;
    ranking.reserve(V);
    for (int i = 0; i < V; ++i) ranking.push_back({i, coreness[i]});

    std::sort(ranking.begin(), ranking.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
        return a.second > b.second;
    });

    int limite = std::min(5, V);
    for (int i = 0; i < limite; ++i) {
        std::string nombre_nodo = grafo.mapear_a_externo(ranking[i].first);
        std::cout << i + 1 << ". Nodo: " << nombre_nodo
                  << " | K-Shell: " << ranking[i].second << "\n";
    }
}

// Experimentos de Perturbación
void ejecutar_experimento_perturbacion(Grafo& grafo) {
    std::cout << "\n=============================================\n";
    std::cout << "   E X P E R I M E N T O  A D I C I O N A L\n";
    std::cout << "=============================================\n";
    
    MedidasCentralidad analizador(grafo);

    std::cout << "\n[0] ESTADO ORIGINAL (Top 5 Betweenness)" << std::endl;
    std::vector<double> base_betw = analizador.calcular_betweenness_centrality();
    DCentralityTop(grafo, base_betw, "Original");

    int id_com = -1;
    for(int i = 0; i < grafo.obtener_num_vertices(); i++) {
        if(grafo.mapear_a_externo(i) == "COM") { id_com = i; break; }
    }

    // --- EXPERIMENTO 1: EL BYPASS ---
    std::cout << "\n[1] ANADIENDO UN 'BYPASS' GLOBAL (Conectando el resto del Top 5)" << std::endl;
    grafo.agregar_arista("BFA", "BRB", 1.0);
    grafo.agregar_arista("TZA", "BGR", 1.0);
    grafo.agregar_arista("BFA", "TZA", 1.0); 

    std::vector<double> exp1_betw = analizador.calcular_betweenness_centrality();
    DCentralityTop(grafo, exp1_betw, "Con Bypass (COM deberia desplomarse)");
    if(id_com != -1) {
        std::cout << " -> El puntaje de COM cayo a: " << std::fixed << std::setprecision(4) << exp1_betw[id_com] << "\n";
    }

    grafo.remover_arista("BFA", "BRB");
    grafo.remover_arista("TZA", "BGR");
    grafo.remover_arista("BFA", "TZA");

    // --- EXPERIMENTO 2: AISLAR AL REY ---
    std::cout << "\n[2] QUITANDO ARISTAS DE ABASTECIMIENTO A COM (FRA, USA, GBR -> COM)" << std::endl;
    grafo.remover_arista("FRA", "COM");
    grafo.remover_arista("USA", "COM");
    grafo.remover_arista("GBR", "COM");
    
    std::vector<double> exp2_betw = analizador.calcular_betweenness_centrality();
    DCentralityTop(grafo, exp2_betw, "Sin Aristas Vitales hacia COM");
    if(id_com != -1) {
        std::cout << " -> El puntaje de COM cayo a: " << std::fixed << std::setprecision(4) << exp2_betw[id_com] << "\n";
    }

    grafo.agregar_arista("FRA", "COM", 1.0);
    grafo.agregar_arista("USA", "COM", 1.0);
    grafo.agregar_arista("GBR", "COM", 1.0);
}
