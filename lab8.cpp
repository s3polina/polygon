#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <queue>
#include <stack>
#include <cmath>
#include <limits>
#include <chrono>
#include <iomanip>
#include <cassert>

using namespace std;

// Структура для представления узла графа
struct Node {
    double lon = 0.0; // Долгота узла
    double lat = 0.0; // Широта узла
    vector<pair<Node*, double>> edges; // Вектор рёбер (соседей) и их весов
};

// Класс для представления графа
class Graph {
public:
    // Метод для добавления узла в граф
    Node* addNode(double lon, double lat) {
        ostringstream keyStream;
        // Форматирование ключа для хранения узла
        keyStream << fixed << setprecision(10) << lon << "," << lat;
        string key = keyStream.str();

        // Проверка, существует ли узел с такими координатами
        auto it = node_map.find(key);
        if (it != node_map.end()) {
            return it->second; // Возвращаем уже существующий узел
        }

        // Создание нового узла и добавление его в вектор
        auto node = std::make_unique<Node>();
        node->lon = lon;
        node->lat = lat;
        Node* nodePtr = node.get();
        nodes.push_back(move(node));
        node_map[key] = nodePtr; // Сохранение указателя на узел в хэш-таблице
        return nodePtr;
    }

    // Метод для получения узла по координатам
    Node* getNode(double lon, double lat) {
        ostringstream keyStream;
        keyStream << fixed << setprecision(10) << lon << "," << lat;
        string key = keyStream.str();

        auto it = node_map.find(key);
        if (it != node_map.end()) {
            return it->second; // Узел найден
        }
        return nullptr; // Узел не найден
    }

    // Метод для чтения графа из файла
    void readFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "could not open the fail: " << filename << "\n"; // Ошибка при открытии файла
            return;
        }

        string line;
        // Чтение файла построчно
        while (getline(file, line)) {
            istringstream lineStream(line);
            string parentData, edgesData;
            // Чтение родительских данных до двоеточия
            if (getline(lineStream, parentData, ':')) {
                replace(parentData.begin(), parentData.end(), ',', ' '); // Замена запятой на пробел
                istringstream parentStream(parentData);

                double lon1, lat1;
                if (!(parentStream >> lon1 >> lat1)) { // Парсинг координат родительского узла
                   cerr << "error of knot: " << parentData << "\n";
                    continue; 
                }

                Node* parentNode = addNode(lon1, lat1); // Добавление родительского узла

                // Чтение данных о рёбрах
                while (getline(lineStream, edgesData, ';')) {
                    replace(edgesData.begin(), edgesData.end(), ',', ' '); // Замена запятой на пробел
                    istringstream edgeStream(edgesData);

                    double lon2, lat2, weight;
                    if (!(edgeStream >> lon2 >> lat2 >> weight)) { // Парсинг координат дочернего узла и веса
                        cerr << "error of edge: " << edgesData << "\n";
                        continue;
                    }

                    Node* childNode = addNode(lon2, lat2); // Добавление дочернего узла

                    // Создание рёбер между узлом и дочерним узлом
                    parentNode->edges.emplace_back(childNode, weight);
                    childNode->edges.emplace_back(parentNode, weight); // Данные о рёбрах хранятся в обоих узлах
                }
            }
        }

        file.close(); // Закрытие файла
    }

    // Метод для поиска ближайшего узла к заданным координатам
    Node* findClosestNode(double lat, double lon) const {
        double minDist = std::numeric_limits<double>::max(); // Начальное большое значение для минимального расстояния
        Node* closest = nullptr; // Указатель на ближайший узел

        // Перебор всех узлов графа для поиска ближайшего
        for (auto& up : const_cast<Graph*>(this)->nodes) {
            Node* n = up.get();
            double dx = n->lon - lon; // Разница в долготе
            double dy = n->lat - lat; // Разница в широте
            double dist = std::sqrt(dx * dx + dy * dy); // Евклидово расстояние
            if (dist < minDist) {
                minDist = dist; // Обновление минимального расстояния
                closest = n; // Обновление ближайшего узла
            }
        }
        return closest; // Возврат ближайшего узла
    }

    // Метод для вывода пути
    void printPath(const vector<Node*>& path) const {
        if (path.empty()) {
            cout << "distance not found.\n"; // Если путь пуст
            return;
        }
        double totalWeight = 0.0; // Общий вес пути
        cout << "distance:\n";
        for (size_t i = 0; i < path.size(); ++i) {
            cout << "(" << path[i]->lat << ", " << path[i]->lon << ")"; // Вывод координат
            if (i < path.size() - 1) {
                cout << " -> ";
                Node* cur = path[i];
                Node* nxt = path[i + 1];
                // Найти вес ребра между текущим и следующим узлом
                for (auto& edge : cur->edges) {
                    if (edge.first == nxt) {
                        totalWeight += edge.second; // Суммирование весов
                        break;
                    }
                }
            }
        }
        cout << "\ntotal length of the path: " << totalWeight << "\n"; // Вывод общей длины пути
    }

private:
    vector<unique_ptr<Node>> nodes; // Вектор уникальных указателей на узлы
    unordered_map<string, Node*> node_map; // Хэш-таблица для хранения узлов по ключу
};

// Поиск в ширину
vector<Node*> BFS(Node* start, Node* goal) {
    if (!start || !goal) return {}; // Проверка наличия узлов

    unordered_map<Node*, bool> visited; // Хранилище посещённых узлов
    unordered_map<Node*, Node*> cameFrom; // Хранилище для построения пути
    queue<Node*> q; // Очередь для BFS

    q.push(start); // Добавление стартового узла в очередь
    visited[start] = true;

    while (!q.empty()) {
        Node* current = q.front(); // Текущий узел
        q.pop();

        if (current == goal) { // Если достигли цели
            vector<Node*> path; // Создание пути
            for (Node* p = goal; p; p = cameFrom[p]) {
                path.push_back(p); // Добавление узлов в путь
            }
            reverse(path.begin(), path.end()); // Обратный порядок
            return path; // Возврат найденного пути
        }

        // Обработка соседей
        for (auto& edge : current->edges) {
            Node* neighbor = edge.first; // Соседний узел
            if (!visited[neighbor]) { // Если сосед не посещен
                visited[neighbor] = true; // Отмечаем как посещенный
                cameFrom[neighbor] = current; // Запоминаем предшественника
                q.push(neighbor); // Добавление соседа в очередь
            }
        }
    }
    return {}; // Возврат пустого пути, если не нашли цель
}

// Поиск в глубину
vector<Node*> DFS(Node* start, Node* goal) {
    if (!start || !goal) return {}; // Проверка наличия узлов

    unordered_map<Node*, bool> visited; // Хранилище посещённых узлов
    unordered_map<Node*, Node*> cameFrom; // Хранилище для построения пути
    stack<Node*> st; // Стек для DFS

    st.push(start); // Добавление стартового узла в стек
    visited[start] = true;

    while (!st.empty()) {
        Node* current = st.top(); // Текущий узел
        st.pop();

        if (current == goal) { // Если достигли цели
            vector<Node*> path; // Создание пути
            for (Node* p = goal; p; p = cameFrom[p]) {
                path.push_back(p); // Добавление узлов в путь
            }
            reverse(path.begin(), path.end()); // Обратный порядок
            return path;
        }

        // Обработка соседей
        for (auto& edge : current->edges) {
            Node* neighbor = edge.first; // Соседний узел
            if (!visited[neighbor]) { // Если сосед не посещен
                visited[neighbor] = true; // Отмечаем как посещенный
                cameFrom[neighbor] = current; // Запоминаем предшественника
                st.push(neighbor); // Добавление соседа в стек
            }
        }
    }
    return {}; // Возврат пустого пути, если не нашли цель
}

// Алгоритм Дейкстры
vector<Node*> Dijkstra(Graph& graph, Node* start, Node* goal) {
    if (!start || !goal) return {}; // Проверка наличия узлов

    unordered_map<Node*, double> dist; // Хранилище расстояний до узлов
    unordered_map<Node*, Node*> cameFrom; // Хранилище для построения пути
    unordered_map<Node*, bool> visited; // Хранилище посещённых узлов

    dist[start] = 0.0; // Начальное расстояние до стартового узла

    using PQItem = pair<double, Node*>; // Пара (расстояние, узел) для приоритетной очереди
    priority_queue<PQItem, vector<PQItem>, greater<>> pq; // Приоритетная очередь
    pq.emplace(0.0, start); // Добавление стартового узла

    while (!pq.empty()) {
        PQItem topPair = pq.top(); // Текущий узел с минимальным расстоянием
        pq.pop();

        double currentDist = topPair.first; // Удаляемый узел
        Node* currentNode = topPair.second; // Текущий узел

        if (visited[currentNode]) {
            continue; // Если узел уже посещен, пропускаем
        }
        visited[currentNode] = true;

        if (currentNode == goal) { // Если достигли цели
            vector<Node*> path; // Создание пути
            for (Node* p = goal; p; p = cameFrom[p]) {
                path.push_back(p); // Добавление узлов в путь
            }
            reverse(path.begin(), path.end()); // Обратный порядок
            return path;
        }

        // Обработка соседей
        for (auto& edge : currentNode->edges) {
            Node* neighbor = edge.first; // Соседний узел
            double weight = edge.second; // Вес ребра
            double alt = currentDist + weight; // Альтернативное расстояние

            if (dist.find(neighbor) == dist.end()) {
                dist[neighbor] = std::numeric_limits<double>::infinity(); // Устанавливаем начальное расстояние
            }
            if (alt < dist[neighbor]) { // Обновление расстояния до соседа
                dist[neighbor] = alt;
                cameFrom[neighbor] = currentNode; // Запоминаем предшественника
                pq.emplace(alt, neighbor); // Добавление соседа в очередь
            }
        }
    }

    return {}; // Возврат пустого пути, если не нашли цель
}

// Эвристика для A*
static double heuristic(Node* a, Node* b) {
    double dx = a->lon - b->lon; // Разница в долготе
    double dy = a->lat - b->lat; // Разница в широте
    return sqrt(dx * dx + dy * dy); // Евклидово расстояние
}

// Алгоритм A*
vector<Node*> AStar(Graph& graph, Node* start, Node* goal) {
    if (!start || !goal) return {}; // Проверка наличия узлов

    unordered_map<Node*, double> gScore; // Хранилище стоимости пути от старта до узла
    unordered_map<Node*, double> fScore; // Хранилище общей стоимости
    unordered_map<Node*, Node*> cameFrom; // Хранилище для построения пути
    unordered_map<Node*, bool> visited; // Хранилище посещённых узлов

    gScore[start] = 0.0; // Начальная стоимость
    fScore[start] = heuristic(start, goal); // Эвристическая стоимость

    using PQItem = pair<double, Node*>; // Пара (стоимость, узел) для приоритетной очереди
    priority_queue<PQItem, vector<PQItem>, greater<>> openSet; // Приоритетная очередь
    openSet.emplace(fScore[start], start); // Добавление стартового узла

    while (!openSet.empty()) {
        PQItem topPair = openSet.top(); // Текущий узел с минимальной стоимостью
        openSet.pop();

        double currentF = topPair.first; // Текущая стоимость
        Node* currentNode = topPair.second; // Текущий узел

        if (currentNode == goal) { // Если достигли цели
            vector<Node*> path; // Создание пути
            for (Node* p = goal; p; p = cameFrom[p]) {
                path.push_back(p); // Добавление узлов в путь
            }
            reverse(path.begin(), path.end()); // Обратный порядок
            return path;
        }

        visited[currentNode] = true; // Отмечаем как посещенный

        // Обработка соседей
        for (auto& edge : currentNode->edges) {
            Node* neighbor = edge.first; // Соседний узел
            double weight = edge.second; // Вес ребра
            if (visited[neighbor]) {
                continue; // Пропускаем посещенные узлы
            }

            double tentative_gScore = gScore[currentNode] + weight; // Рассчитываем временное расстояние

            if (gScore.find(neighbor) == gScore.end()) {
                gScore[neighbor] = numeric_limits<double>::infinity(); // Начальное расстояние для соседа
            }

            // Если новый путь короче, чем ранее известный
            if (tentative_gScore < gScore[neighbor]) {
                cameFrom[neighbor] = currentNode; // Запоминаем предшественника
                gScore[neighbor] = tentative_gScore; // Обновляем gScore
                fScore[neighbor] = tentative_gScore + heuristic(neighbor, goal); // Обновляем fScore
                openSet.emplace(fScore[neighbor], neighbor); // Добавление соседа в очередь
            }
        }
    }
    return {}; // Возврат пустого пути, если не нашли цель
}

// Метод для вычисления общего веса пути
double computePathWeight(const vector<Node*>& path) {
    double total = 0.0; // Общая сумма весов
    for (size_t i = 0; i + 1 < path.size(); i++) {
        Node* cur = path[i]; // Текущий узел
        Node* nxt = path[i + 1]; // Следующий узел
        for (auto& e : cur->edges) { // Поиск веса ребра между узлами
            if (e.first == nxt) {
                total += e.second; // Суммирование веса
                break;
            }
        }
    }
    return total; // Возврат общего веса пути
}

// Основная функция
int main() {
    setlocale(LC_ALL, "Russian"); // Установка локали на русский

    Graph graph; // Создание объекта графа
    graph.readFromFile("spb_graph.txt"); // Чтение графа из файла

    // Задание координат начала и конца пути
    double startLat = 59.885286;
    double startLon = 30.368213;
    double endLat = 59.957238;
    double endLon = 30.308108;

    // Поиск ближайших узлов к заданным координатам
    Node* startNode = graph.findClosestNode(startLat, startLon);
    Node* endNode = graph.findClosestNode(endLat, endLon);

    // Проверка на наличие найденных узлов
    if (!startNode || !endNode) {
        cerr << "could not found knots for start.\n";
        return 1;
    }

    // Поиск путей с использованием различных алгоритмов
    {
        auto t1 = chrono::high_resolution_clock::now();
        vector<Node*> pathBFS = BFS(startNode, endNode); // Поиск с использованием BFS
        auto t2 = chrono::high_resolution_clock::now();

        cout << "\n[BFS] distance:\n";
        graph.printPath(pathBFS); // Печать пути

        auto duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count(); // Время BFS
        cout << "time BFS: " << duration << " ms\n";
    }

    {
        auto t1 = chrono::high_resolution_clock::now();
        vector<Node*> pathDFS = DFS(startNode, endNode); // Поиск с использованием DFS
        auto t2 = chrono::high_resolution_clock::now();

        cout << "\n[DFS] distance:\n";
        graph.printPath(pathDFS); // Печать пути

        auto duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count(); // Время DFS
        cout << "time DFS: " << duration << " ms\n";
    }

    {
        auto t1 = chrono::high_resolution_clock::now();
        vector<Node*> pathDij = Dijkstra(graph, startNode, endNode); // Поиск с использованием алгоритма Дейкстры
        auto t2 = chrono::high_resolution_clock::now();

        cout << "\n[Dijkstra] distance:\n";
        graph.printPath(pathDij); // Печать пути

        auto duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count(); // Время Дейкстры
        cout << "time Dijkstra: " << duration << " ms\n";
    }

    {
        auto t1 = chrono::high_resolution_clock::now();
        vector<Node*> pathAStar = AStar(graph, startNode, endNode); // Поиск с использованием алгоритма A*
        auto t2 = chrono::high_resolution_clock::now();

        cout << "\n[A*] distance:\n";
        graph.printPath(pathAStar); // Печать пути

        auto duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count(); // Время A*
        cout << "time A*: " << duration << " ms\n";
    }

    return 0; // Успешное завершение программы
}