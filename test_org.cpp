#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <set>
#include <chrono>
#include <vector>
#include <string>

// README
// ============================================================================================================
// Indeks studenta: 264164
// Indeks studenta: 269177
// ============================================================================================================

// Zmienne
std::string filename; // Nazwa pliku tekstowego

// Struktura krawedzi grafu
struct Edge {
    int beginning;
    int end;
    int weight;
};

// Struktura reprezentujaca graf
struct Graph {
    int V; // Liczba wierzcholkow
    int E; // Liczba krawedzi
    std::vector<Edge> edges; // Wektor krawedzi
    std::vector<std::vector<int>> matrix; // Macierz sasiedztwa

    // Konstruktor grafu
    Graph(int vertices = 0, int edgesCount = 0) : V(vertices), E(edgesCount), edges(edgesCount), matrix(vertices, std::vector<int>(vertices, 0)) {}
};

// Funkcja wczytujaca graf z pliku tekstowego
void load_graph_from_file(const std::string &filename, Graph &graph) {
    auto begin = std::chrono::high_resolution_clock::now();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Nie mozna otworzyc pliku.\n";
        exit(EXIT_FAILURE);
    }

    file >> graph.E >> graph.V;

    graph.edges.resize(graph.E);
    graph.matrix.assign(graph.V, std::vector<int>(graph.V, 0));

    for (int i = 0; i < graph.E; ++i) {
        file >> graph.edges[i].beginning >> graph.edges[i].end >> graph.edges[i].weight;
        graph.matrix[graph.edges[i].beginning][graph.edges[i].end] = graph.edges[i].weight;
    }

    file.close();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    std::cout << "Operacja trwala: " << elapsed.count() << " mikrosekund.\n";
}

// Funkcja generujaca graf
void generate_random_graph(Graph &graph, int density, int vertice) {
    auto begin = std::chrono::high_resolution_clock::now();
    srand(static_cast<unsigned int>(time(nullptr)));

    graph.V = vertice;
    graph.E = (graph.V * (graph.V - 1) * density) / 200;

    graph.edges.resize(graph.E);
    graph.matrix.assign(graph.V, std::vector<int>(graph.V, 0));

    for (int i = 0; i < graph.E; ++i) {
        int u = rand() % graph.V;
        int v = rand() % graph.V;
        int weight = rand() % 10 + 1;
        graph.edges[i] = {u, v, weight};
        graph.matrix[u][v] = weight;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    std::cout << "Operacja trwala: " << elapsed.count() << " mikrosekund.\n";
}

// Funkcja wyswietlajaca graf
void display_graph(const Graph &graph) {
    auto begin = std::chrono::high_resolution_clock::now();
    std::cout << "Liczba wierzcholkow: " << graph.V << "\n";
    std::cout << "Gestosc: " << static_cast<double>(graph.E) / (graph.V * (graph.V - 1) / 2) * 100 << "%" << "\n";
    std::cout << "Liczba krawedzi: " << graph.E << "\n";
    std::cout << "Opis krawedzi:\n";
    for (const auto &edge : graph.edges) {
        std::cout << "Poczatek: " << edge.beginning << ", Koniec: " << edge.end
                  << ", Waga/przepustowosc: " << edge.weight << "\n";
    }

    std::cout << "\nGraf macierzowy:\n";
    for (const auto &row : graph.matrix) {
        for (const auto &weight : row) {
            std::cout << weight << " ";
        }
        std::cout << "\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    std::cout << "Cala operacja trwala: " << elapsed.count() << " mikrosekund.\n";
}

// Funkcja zapisujaca graf do pliku tekstowego
void save_graph(const Graph &graph, const std::string &filename) {
    auto begin = std::chrono::high_resolution_clock::now();
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Nie mozna otworzyc pliku do zapisu.\n";
        return;
    }

    file << graph.E << " " << graph.V << "\n";

    for (const auto &edge : graph.edges) {
        file << edge.beginning << " " << edge.end << " " << edge.weight << "\n";
    }
    std::cout << "Graf zostal zapisany w pliku tekstowym pod nazwa " << filename << ".\n";
    file.close();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    std::cout << "Operacja trwala: " << elapsed.count() << " mikrosekund.\n";
}

// Algorytm Dijkstry
void dijkstra_matrix(const Graph &graph, int start_vertex, int end_vertex) {
    auto begin = std::chrono::high_resolution_clock::now();

    int V = graph.V;
    std::vector<int> dist(V, INT_MAX);
    std::vector<bool> visited(V, false);

    dist[start_vertex] = 0;

    for (int i = 0; i < V - 1; ++i) {
        int min_dist = INT_MAX, min_vertex = -1;

        for (int v = 0; v < V; ++v) {
            if (!visited[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                min_vertex = v;
            }
        }

        if (min_vertex == -1) break;

        visited[min_vertex] = true;

        for (int v = 0; v < V; ++v) {
            if (!visited[v] && graph.matrix[min_vertex][v] &&
                dist[min_vertex] + graph.matrix[min_vertex][v] < dist[v]) {
                dist[v] = dist[min_vertex] + graph.matrix[min_vertex][v];
            }
        }
    }

    std::cout << "Najkrotsza odleglosc z " << start_vertex << " do " << end_vertex << " wynosi: "
              << dist[end_vertex] << "\n";

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    std::cout << "Czas wykonania: " << elapsed.count() << " mikrosekund.\n";
}

int main() {
    Graph myGraph;
    int choice;

    while (true) {
        std::cout << "==================================\n";
        std::cout << "               MENU               \n";
        std::cout << "==================================\n";
        std::cout << "1. Wczytaj graf z pliku tekstowego\n";
        std::cout << "2. Generowanie losowego grafu\n";
        std::cout << "3. Wyswietlenie grafu\n";
        std::cout << "4. Zapisanie grafu do pliku\n";
        std::cout << "5. Algorytm Dijkstry\n";
        std::cout << "6. Wyjście\n";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Podaj nazwę pliku: ";
                std::cin >> filename;
                load_graph_from_file(filename, myGraph);
                break;
            case 2:
                int density, vertices;
                std::cout << "Podaj gęstość grafu (%): ";
                std::cin >> density;
                std::cout << "Podaj liczbę wierzchołków: ";
                std::cin >> vertices;
                generate_random_graph(myGraph, density, vertices);
                break;
            case 3:
                display_graph(myGraph);
                break;
            case 4:
                std::cout << "Podaj nazwę pliku do zapisu: ";
                std::cin >> filename;
                save_graph(myGraph, filename);
                break;
            case 5:
                int start, end;
                std::cout << "Podaj wierzchołek początkowy: ";
                std::cin >> start;
                std::cout << "Podaj wierzchołek końcowy: ";
                std::cin >> end;
                dijkstra_matrix(myGraph, start, end);
                break;
            case 6:
                return 0;
            default:
                std::cout << "Nieprawidłowy wybór, spróbuj ponownie.\n";
        }
    }
}
