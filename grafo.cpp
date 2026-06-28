#include "grafo.h"
#include <algorithm>
// Constructor
Grafo::Grafo(bool dirigido) {
    this->es_dirigido = dirigido;
    this->num_vertices = 0;
    this->num_aristas = 0;
}

/*
Método privado para mapear IDs externos a índices internos continuos (0 a V-1)
*/
int Grafo::obtener_o_crear_indice(const std::string& id_externo) {

    auto it = id_externo_a_interno.find(id_externo);
    if (it != id_externo_a_interno.end()) {
        return it->second;
    }

    int nuevo_indice = num_vertices;
    id_externo_a_interno[id_externo] = nuevo_indice; 
    id_interno_a_externo.push_back(id_externo); 

    // Expandimos los vectores maestros empujando nuevos vectores (vacíos) para el nodo
    lista_adyacencia.push_back(std::vector<Arista>());
    lista_adyacencia_inversa.push_back(std::vector<Arista>());

    num_vertices++;
    return nuevo_indice;
}

void Grafo::agregar_vertice(const std::string& id_externo) {
    obtener_o_crear_indice(id_externo);
}

/*
Agrega una arista conectando dos nodos con un peso determinado
*/
void Grafo::agregar_arista(const std::string& origen_externo, const std::string& destino_externo, double peso) {
    int u = obtener_o_crear_indice(origen_externo);
    int v = obtener_o_crear_indice(destino_externo);

    // Insertar en la lista de adyacencia estándar (push_back es compatible con std::vector)
    lista_adyacencia[u].push_back(Arista(v, peso));
    
    // Insertar en la lista inversa
    lista_adyacencia_inversa[v].push_back(Arista(u, peso));

    if (!es_dirigido) {
        lista_adyacencia[v].push_back(Arista(u, peso));
        lista_adyacencia_inversa[u].push_back(Arista(v, peso));
    }

    num_aristas++;
}

// === METODOS DE CONSULTA ===

int Grafo::obtener_num_vertices() const {
    return num_vertices;
}

int Grafo::obtener_num_aristas() const {
    return num_aristas;
}

bool Grafo::comprobar_si_es_dirigido() const {
    return es_dirigido;
}

std::string Grafo::mapear_a_externo(int id_interno) const {
    if (id_interno >= 0 && id_interno < num_vertices) {
        return id_interno_a_externo[id_interno];
    }
    return ""; 
}

/*
Retorna la referencia contigua a las aristas salientes
*/
const std::vector<Arista>& Grafo::obtener_adyacentes_salientes(int id_interno) const {
    return lista_adyacencia[id_interno];
}

/* Retorna la referencia contigua a las aristas entrantes
*/
const std::vector<Arista>& Grafo::obtener_adyacentes_entrantes(int id_interno) const {
    return lista_adyacencia_inversa[id_interno];

/*
Elimina una arista del grafo. Utiliza el idioma Erase-Remove para 
mantener la eficiencia de memoria contigua en std::vector.
*/

}


/*
Elimina una arista del grafo. Utiliza el idioma Erase-Remove para 
mantener la eficiencia de memoria contigua en std::vector.
*/
void Grafo::remover_arista(const std::string& origen_externo, const std::string& destino_externo) {
    // 1. Buscar si los nodos existen en el mapa
    auto it_u = id_externo_a_interno.find(origen_externo);
    auto it_v = id_externo_a_interno.find(destino_externo);

    // Si alguno de los dos nodos no existe, no hay nada que borrar
    if (it_u == id_externo_a_interno.end() || it_v == id_externo_a_interno.end()) {
        return; 
    }

    int u = it_u->second;
    int v = it_v->second;

    // Lambda auxiliar para borrar por ID de destino en un vector específico
    auto borrar_por_destino = [](std::vector<Arista>& vec, int dest) {
        vec.erase(std::remove_if(vec.begin(), vec.end(), 
            [dest](const Arista& a) { return a.destino == dest; }), vec.end());
    };

    // 2. Borrar de la lista directa e inversa
    borrar_por_destino(lista_adyacencia[u], v);
    borrar_por_destino(lista_adyacencia_inversa[v], u);

    // 3. Si es no dirigido, borrar también la conexión de retorno
    if (!es_dirigido) {
        borrar_por_destino(lista_adyacencia[v], u);
        borrar_por_destino(lista_adyacencia_inversa[u], v);
    }
    
    num_aristas--; 
}