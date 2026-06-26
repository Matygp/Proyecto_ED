#include "MedidasCentralidad.h"
#include <queue>
#include <limits>

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

//              A V E R A G E   S H O R T E S T   P A T H

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


//             B E T W E E N N E S S     C E N T R A L I T Y

std::vector<double> MedidasCentralidad::calcular_betweenness_centrality() const {
    int V = grafo.obtener_num_vertices();
    std::vector<double> betweenness(V, 0.0);

    // Ejecutamos una variación de Dijkstra desde cada nodo 's'
    for (int s = 0; s < V; ++s) {
        std::stack<int> S;                     // Pila para procesar los nodos en orden inverso de distancia
        std::vector<std::vector<int>> P(V);    // Lista de predecesores de cada nodo
        std::vector<double> sigma(V, 0.0);     // Número de caminos más cortos desde 's'
        sigma[s] = 1.0;
        std::vector<double> d(V, std::numeric_limits<double>::infinity()); // Distancias
        d[s] = 0.0;

        // Min-Heap: {distancia, nodo}
        std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<std::pair<double, int>>> pq;
        pq.push({0.0, s});

        while (!pq.empty()) {
            double dist_u = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            if (dist_u > d[u]) continue;

            S.push(u); // Se apila el nodo para la fase de acumulación

            for (const Arista& arista : grafo.obtener_adyacentes_salientes(u)) {
                int v = arista.destino;
                double peso = arista.peso;

                // 1. Se encontró un camino estrictamente más corto
                if (d[u] + peso < d[v]) {
                    d[v] = d[u] + peso;
                    pq.push({d[v], v});
                    sigma[v] = sigma[u];   // Reiniciamos el conteo de caminos
                    P[v].clear();
                    P[v].push_back(u);     // 'u' es el nuevo predecesor
                }
                // 2. Se encontró otro camino alternativo con la misma longitud mínima
                else if (d[u] + peso == d[v]) {
                    sigma[v] += sigma[u];  // Sumamos los caminos
                    P[v].push_back(u);     // Añadimos 'u' como predecesor alternativo
                }
            }
        }

        // Fase de acumulación (Back-propagation)
        std::vector<double> delta(V, 0.0);
        while (!S.empty()) {
            int w = S.top();
            S.pop();
            
            for (int v : P[w]) { // v es predecesor de w
                // Validamos división por cero en caso de imprecisiones de coma flotante
                if (sigma[w] != 0) {
                    delta[v] += (sigma[v] / sigma[w]) * (1.0 + delta[w]);
                }
            }
            if (w != s) {
                betweenness[w] += delta[w];
            }
        }
    }

    // Si el grafo es no dirigido, cada camino se contó dos veces (ida y vuelta), así que dividimos a la mitad
    if (!grafo.comprobar_si_es_dirigido()) {
        for (int i = 0; i < V; ++i) {
            betweenness[i] /= 2.0;
        }
    }

    return betweenness;
}


//            C L O S E N E S S    C E N T R A L I T Y

std::vector<double> MedidasCentralidad::calcular_closeness_centrality() const {
    int V = grafo.obtener_num_vertices();
    std::vector<double> closeness(V, 0.0);

    if (V <= 1) return closeness;

    // Se calcula los caminos más cortos desde cada nodo s
    for (int s = 0; s < V; ++s) {
        std::vector<double> d(V, std::numeric_limits<double>::infinity());
        d[s] = 0.0;
        
        //Cola de prioridad min-Heap para Dijkstra, almacena pares {distancia, nodo}
        std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<std::pair<double, int>>> pq;
        pq.push({0.0, s});

        int nodos_alcanzables = 0;
        double suma_distancias = 0.0;

        while (!pq.empty()) { //procesa iterativamente el camino mas eficiente
            double dist_u = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            //Si hay una distancia mayor a la registrada, se ignora
            if (dist_u > d[u]) continue;

            nodos_alcanzables++;
            suma_distancias += dist_u;

            // Se exploran los vecinos (relajación de Dijkstra)
            for (const auto& arista : grafo.obtener_adyacentes_salientes(u)) {
                int v = arista.destino;
                double peso = arista.peso;
                
                if (d[u] + peso < d[v]) {
                    d[v] = d[u] + peso;
                    pq.push({d[v], v});
                }
            }
        }

        /* 
        Se aplica la fórmula normalizada de Wasserman y Faust para grafos con componentes desconectados.
        Esto ya que al usar la fórmula clásica en un grafo disconexo, existen nodos a los que u no puede llegar,
        la distancia a esos nodos será infinito, por lo que al sumarlos con los demás toda la fracción se vuelve
        cero perdiendo toda la información de los nodos a los que sí podía llegar.
        */
        if (nodos_alcanzables > 1) {
            double numerador = static_cast<double>(nodos_alcanzables - 1); // (n-1)
            double factor_escala = numerador / static_cast<double>(V - 1); // Penaliza si no alcanza a todos
            closeness[s] = (factor_escala * numerador) / suma_distancias;
        } else {
            closeness[s] = 0.0; // Nodo aislado
        }
    }

    return closeness;
}


// CLOSENESS CENTRALITY B (Yeast - Sin Peso usando BFS)
std::vector<double> MedidasCentralidad::calcular_closeness_centrality_bfs() const {
    int V = grafo.obtener_num_vertices();
    std::vector<double> closeness(V, 0.0);

    if (V <= 1) return closeness;

    // Calculamos los caminos más cortos desde cada nodo s usando BFS 
    for (int s = 0; s < V; ++s) {
        std::vector<int> d(V, -1); //distancias inicializadas en -1 (no visitado)
        d[s] = 0;
        
        std::queue<int> q; //Cola simple para BFS (no utiliza priority_queue)
        q.push(s);

        int nodos_alcanzables = 0;
        double suma_distancias = 0.0;

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            nodos_alcanzables++;
            suma_distancias += d[u];

            // Se exploran los vecinos
            for (const auto& arista : grafo.obtener_adyacentes_salientes(u)) {
                int v = arista.destino;
                
                // Si el nodo no ha sido visitado
                if (d[v] == -1) {
                    d[v] = d[u] + 1; //En BFS cada salto vale 1 exacto
                    q.push(v);
                }
            }
        }

        //Se aplica la misma fórmula normalizada de Wasserman y Faust
        if (nodos_alcanzables > 1) {
            double numerador = static_cast<double>(nodos_alcanzables - 1);
            double factor_escala = numerador / static_cast<double>(V - 1); 
            closeness[s] = (factor_escala * numerador) / suma_distancias;
        } else {
            closeness[s] = 0.0; 
        }
    }

    return closeness;
}
// -fin de closeness centrality-