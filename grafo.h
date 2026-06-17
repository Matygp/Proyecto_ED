#ifndef GRAFO_H
#define GRAFO_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>

struct Arista {
    int destino;
    double peso;

    Arista(int dest, double w = 1.0) : destino(dest), peso(w) {}
};

class Grafo {
private:
    // Mapea el ID original del dataset (que puede ser un string o un int cualquiera) 
    // a un índice interno contiguo (0, 1, 2, ... V-1). Esto facilita el uso de vectores.
    std::unordered_map<std::string, int> id_externo_a_interno;
    std::vector<std::string> id_interno_a_externo;
    
    // La lista de adyacencia: un vector donde cada posición es la lista de aristas salientes de ese nodo
    std::vector<std::list<Arista>> lista_adyacencia;
    
    /* Lista de adyacencia inversa para calcular de forma eficiente el In-Degree en grafos dirigidos
    Puede servir para calcular Degree Centrality en O(1) OJO con esta */
    std::vector<std::list<Arista>> lista_adyacencia_inversa;

    bool es_dirigido;
    int num_vertices;
    int num_aristas;

    // Método privado para registrar un nodo internamente si no existe
    int obtener_o_crear_indice(const std::string& id_externo);

public:
    // Constructor: define si el grafo de entrada será dirigido o no
    Grafo(bool dirigido);

    // Métodos principales del ADT
    void agregar_vertice(const std::string& id_externo);
    void agregar_arista(const std::string& origen_externo, const std::string& destino_externo, double peso = 1.0);

    // Métodos de consulta esenciales para las métricas
    int obtener_num_vertices() const;
    int obtener_num_aristas() const;
    bool comprobar_si_es_dirigido() const;
    
    std::string mapear_a_externo(int id_interno) const;

    // Selectores para recorrer los vecinos (útiles para BFS, Dijkstra y PageRank)
    const std::list<Arista>& obtener_adyacentes_salientes(int id_interno) const;
    const std::list<Arista>& obtener_adyacentes_entrantes(int id_interno) const;
};

#endif // GRAFO_H