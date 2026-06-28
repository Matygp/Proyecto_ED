#ifndef GRAFO_H
#define GRAFO_H

#include <iostream>
#include <vector>
#include <unordered_map>
// Se elimina <list> para priorizar localidad de caché (Data Prefetching)

struct Arista {
    int destino;
    double peso;

    Arista(int dest, double w = 1.0) : destino(dest), peso(w) {}
};

class Grafo {
private:
    // Mapea el ID original del dataset a un índice interno contiguo
    std::unordered_map<std::string, int> id_externo_a_interno;
    std::vector<std::string> id_interno_a_externo;
    
    // Refactorización HPC: Arrays contiguos en memoria para acelerar algoritmos de grafos
    std::vector<std::vector<Arista>> lista_adyacencia;
    std::vector<std::vector<Arista>> lista_adyacencia_inversa;

    bool es_dirigido;
    int num_vertices;
    int num_aristas;

    // Método privado para registrar un nodo internamente si no existe
    int obtener_o_crear_indice(const std::string& id_externo);

public:
    // Constructor
    Grafo(bool dirigido);

    // Métodos principales del ADT
    void agregar_vertice(const std::string& id_externo);
    void agregar_arista(const std::string& origen_externo, const std::string& destino_externo, double peso = 1.0);

    // Métodos de consulta esenciales
    int obtener_num_vertices() const;
    int obtener_num_aristas() const;
    bool comprobar_si_es_dirigido() const;
    
    std::string mapear_a_externo(int id_interno) const;

    // Selectores para recorrer los vecinos (ahora retornan referencias a vectores)
    const std::vector<Arista>& obtener_adyacentes_salientes(int id_interno) const;
    const std::vector<Arista>& obtener_adyacentes_entrantes(int id_interno) const;
};

#endif // GRAFO_H