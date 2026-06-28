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
#include "main.h"

int main() {
    std::cout << "========= PERFILADOR DE RENDIMIENTO Y TESTEO =========\n\n";

    // Inicializar el archivo CSV
    std::ofstream archivo_csv("rendimiento.csv");
    if (!archivo_csv.is_open()) {
        std::cerr << "Error: No se pudo crear el archivo rendimiento.csv\n";
        return 1;
    }
    archivo_csv << "dataset,metrica,iteracion,tiempo_ms,memoria_kb\n";

    // =========================================================================
    //                            DATASET YEAST
    // =========================================================================
    std::cout << "===================================" << std::endl;
    std::cout << "-> Cargando dataset Yeast (Proteinas): " << std::endl;
    Grafo yeast(false); 

    if (cargarDataset(yeast, "datasets/yeast.edgelist", false)) { 
        MedidasCentralidad analizadorYeast(yeast);
        std::cout << "   [OK] Dataset Yeast cargado con exito.\n";
        std::cout << "   [*] Numero de vertices (V): " << yeast.obtener_num_vertices() << std::endl;
        std::cout << "   [*] Numero de aristas (E) : " << yeast.obtener_num_aristas() << std::endl;
        
        // ── DEGREE CENTRALITY (Yeast) ──
    std::vector<double> degree_yeast;
    for (int i = 1; i <= 10; ++i) {
        auto inicio = std::chrono::high_resolution_clock::now();
        degree_yeast = analizadorYeast.calcular_degree_centrality(); // Asumiendo este nombre de método
        auto fin = std::chrono::high_resolution_clock::now();
        archivo_csv << "yeast,degree," << i << "," << std::chrono::duration<double, std::milli>(fin-inicio).count() << "," << obtener_memoria_kb() << "\n";
    }
    DCentralityTop(yeast, degree_yeast, "Degree Centrality");

        
        // ── CLOSENESS CENTRALITY (10 iteraciones) ──
        std::cout << "\n[!] Ejecutando Closeness Centrality (10 repeticiones)...\n";
        std::vector<double> closeness_yeast;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            closeness_yeast = analizadorYeast.calcular_closeness_centrality_bfs();
            auto fin = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> tiempo = fin - inicio;
            archivo_csv << "yeast,closeness," << i << "," << std::fixed << std::setprecision(4) << tiempo.count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(yeast, closeness_yeast, "Closeness Centrality (Proteinas Centrales)");

        
        // ── AVERAGE SHORTEST PATH ──
        double avg_path = 0.0;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            avg_path = analizadorYeast.calcular_average_shortest_path();
            auto fin = std::chrono::high_resolution_clock::now();
            archivo_csv << "yeast,avg_path," << i << "," << std::chrono::duration<double, std::milli>(fin-inicio).count() << "," << obtener_memoria_kb() << "\n";
        }
        std::cout << "\n- Average Shortest Path: " << avg_path << "\n";

        // ── HITS (10 iteraciones) ──
        std::cout << "\n[!] Ejecutando HITS (10 repeticiones)...\n";
        std::pair<std::vector<double>, std::vector<double>> hits_trade;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            hits_trade = analizadorYeast.calcular_hits();
            auto fin = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> tiempo = fin - inicio;
            archivo_csv << "yeast,hits," << i << "," << std::fixed << std::setprecision(4) << tiempo.count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(yeast, hits_trade.first,  "HITS - Hub Scores");
        DCentralityTop(yeast, hits_trade.second, "HITS - Authority Scores");

        // ── BETWEENNESS CENTRALITY  ──
        std::vector<double> betw_data;
        for (int i = 1; i <= 10; ++i) {
             auto inicio = std::chrono::high_resolution_clock::now();
            betw_data = analizadorYeast.calcular_betweenness_centrality();
            auto fin = std::chrono::high_resolution_clock::now();
            archivo_csv << "yeast,betweenness," << i << "," << std::chrono::duration<double, std::milli>(fin-inicio).count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(yeast, betw_data, "Betweenness Centrality");

        // ── PAGERANK  ──
        std::vector<double> pr_data;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            pr_data = analizadorYeast.calcular_pagerank();
            auto fin = std::chrono::high_resolution_clock::now();
            archivo_csv << "yeast,pagerank," << i << "," << std::chrono::duration<double, std::milli>(fin-inicio).count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(yeast, pr_data, "PageRank");
        
        // ── CORENESS (10 iteraciones) ──
        std::cout << "\n[!] Ejecutando Coreness (10 repeticiones)...\n";
        std::vector<int> coreness_yeast;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            coreness_yeast = analizadorYeast.calcular_coreness();
            auto fin = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> tiempo = fin - inicio;
            archivo_csv << "yeast,coreness," << i << "," << std::fixed << std::setprecision(4) << tiempo.count() << "," << obtener_memoria_kb() << "\n";
        }
        DCoreTop(yeast, coreness_yeast, "Coreness (Proteinas en el nucleo de la red)");
    }

    // =========================================================================
    //                         DATASET TRADE NETWORK
    // =========================================================================
    std::cout << "\n===================================" << std::endl;
    std::cout << "-> Cargando dataset Trade Network (Economia): " << std::endl;
    Grafo trade(true); 

    if (cargarDataset(trade, "datasets/2005.net", true)) { 
        MedidasCentralidad analizadorTrade(trade);
        std::cout << "   [OK] Dataset Trade Network cargado con exito.\n";
        std::cout << "   [*] Numero de vertices (V): " << trade.obtener_num_vertices() << std::endl;
        std::cout << "   [*] Numero de aristas (E) : " << trade.obtener_num_aristas() << std::endl;


        // ── DEGREE CENTRALITY (Trade) ──
        std::vector<double> degree_trade;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            degree_trade = analizadorTrade.calcular_degree_centrality(); 
            auto fin = std::chrono::high_resolution_clock::now();
            archivo_csv << "trade,degree," << i << "," << std::chrono::duration<double, std::milli>(fin-inicio).count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(trade, degree_trade, "Degree Centrality");

        // ── CLOSENESS CENTRALITY (10 iteraciones) ──
        std::cout << "\n[!] Ejecutando Closeness Centrality (10 repeticiones)...\n";
        std::vector<double> closeness_trade;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            closeness_trade = analizadorTrade.calcular_closeness_centrality();
            auto fin = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> tiempo = fin - inicio;
            archivo_csv << "trade,closeness," << i << "," << std::fixed << std::setprecision(4) << tiempo.count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(trade, closeness_trade, "Closeness Centrality (Nodos mas interconectados)");

        // ── AVERAGE SHORTEST PATH ──
        double avg_path = 0.0;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            avg_path = analizadorTrade.calcular_average_shortest_path(); // Cambiar a analizadorYeast en la otra sección
            auto fin = std::chrono::high_resolution_clock::now();
            archivo_csv << "trade,avg_path," << i << "," << std::chrono::duration<double, std::milli>(fin-inicio).count() << "," << obtener_memoria_kb() << "\n";
        }
        std::cout << "\n- Average Shortest Path: " << avg_path << "\n";

        // ── HITS (10 iteraciones) ──
        std::cout << "\n[!] Ejecutando HITS (10 repeticiones)...\n";
        std::pair<std::vector<double>, std::vector<double>> hits_trade;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            hits_trade = analizadorTrade.calcular_hits();
            auto fin = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> tiempo = fin - inicio;
            archivo_csv << "trade,hits," << i << "," << std::fixed << std::setprecision(4) << tiempo.count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(trade, hits_trade.first,  "HITS - Hub Scores (Exportadores mas influyentes)");
        DCentralityTop(trade, hits_trade.second, "HITS - Authority Scores (Principales destinos comerciales)");


        // ── BETWEENNESS CENTRALITY ──
        std::vector<double> betw_data;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            betw_data = analizadorTrade.calcular_betweenness_centrality();
            auto fin = std::chrono::high_resolution_clock::now();
            archivo_csv << "trade,betweenness," << i << "," << std::chrono::duration<double, std::milli>(fin-inicio).count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(trade, betw_data, "Betweenness Centrality");

        // ── PAGERANK ──
        std::vector<double> pr_data;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            pr_data = analizadorTrade.calcular_pagerank();
            auto fin = std::chrono::high_resolution_clock::now();
            archivo_csv << "trade,pagerank," << i << "," << std::chrono::duration<double, std::milli>(fin-inicio).count() << "," << obtener_memoria_kb() << "\n";
        }
        DCentralityTop(trade, pr_data, "PageRank");
        // ── CORENESS (10 iteraciones) ──
        std::cout << "\n[!] Ejecutando Coreness (10 repeticiones)...\n";
        std::vector<int> coreness_trade;
        for (int i = 1; i <= 10; ++i) {
            auto inicio = std::chrono::high_resolution_clock::now();
            coreness_trade = analizadorTrade.calcular_coreness();
            auto fin = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> tiempo = fin - inicio;
            archivo_csv << "trade,coreness," << i << "," << std::fixed << std::setprecision(4) << tiempo.count() << "," << obtener_memoria_kb() << "\n";
        }
        DCoreTop(trade, coreness_trade, "Coreness (Nucleo de la red comercial global)");

        // ── EXPERIMENTOS DE PERTURBACIÓN ──
        ejecutar_experimento_perturbacion(trade);
    }

    std::cout << "===================================" << std::endl;
    std::cout << "[+] Datos exportados exitosamente a 'rendimiento.csv'\n";
    archivo_csv.close();

    return 0;
}