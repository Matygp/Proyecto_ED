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

std::vector<double> MedidasCentralidad::calcular_distancias_dijkstra(int origen) const {
    int V = grafo.obtener_num_vertices();
    // Inicializamos todas las distancias en "infinito"
    std::vector<double> dist(V, std::numeric_limits<double>::infinity());
    
    // Min-Heap para extraer siempre el nodo con la distancia más corta: {distancia, nodo}
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<std::pair<double, int>>> pq;

    dist[origen] = 0.0;
    pq.push({0.0, origen});

    while (!pq.empty()) {
        double d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        // Si encontramos una distancia mayor a la ya registrada, es un nodo obsoleto
        if (d > dist[u]) continue;

        // Recorremos los vecinos salientes
        for (const Arista& arista : grafo.obtener_adyacentes_salientes(u)) {
            int v = arista.destino;
            double peso = arista.peso;

            // Relajación de la arista
            if (dist[u] + peso < dist[v]) {
                dist[v] = dist[u] + peso;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

double MedidasCentralidad::calcular_average_shortest_path() const {
    int V = grafo.obtener_num_vertices();
    if (V <= 1) return 0.0;

    double suma_distancias = 0.0;
    long long caminos_validos = 0; // Usamos long long por si V es muy grande (ej. Yeast)

    for (int i = 0; i < V; ++i) {
        std::vector<double> dist = calcular_distancias_dijkstra(i);
        
        for (int j = 0; j < V; ++j) {
            // Solo sumamos si el nodo es distinto al origen y si existe un camino (no es infinito)
            if (i != j && dist[j] != std::numeric_limits<double>::infinity()) {
                suma_distancias += dist[j];
                caminos_validos++;
            }
        }
    }

    if (caminos_validos == 0) return 0.0; // Evita división por cero si el grafo no tiene aristas
    return suma_distancias / caminos_validos;
}