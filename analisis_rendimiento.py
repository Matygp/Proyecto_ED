"""
analisis_rendimiento.py
=======================
Script de análisis de rendimiento para el proyecto de Métricas de Centralidad de Redes.
Estructura de Datos — Grupo 14 (Bastián Pérez, Matías García, Vicente Miranda)

Genera los siguientes artefactos:
  1. Tabla resumen estadístico (μ, σ) por métrica y dataset, impresa en consola.
  2. Gráfico de barras agrupadas: consumo de memoria (MB) por métrica y dataset.
  3. Boxplot: distribución de tiempos de ejecución para el dataset 'yeast', con
     escala logarítmica para manejar el amplio rango dinámico (degree ~0.01 ms
     vs betweenness ~31 000 ms).
  4. Heatmap comparativo: tiempos medios log-normalizados para ambos datasets.

Uso:
    python analisis_rendimiento.py

Requisitos:
    pip install pandas matplotlib seaborn
"""

import warnings
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np
import pandas as pd
import seaborn as sns

warnings.filterwarnings("ignore")


# ══════════════════════════════════════════════════════════════════════════════
# CONFIGURACIÓN GLOBAL
# ══════════════════════════════════════════════════════════════════════════════

# Estilo académico coherente en todos los gráficos
sns.set_theme(style="whitegrid", font_scale=1.1)
plt.rcParams.update({
    "font.family":        "DejaVu Serif",
    "axes.spines.top":    False,
    "axes.spines.right":  False,
    "figure.dpi":         120,   # resolución en pantalla (el PDF usa 300 dpi)
})

# Directorio de salida para los PDFs exportados
OUTPUT_DIR = Path("graficos")
OUTPUT_DIR.mkdir(exist_ok=True)

# Mapeo: nombre interno del CSV → etiqueta legible para los ejes
ETIQUETAS = {
    "degree":      "Degree",
    "closeness":   "Closeness",
    "avg_path":    "Avg. Shortest Path",
    "betweenness": "Betweenness",
    "hits":        "HITS",
    "pagerank":    "PageRank",
    "coreness":    "Coreness",
}

# Paleta de colores por dataset (azul → yeast, naranja → trade)
COLORES = {"yeast": "#1565C0", "trade": "#E64A19"}


# ══════════════════════════════════════════════════════════════════════════════
# 1. CARGA Y LIMPIEZA DE DATOS
# ══════════════════════════════════════════════════════════════════════════════

def cargar_datos(ruta_csv: str) -> pd.DataFrame:
    """
    Lee el CSV de rendimiento y aplica normalización mínima:
      - Convierte texto a minúsculas y elimina espacios.
      - Convierte memoria de KB a MB para mayor legibilidad.
      - Añade columna 'metrica_label' con el nombre legible de cada métrica.
    """
    df = pd.read_csv(ruta_csv)

    # Normalizar cadenas para evitar problemas de capitalización
    df["dataset"] = df["dataset"].str.lower().str.strip()
    df["metrica"] = df["metrica"].str.lower().str.strip()

    # Unidad de memoria más cómoda para graficar
    df["memoria_mb"] = df["memoria_kb"] / 1024.0

    # Etiqueta legible derivada del diccionario centralizado
    df["metrica_label"] = df["metrica"].map(ETIQUETAS)

    return df


def calcular_estadisticos(df: pd.DataFrame) -> pd.DataFrame:
    """
    Agrupa por (dataset, métrica) y calcula:
      μ (media) y σ (desviación estándar) de tiempo_ms y memoria_mb.

    Retorna un DataFrame plano con una fila por combinación dataset×métrica.
    """
    stats = (
        df.groupby(["dataset", "metrica"])
        .agg(
            mu_tiempo_ms   = ("tiempo_ms",  "mean"),
            sigma_tiempo_ms= ("tiempo_ms",  "std"),
            mu_memoria_mb  = ("memoria_mb", "mean"),
            sigma_memoria_mb=("memoria_mb", "std"),
        )
        .reset_index()
    )
    stats["metrica_label"] = stats["metrica"].map(ETIQUETAS)
    return stats


def imprimir_tabla(stats: pd.DataFrame) -> None:
    """
    Imprime en consola la tabla de estadísticos formateada,
    ordenada por dataset y por tiempo medio descendente.
    """
    tabla = stats[[
        "dataset", "metrica_label",
        "mu_tiempo_ms", "sigma_tiempo_ms",
        "mu_memoria_mb", "sigma_memoria_mb",
    ]].copy()

    tabla.columns = [
        "Dataset", "Métrica",
        "μ Tiempo (ms)", "σ Tiempo (ms)",
        "μ Memoria (MB)", "σ Memoria (MB)",
    ]
    tabla = tabla.sort_values(
        ["Dataset", "μ Tiempo (ms)"], ascending=[True, False]
    )

    separador = "─" * 80
    print(f"\n{separador}")
    print("  TABLA DE ESTADÍSTICOS DE RENDIMIENTO (μ ± σ)")
    print(separador)
    print(tabla.to_string(index=False, float_format=lambda x: f"{x:>12.4f}"))
    print(separador)


# ══════════════════════════════════════════════════════════════════════════════
# 2. EXPORTACIÓN
# ══════════════════════════════════════════════════════════════════════════════

def guardar_figura(fig: plt.Figure, nombre: str, dpi: int = 300) -> None:
    """
    Guarda una figura en formato PDF en alta resolución (300 dpi por defecto).
    El bbox_inches='tight' evita que las etiquetas queden recortadas.
    """
    ruta = OUTPUT_DIR / f"{nombre}.pdf"
    fig.savefig(ruta, dpi=dpi, bbox_inches="tight", format="pdf")
    print(f"  [✓] Exportado: {ruta}")


# ══════════════════════════════════════════════════════════════════════════════
# 3. GRÁFICO DE BARRAS — MEMORIA
# ══════════════════════════════════════════════════════════════════════════════

def grafico_memoria(stats: pd.DataFrame) -> plt.Figure:
    """
    Barras agrupadas del consumo de memoria pico (MB ± σ) por métrica.
    Cada métrica tiene dos barras, una por dataset, permitiendo comparar
    el overhead de memoria entre la red biológica (yeast) y la económica (trade).

    Detalles técnicos:
      - Las barras incluyen barras de error (±σ) para visualizar la variabilidad
        entre las 10 iteraciones.
      - El valor numérico se anota sobre cada barra para facilitar la lectura exacta.
    """
    # Orden natural de ejecución dentro del programa
    orden = list(ETIQUETAS.values())

    # Convertir la columna a Categorical para ordenar correctamente
    stats_mem = stats.copy()
    stats_mem["metrica_label"] = pd.Categorical(
        stats_mem["metrica_label"], categories=orden, ordered=True
    )
    stats_mem = stats_mem.sort_values("metrica_label")

    datasets    = sorted(stats_mem["dataset"].unique())   # ['trade', 'yeast']
    metricas    = stats_mem["metrica_label"].unique()     # categorías ordenadas
    n_metricas  = len(metricas)
    ancho       = 0.36                                    # ancho de cada barra
    x           = np.arange(n_metricas)

    fig, ax = plt.subplots(figsize=(12, 6))

    for i, ds in enumerate(datasets):
        subset = (
            stats_mem[stats_mem["dataset"] == ds]
            .sort_values("metrica_label")
        )
        # Desplazamiento horizontal para que las barras queden agrupadas
        offset = (i - len(datasets) / 2 + 0.5) * ancho

        bars = ax.bar(
            x + offset,
            subset["mu_memoria_mb"],
            width       = ancho,
            label       = ds.capitalize(),
            color       = COLORES[ds],
            alpha       = 0.85,
            edgecolor   = "white",
            linewidth   = 0.8,
            # Barras de error: ±1σ sobre las 10 iteraciones
            yerr        = subset["sigma_memoria_mb"],
            capsize     = 4,
            error_kw    = {"elinewidth": 1.2, "ecolor": "#555"},
        )

        # Anotación numérica sobre cada barra (en MB con 1 decimal)
        for bar in bars:
            h = bar.get_height()
            ax.text(
                bar.get_x() + bar.get_width() / 2.0,
                h + 0.5,                    # pequeño margen sobre la barra
                f"{h:.1f}",
                ha       = "center",
                va       = "bottom",
                fontsize = 8.0,
                color    = "0.35",
            )

    # ── Formato de ejes y título ─────────────────────────────────────
    ax.set_xticks(x)
    ax.set_xticklabels(metricas, rotation=22, ha="right", fontsize=11)
    ax.set_xlabel("Métrica de Centralidad", fontsize=12, labelpad=10)
    ax.set_ylabel("Memoria Pico Promedio (MB)", fontsize=12, labelpad=10)
    ax.set_title(
        "Consumo de Memoria por Métrica y Dataset",
        fontsize=14, fontweight="bold", pad=16,
    )
    ax.legend(title="Dataset", fontsize=11, title_fontsize=11,
              framealpha=0.9, loc="upper left")
    ax.yaxis.set_major_formatter(
        mticker.FuncFormatter(lambda v, _: f"{v:.0f} MB")
    )
    # Ampliar el eje Y para dar espacio a las anotaciones numéricas
    ax.set_ylim(0, ax.get_ylim()[1] * 1.12)

    fig.tight_layout()
    return fig


# ══════════════════════════════════════════════════════════════════════════════
# 4. BOXPLOT — DISTRIBUCIÓN DE TIEMPOS (YEAST)
# ══════════════════════════════════════════════════════════════════════════════

def grafico_boxplot_yeast(df: pd.DataFrame) -> plt.Figure:
    """
    Boxplot con escala logarítmica en el eje Y para el dataset 'yeast'.
    Las métricas se ordenan de MAYOR a MENOR tiempo promedio, facilitando
    la identificación inmediata de los cuellos de botella algorítmicos.

    Superponemos los 10 puntos individuales de cada métrica (stripplot) para
    mostrar la densidad real de los datos en lugar de ocultarlos en el cuartil.

    Escala logarítmica: necesaria porque el rango dinámico abarca ~7 órdenes
    de magnitud (Degree ~0.01 ms  vs  Betweenness ~31 000 ms).
    """
    yeast = df[df["dataset"] == "yeast"].copy()

    # Calcular el orden: métrica de mayor media primero
    orden_desc = (
        yeast.groupby("metrica_label")["tiempo_ms"]
        .mean()
        .sort_values(ascending=False)
        .index.tolist()
    )

    fig, ax = plt.subplots(figsize=(11, 6))

    # ── Caja y bigotes ────────────────────────────────────────────────
    sns.boxplot(
        data       = yeast,
        x          = "metrica_label",
        y          = "tiempo_ms",
        order      = orden_desc,
        palette    = "Blues_d",
        width      = 0.48,
        linewidth  = 1.3,
        flierprops = {
            "marker":          "D",
            "markerfacecolor": "#90CAF9",
            "markeredgecolor": "#1565C0",
            "markersize":      5,
            "alpha":           0.7,
        },
        ax = ax,
    )

    # ── Puntos individuales superpuestos (n = 10 por métrica) ─────────
    sns.stripplot(
        data   = yeast,
        x      = "metrica_label",
        y      = "tiempo_ms",
        order  = orden_desc,
        color  = "#0D47A1",
        size   = 5.5,
        alpha  = 0.50,
        jitter = 0.18,
        ax     = ax,
    )

    # ── Escala logarítmica y formato del eje Y ────────────────────────
    ax.set_yscale("log")
    ax.yaxis.set_major_formatter(
        mticker.FuncFormatter(lambda v, _: f"{v:,.4g} ms")
    )
    ax.yaxis.set_minor_formatter(mticker.NullFormatter())

    # ── Anotación de la media como línea horizontal dentro de cada caja ─
    medias = yeast.groupby("metrica_label")["tiempo_ms"].mean()
    for pos, metrica in enumerate(orden_desc):
        mu = medias[metrica]
        ax.hlines(
            mu, pos - 0.22, pos + 0.22,
            colors="red", linewidths=1.6, linestyles="--",
            label="Media (μ)" if pos == 0 else "",
            zorder=5,
        )

    # ── Etiquetas y título ────────────────────────────────────────────
    ax.set_xticklabels(
        [t.get_text() for t in ax.get_xticklabels()],
        rotation=20, ha="right", fontsize=11,
    )
    ax.set_xlabel("Métrica de Centralidad", fontsize=12, labelpad=10)
    ax.set_ylabel("Tiempo de Ejecución (ms) — escala log₁₀", fontsize=12, labelpad=10)
    ax.set_title(
        "Distribución de Tiempos de Ejecución — Dataset Yeast PPI\n"
        "ordenado de mayor a menor tiempo promedio  |  n = 10 iteraciones",
        fontsize=13, fontweight="bold", pad=16,
    )
    ax.legend(fontsize=10, loc="upper right", framealpha=0.9)
    ax.grid(axis="y", which="both", alpha=0.3)

    fig.tight_layout()
    return fig


# ══════════════════════════════════════════════════════════════════════════════
# 5. HEATMAP — TIEMPOS MEDIOS COMPARATIVOS
# ══════════════════════════════════════════════════════════════════════════════

def grafico_heatmap(stats: pd.DataFrame) -> plt.Figure:
    """
    Heatmap de los tiempos medios de ejecución (ms) para los dos datasets.
    La intensidad del color se basa en log₁₀(μ + 1) para manejar el amplio
    rango de valores sin perder detalle en las métricas rápidas.
    Las celdas muestran el valor exacto de μ en ms como anotación.

    Este gráfico permite comparar en una sola vista:
      - Qué métricas dominan el tiempo de cómputo (fila = dataset).
      - Cómo difieren los tiempos entre yeast (grafo disperso) y trade (denso).
    """
    # Tabla pivote: filas = dataset, columnas = métrica
    pivot = stats.pivot(
        index="dataset", columns="metrica_label", values="mu_tiempo_ms"
    )

    # Ordenar columnas por tiempo en yeast (de mayor a menor)
    col_order = pivot.loc["yeast"].sort_values(ascending=False).index.tolist()
    pivot = pivot[col_order]

    # Escala de color logarítmica para evitar que los extremos dominen
    pivot_log = np.log10(pivot + 1)

    # Formatear anotaciones: número legible con separador de miles
    def fmt(v: float) -> str:
        if v >= 1000:
            return f"{v:,.1f}"
        elif v >= 1:
            return f"{v:.2f}"
        else:
            return f"{v:.4f}"

    annot = pivot.map(fmt)          # pandas 2.x usa .map en lugar de .applymap

    fig, ax = plt.subplots(figsize=(13, 3.8))

    sns.heatmap(
        pivot_log,
        annot      = annot,
        fmt        = "s",
        cmap       = "YlOrRd",
        linewidths = 0.6,
        linecolor  = "white",
        ax         = ax,
        cbar_kws   = {
            "label":    "log₁₀(μ tiempo ms + 1)",
            "shrink":   0.85,
            "pad":      0.02,
        },
    )

    ax.set_title(
        "Tiempos Medios de Ejecución por Métrica y Dataset\n"
        "[anotaciones en ms  |  intensidad de color = escala log₁₀]",
        fontsize=13, fontweight="bold", pad=14,
    )
    ax.set_xlabel("Métrica de Centralidad", fontsize=12, labelpad=8)
    ax.set_ylabel("Dataset", fontsize=12, labelpad=8)

    # Capitalizar etiquetas del eje Y
    ax.set_yticklabels(
        [lbl.get_text().capitalize() for lbl in ax.get_yticklabels()],
        rotation=0, fontsize=11,
    )
    ax.set_xticklabels(
        ax.get_xticklabels(), rotation=20, ha="right", fontsize=11
    )

    fig.tight_layout()
    return fig


# ══════════════════════════════════════════════════════════════════════════════
# PUNTO DE ENTRADA PRINCIPAL
# ══════════════════════════════════════════════════════════════════════════════

def main():
    RUTA_CSV = "rendimiento.csv"

    print("══ Análisis de Rendimiento — Métricas de Centralidad de Redes ══")

    # ── Fase 1: carga y estadísticos ─────────────────────────────────
    print("\n[1/4] Cargando y procesando datos...")
    df    = cargar_datos(RUTA_CSV)
    stats = calcular_estadisticos(df)
    imprimir_tabla(stats)

    # ── Fase 2: gráfico de memoria ────────────────────────────────────
    print("\n[2/4] Generando gráfico de memoria...")
    fig_mem = grafico_memoria(stats)
    guardar_figura(fig_mem, "01_memoria_por_metrica")

    # ── Fase 3: boxplot de tiempos (yeast) ───────────────────────────
    print("\n[3/4] Generando boxplot de tiempos (yeast)...")
    fig_box = grafico_boxplot_yeast(df)
    guardar_figura(fig_box, "02_boxplot_tiempos_yeast")

    # ── Fase 4: heatmap comparativo ──────────────────────────────────
    print("\n[4/4] Generando heatmap comparativo...")
    fig_heat = grafico_heatmap(stats)
    guardar_figura(fig_heat, "03_heatmap_tiempos")

    print("\n══ Proceso completado. PDFs guardados en ./graficos/ ══")
    plt.show()


if __name__ == "__main__":
    main()
