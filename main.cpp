#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

struct MovingPoint
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
};

struct CellHash
{
    size_t operator()(sf::Vector2f c) const
    {
        return hash<int>()(c.x) ^ (hash<int>()(c.y));
    }
};

// Функция для нахождения ячейки сетки, в которой находится точка
sf::Vector2f getCell(sf::Vector2f point, float cellSize)
{
    return sf::Vector2f{
        static_cast<int>(floor(point.x / cellSize)),
        static_cast<int>(floor(point.y / cellSize))};
}

// Функция для нахождения расстояния между двумя точками
float distance(sf::Vector2f a, sf::Vector2f b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

// Нахождение K ближайших соседей
vector<sf::Vector2f> findKNearest(sf::Vector2f point, unordered_map<sf::Vector2f, vector<sf::Vector2f>, CellHash> &grid, int k, float cellSize)
{
    vector<sf::Vector2f> neighbors;
    sf::Vector2f cell = getCell(point, cellSize);

    // Перебираем соседние ячейки

    for (int s = 1; s <= sqrt(grid.size()); s++)
    {
        for (int dx = -s; dx <= s; ++dx)
        {
            for (int dy = -s; dy <= s; ++dy)
            {
                sf::Vector2f neighborCell = {cell.x + dx, cell.y + dy};
                if (grid.find(neighborCell) != grid.end())
                {
                    for (sf::Vector2f neighbor : grid.find(neighborCell)->second)
                    {
                        if (neighbor != point)
                        {
                            neighbors.push_back(neighbor);
                        }
                    }
                }
            }
        }
        if (neighbors.size() >= k)
            break;
        neighbors.clear();
    }

    // Сортируем и выбираем K ближайших
    sort(neighbors.begin(), neighbors.end(), [&](sf::Vector2f a, sf::Vector2f b)
         { return distance(a, point) < distance(b, point); });

    if (neighbors.size() > k)
    {
        neighbors.resize(k);
    }

    return neighbors;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Spatial Hashing Visualization");

    int numPoints = 100;
    float cellSize = 50;
    int k = 3;
    float maxSpeed = 100; // Максимальная скорость точек (пикселей в секунду)

    vector<MovingPoint> points(numPoints);
    unordered_map<sf::Vector2f, vector<sf::Vector2f>, CellHash> grid;

    srand(static_cast<unsigned int>(time(0)));
    for (MovingPoint &point : points)
    {
        point.position.x = static_cast<float>(rand() % window.getSize().x);
        point.position.y = static_cast<float>(rand() % window.getSize().y);
        point.velocity.x = (rand() / static_cast<float>(RAND_MAX) * 2 - 1) * maxSpeed;
        point.velocity.y = (rand() / static_cast<float>(RAND_MAX) * 2 - 1) * maxSpeed;
        point.color = sf::Color(rand() % 256, rand() % 256, rand() % 256);
    }

    sf::Clock clock;

    window.setFramerateLimit(60);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();

        // Обновление позиций и сетки
        grid.clear();
        for (MovingPoint &point : points)
        {
            // Обновление положения с учетом времени
            point.position += point.velocity * dt;


            // Проверка и отражение от границ
            if (point.position.x < 0 || point.position.x > window.getSize().x)
            {
                point.velocity.x = -point.velocity.x;
                point.position.x = clamp(point.position.x, 0.f, static_cast<float>(window.getSize().x));
            }
            if (point.position.y < 0 || point.position.y > window.getSize().y)
            {
                point.velocity.y = -point.velocity.y;
                point.position.y = clamp(point.position.y, 0.f, static_cast<float>(window.getSize().y));
            }

            // Помещаем точку в сетку
            grid[getCell(point.position, cellSize)].push_back(point.position);
        }

        window.clear();

        // Отрисовка сетки
        for (float x = 0; x < window.getSize().x; x += cellSize)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, 0), sf::Color(50, 50, 50)),
                sf::Vertex(sf::Vector2f(x, window.getSize().y), sf::Color(50, 50, 50))};
            window.draw(line, 2, sf::Lines);
        }
        for (float y = 0; y < window.getSize().y; y += cellSize)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(0, y), sf::Color(50, 50, 50)),
                sf::Vertex(sf::Vector2f(window.getSize().x, y), sf::Color(50, 50, 50))};
            window.draw(line, 2, sf::Lines);
        }

        // Отрисовка точек и их связей
        for (MovingPoint point : points)
        {
            sf::CircleShape circle(5);
            circle.setFillColor(point.color);
            circle.setPosition(point.position.x - circle.getRadius(), point.position.y - circle.getRadius());
            window.draw(circle);

            // Поиск и отрисовка K ближайших соседей
            vector<sf::Vector2f> neighbors = findKNearest(point.position, grid, k, cellSize);
            for (sf::Vector2f neighbor : neighbors)
            {
                sf::Vertex line[] = {
                    sf::Vertex(point.position, point.color),
                    sf::Vertex(neighbor, point.color)};
                window.draw(line, 2, sf::Lines);
            }
        }

        window.display();
    }

    return 0;
}
