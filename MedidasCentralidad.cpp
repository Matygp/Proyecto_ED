#include "MedidasCentralidad.h"

// CONSTRUCTOR: Se inicializa la referencia constante al grafo usando 
//una lista de inicialización
MedidasCentralidad::MedidasCentralidad(const Grafo& g) : grafo(g) {}


/* 
        D E G R E E    C E N T R A L I T Y 
Esta métrica se abordará de dos formas, la primera para grafos no dirigidos (útil para yeast)
y la segunda para grafos dirigidos con in-degree y out-degree centrality (trade network)
*/

std::vector<double> MedidasCentralidad::calcular_degree_centrality() const { //para grafos no dirigidos
    int V = grafo.obtener_num_vertices();
    std::vector<double> centralidad(V, 0.0); //Vector de tamaño V, inicializado en cero

    // caso: si hay 1 o 0 nodos, no hay conexiones posibles 
    if (V <= 1) return centralidad; 

    //Factor de normalización una sola vez por eficiencia (N - 1)
    double normalizacion = static_cast<double>(V - 1);

    for (int i = 0; i < V; ++i) {
        //Para grafos no dirigidos, la arista existe en ambos sentidos
        int grado = grafo.obtener_adyacentes_salientes(i).size();
        
        //se aplica la fórmula normalizada para el nodo i
        centralidad[i] = grado / normalizacion;
    }

    return centralidad;
}


// IN-DEGREE CENTRALITY (Grafo Dirigido)
std::vector<double> MedidasCentralidad::calcular_in_degree_centrality() const {
    int V = grafo.obtener_num_vertices();
    std::vector<double> centralidad(V, 0.0);

    if (V <= 1) return centralidad;

    double normalizacion = static_cast<double>(V - 1);

    for (int i = 0; i < V; ++i) {
        //Se utiliza la lista_adyacencia_inversa implementada en grafo.cpp
        int in_degree = grafo.obtener_adyacentes_entrantes(i).size();
        centralidad[i] = in_degree / normalizacion;
    }

    return centralidad;
}


// OUT-DEGREE CENTRALITY (Grafo Dirigido)
std::vector<double> MedidasCentralidad::calcular_out_degree_centrality() const {
    int V = grafo.obtener_num_vertices();
    std::vector<double> centralidad(V, 0.0);

    if (V <= 1) return centralidad;

    double normalizacion = static_cast<double>(V - 1);

    for (int i = 0; i < V; ++i) {
        int out_degree = grafo.obtener_adyacentes_salientes(i).size();
        centralidad[i] = out_degree / normalizacion;
    }

    return centralidad;
// FIN de degree centrality

}