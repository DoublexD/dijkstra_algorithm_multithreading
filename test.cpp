#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <climits>
#include <set>
#include <chrono>
#include <vector>
#include <limits>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

// Struktura krawędzi grafu
struct Edge {
    int beginning;
    int end;
    int weight;
};

// Struktura reprezentująca graf
struct Graph {
    int V; // Liczba wierzchołków
    int E; // Liczba krawędzi
    std::vector<Edge> edges; // Wektor krawędzi
    std::vector<std::vector<int>> matrix; // Macierz sąsiedztwa

    Graph(int vertices = 0, int edgesCount = 0)
        : V(vertices), E(edgesCount), edges(edgesCount), matrix(vertices, std::vector<int>(vertices, 0)) {}
};

// Funkcja wczytująca graf z pliku tekstowego
void load_graph_from_file(const std::string &filename, Graph &graph) {
    auto begin = std::chrono::high_resolution_clock::now();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku.\n";
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
    std::cout << "Operacja trwała: " << elapsed.count() << " mikrosekund.\n";
}

// Funkcja generująca losowy graf
void generate_random_graph_optimized(Graph &graph, int density, int vertices) {
    auto begin = std::chrono::high_resolution_clock::now();
    srand(static_cast<unsigned int>(time(nullptr)));

    graph.V = vertices;
    graph.E = (graph.V * (graph.V - 1) * density) / 200;

    graph.edges.resize(graph.E);
    graph.matrix.assign(graph.V, std::vector<int>(graph.V, 0));

    unsigned int num_threads = std::min(std::thread::hardware_concurrency(), static_cast<unsigned int>(8));
    int edges_per_thread = graph.E / num_threads;

    auto generate_edges = [&](int start, int end) {
        for (int i = start; i < end; ++i) {
            int u = rand() % graph.V;
            int v = rand() % graph.V;
            int weight = rand() % 10 + 1;
            graph.edges[i] = {u, v, weight};
            graph.matrix[u][v] = weight;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * edges_per_thread;
        int end = (i == num_threads - 1) ? graph.E : start + edges_per_thread;
        threads.emplace_back(generate_edges, start, end);
    }

    for (auto &t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "Operacja trwała: " << elapsed.count() << " mikrosekund.\n";
}

// Funkcja wyswietlająca graf
void display_graph(const Graph &graph) {
    auto begin = std::chrono::high_resolution_clock::now();
    std::cout << "Liczba wierzchołków: " << graph.V << "\n";
    std::cout << "Gestosc: " << static_cast<double>(graph.E) / (graph.V * (graph.V - 1) / 2) * 100 << "%\n";
    std::cout << "Liczba krawędzi: " << graph.E << "\n";

    std::cout << "\nGraf macierzowy:\n";
    for (const auto &row : graph.matrix) {
        for (const auto &weight : row) {
            std::cout << weight << " ";
        }
        std::cout << "\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "Operacja trwała: " << elapsed.count() << " mikrosekund.\n";
}

// Funkcja zapisująca graf do pliku tekstowego
void save_graph(const Graph &graph, const std::string &filename) {
    auto begin = std::chrono::high_resolution_clock::now();
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Nie można otworzyć pliku do zapisu.\n";
        return;
    }

    file << graph.E << " " << graph.V << "\n";

    for (const auto &edge : graph.edges) {
        file << edge.beginning << " " << edge.end << " " << edge.weight << "\n";
    }

    std::cout << "Graf został zapisany do pliku: " << filename << "\n";
    file.close();

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "Operacja trwała: " << elapsed.count() << " mikrosekund.\n";
}



// Funkcja wykonująca algorytm Dijkstry (poprawiona wersja)
void dijkstra_matrix_multithreaded(const Graph &graph, int start_vertex, int end_vertex) {
    auto begin = std::chrono::high_resolution_clock::now();
    int V = graph.V;
    std::vector<std::atomic<int>> dist(V);
    std::vector<bool> visited(V, false);
    for (int i = 0; i < V; ++i) dist[i] = INT_MAX;
    dist[start_vertex] = 0;

    std::mutex mtx; // Mutex do synchronizacji
    int num_threads = std::min(std::thread::hardware_concurrency(), 4u);

    for (int count = 0; count < V - 1; ++count) {
        int min_vertex = -1;
        int min_dist = INT_MAX;

        // Znajdowanie wierzchołka o minimalnej odległości (równoległe)
        auto find_min_distance = [&](int thread_id) {
            for (int v = thread_id; v < V; v += num_threads) {
                if (!visited[v] && dist[v] < min_dist) {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (dist[v] < min_dist) {
                        min_dist = dist[v];
                        min_vertex = v;
                    }
                }
            }
        };

        std::vector<std::thread> threads;
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back(find_min_distance, t);
        }
        for (auto &t : threads) {
            t.join();
        }

        if (min_vertex == -1) break;
        visited[min_vertex] = true;

        // Przetwarzanie sąsiadów równolegle
        auto process_neighbors = [&](int thread_id) {
            for (int v = thread_id; v < V; v += num_threads) {
                if (!visited[v] && graph.matrix[min_vertex][v] != 0 &&
                    dist[min_vertex] != INT_MAX &&
                    dist[min_vertex] + graph.matrix[min_vertex][v] < dist[v]) {
                    dist[v] = dist[min_vertex] + graph.matrix[min_vertex][v];
                }
            }
        };

        threads.clear();
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back(process_neighbors, t);
        }
        for (auto &t : threads) {
            t.join();
        }
    }

    std::cout << "Najkrótsza odległość z wierzchołka " << start_vertex << " do " << end_vertex
              << " wynosi: " << dist[end_vertex] << "\n";

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
    std::cout << "Operacja trwała: " << elapsed.count() << " mikrosekund.\n";
}

int main() {
    Graph myGraph;
    std::string filename;
    int choice;

    while (true) {
        std::cout << "==================================\n";
        std::cout << "               MENU               \n";
        std::cout << "==================================\n";
        std::cout << "1. Wczytaj graf z pliku tekstowego\n";
        std::cout << "2. Generowanie losowego grafu\n";
        std::cout << "3. Wyswietlenie grafu\n";
        std::cout << "4. Zapisanie grafu do pliku\n";
        std::cout << "5. Uruchom algorytm Dijkstry (macierzowo, wielowątkowo)\n";
        std::cout << "6. Wyjście\n";
        std::cin >> choice;

        switch (choice) {
            case 1: {
                std::cout << "Podaj nazwę pliku: ";
                std::cin >> filename;
                load_graph_from_file(filename, myGraph);
                break;
            }
            case 2: {
                int density, vertices;
                std::cout << "Podaj gęstość grafu (%): ";
                std::cin >> density;
                std::cout << "Podaj liczbę wierzchołków: ";
                std::cin >> vertices;
                generate_random_graph_optimized(myGraph, density, vertices);
                break;
            }
            case 3:
                display_graph(myGraph);
                break;
            case 4:
                std::cout << "Podaj nazwę pliku do zapisu: ";
                std::cin >> filename;
                save_graph(myGraph, filename);
                break;
            case 5: {
                int start_vertex, end_vertex;
                std::cout << "Podaj wierzchołek początkowy: ";
                std::cin >> start_vertex;
                std::cout << "Podaj wierzchołek końcowy: ";
                std::cin >> end_vertex;
                dijkstra_matrix_multithreaded(myGraph, start_vertex, end_vertex);
                break;
            }
            case 6:
                return 0;
            default:
                std::cout << "Nieprawidłowy wybór. Spróbuj ponownie.\n";
        }
    }
}
