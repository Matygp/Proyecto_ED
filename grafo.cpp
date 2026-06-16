#include "grafo.h"

//    Constructor
Grafo::Grafo(bool dirigido) {
    this-> es_dirigido = dirigido;
    this-> num_vertices = 0;
    this-> num_aristas = 0;
}

/*
Método privado para mapear IDs externos a índices internos continuos (0 a V-1)
traduce el valor que viene del dataaset a un índice del arreglo
Puede obtener los indices de los vertices ya creados, si no existen, los crea
*/
int Grafo::obtener_o_crear_indice(int id_externo) {

    //Si el ID externo ya existe en nuestro mapa, retorna su índice interno
    auto it = id_externo_a_interno.find(id_externo);
    if (it != id_externo_a_interno.end()) {
        return it->second;
    }

    //Si es un ID nuevo, le asignamos el índice correspondiente al valor actual de num_vertices
    int nuevo_indice = num_vertices;
    id_externo_a_interno[id_externo] = nuevo_indice; //guarda relación en el mapa x -> y
    id_interno_a_externo.push_back(id_externo); //guarda la relación inversa y -> x

    //Expandimos los vectores de las listas de adyacencia para hacer espacio al nuevo nodo
    lista_adyacencia.push_back(std::list<Arista>());
    lista_adyacencia_inversa.push_back(std::list<Arista>());

    num_vertices++;
    return nuevo_indice;
}

//Agrega un vértice, también permite agregar vértices aislados
void Grafo::agregar_vertice(int id_externo) {
    obtener_o_crear_indice(id_externo);
}

/*
Agrega una arista conectando dos nodos con un peso determinado
Si uno de los nodos no existe, lo crea
*/
void Grafo::agregar_arista(int origen_externo, int destino_externo, double peso) {
    //obtener los índices internos contiguos para ambos nodos
    int u = obtener_o_crear_indice(origen_externo);
    int v = obtener_o_crear_indice(destino_externo);

    //Insertar en la lista de adyacencia estándar: u -> v
    lista_adyacencia[u].push_back(Arista(v, peso));
    
    //Insertar en la lista inversa (v es apuntado por u)
    lista_adyacencia_inversa[v].push_back(Arista(u, peso));

    //Si el grafo NO es dirigido, la relación es simétrica
    if (!es_dirigido) {
        //Añadimos el camino de regreso: v -> u
        lista_adyacencia[v].push_back(Arista(u, peso));
        //y su correspondiente en la inversa
        lista_adyacencia_inversa[u].push_back(Arista(v, peso));
    }

    num_aristas++;
}


//                 === METODOS DE CONSULTA ===

int Grafo::obtener_num_vertices() const {
    return num_vertices;
}

int Grafo::obtener_num_aristas() const {
    return num_aristas;
}

bool Grafo::comprobar_si_es_dirigido() const {
    return es_dirigido;
}

//Permite recuperar el ID original del dataset usando el índice interno
int Grafo::mapear_a_externo(int id_interno) const {
    if (id_interno >= 0 && id_interno < num_vertices) {
        return id_interno_a_externo[id_interno];
    }
    return -1; //Retorna ID inválido si está fuera de rango
}

/*
Retorna nodos a los que el nodo ingresado apunta, para esto se hace uso de 
la lista de adyacencia.
(útil para BFS, Dijkstra, Out-Degree)
*/
const std::list<Arista>& Grafo::obtener_adyacentes_salientes(int id_interno) const {
    return lista_adyacencia[id_interno];
}

/* 
Retorna los nodos que apuntan a este nodo, para esto se utiliza la lista de adyacencia
inversa. 
(útil para In-Degree o PageRank de forma ultra eficiente)
*/
const std::list<Arista>& Grafo::obtener_adyacentes_entrantes(int id_interno) const {
    return lista_adyacencia_inversa[id_interno];
}