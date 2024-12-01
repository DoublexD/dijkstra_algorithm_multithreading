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
void generate_random_graph_optimized(Graph &graph, int density, int vertice) {
    auto begin = std::chrono::high_resolution_clock::now();
    srand(static_cast<unsigned int>(time(nullptr)));

    graph.V = vertice;
    graph.E = (graph.V * (graph.V - 1) * density) / 200;

    graph.edges.resize(graph.E);
    graph.matrix.assign(graph.V, std::vector<int>(graph.V, 0));

    unsigned int num_threads = std::min(std::thread::hardware_concurrency(), static_cast<unsigned int>(8)); // Ogranicz liczbe wątków do 8
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

    // Wyswietlanie grafu macierzowego
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

// Funkcja wykonujaca algorytm Dijkstry dla reprezentacji macierzowej
void dijkstry_matrix_multithreaded(const Graph &graph, int start_vertex, int end_vertex) {
    auto begin = std::chrono::high_resolution_clock::now();
    int V = graph.V;
    std::vector<std::atomic<int>> dist(V);
    std::vector<bool> visited(V, false);
    for (int i = 0; i < V; ++i) dist[i] = INT_MAX;
    dist[start_vertex] = 0;

    int num_threads = std::min(std::thread::hardware_concurrency(), 8u);
    std::mutex mtx; // Do synchronizacji wątków

    for (int count = 0; count < V - 1; ++count) {
        std::atomic<int> min_dist(INT_MAX);
        int min_vertex = -1;

        // Znajdowanie wierzchołka o minimalnej odległości (Parallel Reduction)
        auto find_min_distance = [&](int thread_id) {
            int local_min_dist = INT_MAX;
            int local_min_vertex = -1;
            for (int v = thread_id; v < V; v += num_threads) {
                if (!visited[v] && dist[v] < local_min_dist) {
                    local_min_dist = dist[v].load();
                    local_min_vertex = v;
                }
            }
            std::lock_guard<std::mutex> lock(mtx);
            if (local_min_dist < min_dist) {
                min_dist = local_min_dist;
                min_vertex = local_min_vertex;
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
                    int new_dist = dist[min_vertex] + graph.matrix[min_vertex][v];
                    dist[v].store(std::min(dist[v].load(), new_dist));
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

    std::cout << "W reprezentacji macierzowej (wielowątkowa) najkrótsza odległość z wierzchołka " << start_vertex
              << " do " << end_vertex << " wynosi: " << dist[end_vertex] << "\n";

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    std::cout << "Operacja trwała: " << elapsed.count() << " mikrosekund.\n";
}

// Funkcja wykonująca algorytm Dijkstry dla reprezentacji listowej
void dijkstry_list(const Graph &graph, int start_vertex, int end_vertex) {
    auto begin = std::chrono::high_resolution_clock::now();
    int V = graph.V;
    std::vector<std::atomic<int>> dist(V);
    for (int i = 0; i < V; ++i) dist[i] = INT_MAX;
    dist[start_vertex] = 0;

    // Kolejka priorytetowa (waga, wierzchołek) z mutexem
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
    std::mutex pq_mutex;
    pq.push({0, start_vertex});

    while (!pq.empty()) {
        pq_mutex.lock();
        int current_dist = pq.top().first;
        int current_vertex = pq.top().second;
        pq.pop();
        pq_mutex.unlock();

        if (current_dist > dist[current_vertex]) continue;

        for (const auto &edge : graph.edges) {
            if (edge.beginning == current_vertex) {
                int neighbor = edge.end;
                int weight = edge.weight;

                if (dist[current_vertex] != INT_MAX && dist[current_vertex] + weight < dist[neighbor]) {
                    int new_dist = dist[current_vertex] + weight;
                    dist[neighbor].store(std::min(dist[neighbor].load(), new_dist));
                    pq_mutex.lock();
                    pq.push({dist[neighbor], neighbor});
                    pq_mutex.unlock();
                }
            }
        }
    }

    std::cout << "W reprezentacji listowej najkrotsza odleglosc z wierzcholka " << start_vertex << " do "
              << end_vertex << " wynosi: " << dist[end_vertex] << "\n";
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);

    std::cout << "Operacja trwala: " << elapsed.count() << " mikrosekund.\n";
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
        std::cout << "3. Wyswietlanie grafu (macierzowo oraz listowo)\n";
        std::cout << "4. Zapisz graf do pliku\n";
        std::cout << "5. Uruchom algorytm Dijkstry dla wczytanego/wygenerowanego grafu\n";
        std::cout << "6. Zakoncz program\n";
        std::cout << "==================================\n";
        std::cin >> choice;

        switch (choice) {
            case 1:
                std::cout << "Wybrano wczytanie grafu z pliku tekstowego\n";
                std::cout << "Wpisz nazwe pliku (z rozszerzeniem '.txt')\n";
                std::cin >> filename;
                load_graph_from_file(filename, myGraph);
                std::cout << "Graf zostal wczytany pomyslnie\n";
                break;

            case 2:
                int density, vertice;
                std::cout << "Wybrano generowanie losowego grafu\n";
                std::cout << "Wpisz gestosc grafu (wystarczy sama liczba, nie trzeba pisac znaku '%')\n";
                std::cin >> density;
                std::cout << "Wpisz liczbe wierzcholkow grafu\n";
                std::cin >> vertice;
                generate_random_graph_optimized(myGraph, density, vertice);
                std::cout << "Graf zostal wygenerowany pomyslnie\n";
                break;

            case 3:
                std::cout << "Wybrano wyswietlenie grafu\n";
                display_graph(myGraph);
                break;

            case 4:
                std::cout << "Wybrano zapisanie grafu do pliku\n";
                std::cout << "Napisz nazwe pliku (z rozszerzeniem '.txt')\n";
                std::cin >> filename;
                save_graph(myGraph, filename);
                break;

            case 5:
                int first_vertice, last_vertice;
                std::cout << "Wybrano uruchomienie algorytmu Dijkstry (listowo/macierzowo)\n";
                std::cout << "Podaj wierzcholek poczatkowy\n";
                std::cin >> first_vertice;
                std::cout << "Podaj wierzcholek koncowy\n";
                std::cin >> last_vertice;
                dijkstry_matrix_multithreaded(myGraph, first_vertice, last_vertice);
                dijkstry_list(myGraph, first_vertice, last_vertice);
                break;

            case 6:
                return 0;

            default:
                std::cout << "Nieprawidlowy wybor. Sprobuj ponownie.\n";
        }
    }
}