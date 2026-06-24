#ifndef MEDIDAS_CENTRALIDAD_H
#define MEDIDAS_CENTRALIDAD_H

#include "grafo.h"
#include <vector>
#include <utility> //útil en HITS
#include <queue>   // Añadido para Priority Queue (Dijkstra)
#include <stack>   // Añadido para Brandes
#include <limits>  // Añadido para el valor Infinito

class MedidasCentralidad {
private:
    /*
    Referencia constante al grafo. 
    Se usa 'const' para asegurar que los algoritmos solo lean la red y no la modifiquen.
    */
    const Grafo& grafo;

    // MÉTODOS PRIVADOS AUXILIARES:
    // Aquí irán las funciones que se reutilizan en varias métricas

    //Calcula los caminos más cortos desde un nodo origen para Average
    std::vector<double> calcular_distancias_dijkstra(int origen) const;
    // std::vector<int> calcular_distancias_bfs(int origen) const;

public:
    //Constructor:
    MedidasCentralidad(const Grafo& g);

    /*
     MÉTRICAS BASADAS EN GRADO 
    Retornan un vector donde el índice es el ID interno del nodo y el valor es su centralidad
    */
    std::vector<double> calcular_degree_centrality() const;
    std::vector<double> calcular_in_degree_centrality() const;
    std::vector<double> calcular_out_degree_centrality() const;

    //      MÉTRICAS BASADAS EN CAMINOS MÁS CORTOS
    std::vector<double> calcular_betweenness_centrality() const;
    std::vector<double> calcular_closeness_centrality() const;
    
    double calcular_average_shortest_path() const; //métrica global

    /*
    MÉTRICAS ITERATIVAS Y DE PRESTIGIO
    PageRank recibe el factor de amortiguación (d), la tolerancia de convergencia y un máximo de iteraciones
    */
    std::vector<double> calcular_pagerank(double d = 0.85, double tolerancia = 1e-6, int max_iteraciones = 100) const;


    // vvv AQUI FALTARÍA HITS Y KCORE, QUE NO SE COMO FUNCIONAN vvv

};

#endif // MEDIDAS_CENTRALIDAD_H