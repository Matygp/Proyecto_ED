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

/*
Imprime el TOP 5 de nodos ordenados por su K-Shell (coreness).
Función independiente de DCentralityTop para respetar la firma
std::vector<int> que retorna calcular_coreness(), evitando conversiones
implícitas o mezcla de tipos int/double en la interfaz de display.
*/
void DCoreTop(const Grafo& grafo, const std::vector<int>& coreness, const std::string& titulo) {
    std::cout << "\n-  TOP: " << titulo << "   -\n";
    int V = grafo.obtener_num_vertices();
    if (V == 0) return;

    // Vector de pares {id_interno, k_shell} para poder recuperar el nombre externo tras ordenar
    std::vector<std::pair<int, int>> ranking;
    ranking.reserve(V);
    for (int i = 0; i < V; ++i) {
        ranking.push_back({i, coreness[i]});
    }

    // Orden descendente por K-Shell
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

int main() {
    std::cout << "========= PRUEBAS Y TESTEO =========\n\n";

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
/*
        std::cout << "-- D E G R E E  C E N T R A L I T Y -- " << std::endl;

        // CALCULA DEGREE CENTRALITY
        std::vector<double> degree_yeast = analizadorYeast.calcular_degree_centrality();
        DCentralityTop(yeast, degree_yeast, "Degree Centrality (Hubs Proteicos)"); //imprime
        
        std::cout << "\n-- A V E R A G E  S H O R T E S T  P A T H -- " << std::endl;
        // Calcula Average shortest path
        auto inicioAvgYeast = std::chrono::high_resolution_clock::now();
        double avg_path_yeast = analizadorYeast.calcular_average_shortest_path();
        auto finAvgYeast = std::chrono::high_resolution_clock::now();
        std::cout << "   [*] Largo promedio de la red: " << avg_path_yeast << std::endl;
        std::cout << "   [*] Calculado en: " << std::chrono::duration<double, std::milli>(finAvgYeast - inicioAvgYeast).count() << " ms\n";
        // Al ser muchos nodos puede demorar un poco en realizar el calculo tiempo estimado de 15 min, dada las ejecucuiones hechas.

        std::cout << "\n-- B E T W E E N N E S S  C E N T R A L I T Y -- " << std::endl;
        auto inicioBetwYeast = std::chrono::high_resolution_clock::now();
        std::vector<double> betw_yeast = analizadorYeast.calcular_betweenness_centrality();
        auto finBetwYeast = std::chrono::high_resolution_clock::now();
        DCentralityTop(yeast, betw_yeast, "Betweenness (Cuellos de botella proteicos)");
        std::cout << "   [*] Calculado en: " << std::chrono::duration<double, std::milli>(finBetwYeast - inicioBetwYeast).count() << " ms\n";
        // Al ser muchos nodos puede demorar un poco en realizar el calculo tiempo estimado de 15 min, dada las ejecucuiones hechas.


        // ─────────────── probando CLOSENESS en yeast ─────────────────────
        std::cout << "\n[!] Calculando Closeness Centrality (Version BFS):\n";
        
        auto inicioCloseYeast = std::chrono::high_resolution_clock::now();
        
        // se llama explícitamente a la versión _bfs
        std::vector<double> closeness_yeast = analizadorYeast.calcular_closeness_centrality_bfs();
        
        auto finCloseYeast = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoCloseYeast = finCloseYeast - inicioCloseYeast;

        DCentralityTop(yeast, closeness_yeast, "Closeness Centrality (Proteinas Centrales)");
        std::cout << "    (Tiempo de calculo: " << tiempoCloseYeast.count() << " ms)\n";

        // ── K-CORE DECOMPOSITION (Yeast) ─────────────────────────────
        std::cout << "\n[!] Calculando Coreness (K-Core decomposition):\n";

        auto inicioCoreYeast = std::chrono::high_resolution_clock::now();

        std::vector<int> coreness_yeast = analizadorYeast.calcular_coreness();

        auto finCoreYeast = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoCoreYeast = finCoreYeast - inicioCoreYeast;

        DCoreTop(yeast, coreness_yeast, "Coreness (Proteinas en el nucleo de la red)");
        std::cout << "    (Tiempo de calculo: " << tiempoCoreYeast.count() << " ms)\n";
    */

        //─────────────────── PAGERANK TESTING ───────────────────
        std::cout << "\n[!] Test PageRank\n";
        auto inicioPR = std::chrono::high_resolution_clock::now();

        //Usa los valores por defecto (d=0.85, tol=1e-6)
        std::vector<double> pagerank = analizadorYeast.calcular_pagerank(); 

        auto finPR = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoPR = finPR - inicioPR;

        DCentralityTop(yeast, pagerank, "PageRank");
        std::cout << "    (Tiempo de calculo: " << tiempoPR.count() << " ms)\n";
    }

    //Cargar Dataset Trade Network 
    std::cout << "===================================" << std::endl;
    std::cout << "\n-> Cargando dataset Trade Network (Economia): " << std::endl;
    Grafo trade(true); // -> Dirigido

    auto inicioTrade = std::chrono::high_resolution_clock::now();
    if (cargarDataset(trade, "datasets/2005.net", true)) { 
        MedidasCentralidad analizadorTrade(trade);
        auto finTrade = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoTrade = finTrade - inicioTrade;

        std::cout << "   [OK] Dataset Trade Network cargado con exito." << std::endl;
        std::cout << "   [*] Numero de vertices (V): " << trade.obtener_num_vertices() << std::endl;
        std::cout << "   [*] Numero de aristas (E) : " << trade.obtener_num_aristas() << std::endl;
        std::cout << "   [*] Tiempo de construccion: " << tiempoTrade.count() << " ms\n" << std::endl;

        //std::cout << "-- D E G R E E  C E N T R A L I T Y -- " << std::endl;
/*
        // TEST    DEGREE   CENTRALITY
        // Calcular In-Degree (Importadores) y Out-Degree (Exportadores)
        std::vector<double> in_degree_trade = analizadorTrade.calcular_in_degree_centrality();
        std::vector<double> out_degree_trade = analizadorTrade.calcular_out_degree_centrality();

        DCentralityTop(trade, in_degree_trade, "In-Degree (Mayores Importadores/Destinos)");
        DCentralityTop(trade, out_degree_trade, "Out-Degree (Mayores Exportadores/Origenes)");

        std::cout << "\n-- A V E R A G E  S H O R T E S T  P A T H -- " << std::endl;
        // Calcula Average shortest path 
        auto inicioAvgTrade = std::chrono::high_resolution_clock::now();
        double avg_path_trade = analizadorTrade.calcular_average_shortest_path();
        auto finAvgTrade = std::chrono::high_resolution_clock::now();
        std::cout << "   [*] Largo promedio de la red: " << avg_path_trade << std::endl;
        std::cout << "   [*] Calculado en: " << std::chrono::duration<double, std::milli>(finAvgTrade - inicioAvgTrade).count() << " ms\n";
        
        std::cout << "\n-- B E T W E E N N E S S  C E N T R A L I T Y -- " << std::endl;
        auto inicioBetwTrade = std::chrono::high_resolution_clock::now();
        std::vector<double> betw_trade = analizadorTrade.calcular_betweenness_centrality();
        auto finBetwTrade = std::chrono::high_resolution_clock::now();
        DCentralityTop(trade, betw_trade, "Betweenness (Puntos criticos de comercio)");
        std::cout << "   [*] Calculado en: " << std::chrono::duration<double, std::milli>(finBetwTrade - inicioBetwTrade).count() << " ms\n";

        //Closeness Centrality
        std::cout << "\n[!] Calcula Closeness Centrality:\n";
        
        // Se inicia el cronómetro
        auto inicioClose = std::chrono::high_resolution_clock::now();
        
        // Se llama a la función 
        std::vector<double> closeness = analizadorTrade.calcular_closeness_centrality();
        
        // Detiene cronómetro
        auto finClose = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoClose = finClose - inicioClose;

        // Imprime el Top 5
        DCentralityTop(trade, closeness, "Closeness Centrality (Nodos mas interconectados)");
        std::cout << "    (Tiempo de calculo: " << tiempoClose.count() << " ms)\n";

        // ── HITS — Hubs y Autoridades (Trade Network) ─────────────────
        std::cout << "\n[!] Calculando HITS (Hubs y Autoridades):\n";

        auto inicioHits = std::chrono::high_resolution_clock::now();

        // calcular_hits() retorna std::pair<vector<double>, vector<double>>
        // .first  = Hub Scores      (paises que exportan hacia buenos destinos)
        // .second = Authority Scores (paises que reciben de buenos exportadores)
        auto [hubs_trade, auths_trade] = analizadorTrade.calcular_hits();

        auto finHits = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoHits = finHits - inicioHits;

        DCentralityTop(trade, hubs_trade,  "HITS - Hub Scores (Exportadores mas influyentes)");
        DCentralityTop(trade, auths_trade, "HITS - Authority Scores (Principales destinos comerciales)");
        std::cout << "    (Tiempo de calculo: " << tiempoHits.count() << " ms)\n";

        // ── K-CORE DECOMPOSITION (Trade Network) ──────────────────────
        std::cout << "\n[!] Calculando Coreness (K-Core decomposition):\n";

        auto inicioCoreTrade = std::chrono::high_resolution_clock::now();

        std::vector<int> coreness_trade = analizadorTrade.calcular_coreness();

        auto finCoreTrade = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoCoreTrade = finCoreTrade - inicioCoreTrade;

        DCoreTop(trade, coreness_trade, "Coreness (Nucleo de la red comercial global)");
        std::cout << "    (Tiempo de calculo: " << tiempoCoreTrade.count() << " ms)\n";
        */
        //─────────────────── PAGERANK TESTING ───────────────────
        std::cout << "\n[!] Test PageRank\n";
        auto inicioPR = std::chrono::high_resolution_clock::now();

        //Usa los valores por defecto (d=0.85, tol=1e-6)
        std::vector<double> pagerank = analizadorTrade.calcular_pagerank(); 

        auto finPR = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> tiempoPR = finPR - inicioPR;

        DCentralityTop(trade, pagerank, "PageRank");
        std::cout << "    (Tiempo de calculo: " << tiempoPR.count() << " ms)\n";
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