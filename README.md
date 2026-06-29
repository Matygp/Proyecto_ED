# Análisis de Redes y Medidas de Centralidad

Este proyecto implementa y evalúa el rendimiento de diversos algoritmos de centralidad sobre estructuras de grafos. Desarrollado en C++ para el procesamiento algorítmico de alto rendimiento.

## Características Principales

*   **Estructura Eficiente:** Representación del grafo mediante listas de adyacencia directas e inversas, con un mapeo de IDs para optimizar la memoria caché.
*   **Métricas Implementadas:** 
    *   Degree Centrality (In/Out)
    *   Closeness y Average Shortest Path (mediante BFS y Dijkstra)
    *   Betweenness Centrality (Algoritmo de Brandes)
    *   PageRank y HITS (Iteración de potencias)
    *   Coreness (Algoritmo de Batagelj & Zaversnik)
*   **Perfilamiento Riguroso:** Ejecución iterativa de pruebas para medir con precisión tiempos de ejecución (ms) y consumo de memoria (KB).
*   **Visualización de Datos:** Scripts automatizados (`analisis_rendimiento.py`) para interpretar el archivo `rendimiento.csv` resultante, generando gráficos de barras, boxplots en escala logarítmica y heatmaps.

## Estructura del Proyecto

*   `main.cpp` / `main.h`: Punto de entrada, orquestador de iteraciones y exportación de métricas.
*   `grafo.cpp` / `grafo.h`: Lógica base de la estructura y gestión de memoria del grafo.
*   `MedidasCentralidad.cpp` / `MedidasCentralidad.h`: Núcleo matemático y motor de algoritmos.
*   `analisis_rendimiento.py`: Script para graficar y perfilar los datos obtenidos.
*   `datasets/`: Directorio con redes de prueba (ej. *Yeast*, redes económicas *Trade*).

## Requisitos

*   Compilador C++ (C++11 o superior recomendado)
*   Python 3.x
*   Librerías Python: `pandas`, `seaborn`, `matplotlib`

## Uso

1. **Compilar el proyecto:**
   ```bash
   g++ main.cpp grafo.cpp MedidasCentralidad.cpp -o analisis_redes -O3

2. **Ejecutar el código:**
   ```bash
   ./analisis_redes
