#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <cmath>

using namespace std;

// Вспомогательные функции для шума Перлина
double fade(double t)
{
  return t * t * t * (t * (t * 6 - 15) + 10);
}

double my_lerp(double t, double a, double b)
{
  return a + t * (b - a);
}

double grad(int hash, double x, double y)
{
  int h = hash & 15;
  double u = h < 8 ? x : y;
  double v = h < 4 ? y : (h == 12 || h == 14 ? x : 0);
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

class PerlinNoise
{
private:
  int p[512];

  void initPermutation(unsigned seed)
  {
    srand(seed);
    for (int i = 0; i < 256; ++i)
    {
      p[i] = i;
    }
    for (int i = 0; i < 256; ++i)
    {
      int j = rand() % 256;
      swap(p[i], p[j]);
    }
    for (int i = 0; i < 256; ++i)
    {
      p[256 + i] = p[i];
    }
  }

public:
  PerlinNoise(unsigned seed = 0)
  {
    initPermutation(seed);
  }

  double noise(double x, double y)
  {
    int X = static_cast<int>(floor(x)) & 255;
    int Y = static_cast<int>(floor(y)) & 255;

    x -= floor(x);
    y -= floor(y);

    double u = fade(x);
    double v = fade(y);

    int A = p[X] + Y;
    int B = p[X + 1] + Y;

    return my_lerp(v, my_lerp(u, grad(p[A], x, y), grad(p[B], x - 1, y)),
                   my_lerp(u, grad(p[A + 1], x, y - 1), grad(p[B + 1], x - 1, y - 1)));
  }
};

class WorldGenerator
{
private:
  int width, height;
  vector<vector<string>> world;
  PerlinNoise perlin;

  double fractalNoise(double x, double y, int octaves, double persistence)
  {
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    double maxValue = 0;

    for (int i = 0; i < octaves; ++i)
    {
      total += perlin.noise(x * frequency, y * frequency) * amplitude;
      maxValue += amplitude;
      amplitude *= persistence;
      frequency *= 2;
    }

    return total / maxValue;
  }

public:
  WorldGenerator() : width(0), height(0) {}

  void clean_map()
  {
    world.clear();
  }

  void generator(int w, int h, unsigned seed = 0)
  {
    width = w;
    height = h;
    world.resize(height, vector<string>(width, " "));
    perlin = PerlinNoise(seed);
  }

  void generate()
  {
    double scale = 0.05;
    int octaves = 5;
    double persistence = 0.5;

    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        double n = fractalNoise(x * scale, y * scale, octaves, persistence);
        if (n < -0.3)
          world[y][x] = "\033[46m \033[0m"; // Вода
        else if (n < -0.1)
          world[y][x] = "\033[44m \033[0m"; // Глубокая вода
        else if (n < 0.2)
          world[y][x] = "\033[43m \033[0m"; // Равнина
        else if (n < 0.5)
          world[y][x] = "\033[42m \033[0m"; // Лес
        else if (n < 0.7)
          world[y][x] = "\033[45m \033[0m"; // Горы
        else
          world[y][x] = "\033[47m \033[0m"; // Снег
      }
    }
  }

  void display()
  {
    cout << endl;
    for (const auto &line : world)
    {
      for (const auto &c : line)
      {
        cout << c;
      }
      cout << endl;
    }
    cout << "Размер карты: " << width << "x" << height << endl;
  }

  void save_to_file(const string &filename)
  {
    ofstream file(filename);
    for (const auto &line : world)
    {
      for (const auto &c : line)
      {
        file << c;
      }
      file << "\n";
    }
    file.close();
  }

  void load_from_file(const string &filename)
  {
    ifstream file(filename);
    if (!file.is_open())
    {
      cerr << "Ошибка: не удалось открыть файл " << filename << "\n";
      return;
    }

    vector<string> lines;
    string line;
    width = 0;
    height = 0;

    while (getline(file, line))
    {
      lines.push_back(line);
      width = max(width, static_cast<int>(line.size()));
      height++;
    }

    world.resize(height, vector<string>(width, " "));
    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < lines[y].size(); ++x)
      {
        world[y][x] = string(1, lines[y][x]);
      }
    }

    file.close();
    width /= 10;
  }
};

int main()
{
  cout << "\033[46m \033[0m: Вода\n"
       << "\033[44m \033[0m: Глубокая вода\n"
       << "\033[43m \033[0m: Равнина\n"
       << "\033[42m \033[0m: Лес\n"
       << "\033[45m \033[0m: Горы\n"
       << "\033[47m \033[0m: Снег\n";

  unsigned seed = static_cast<unsigned>(time(0));
  srand(seed);

  WorldGenerator world;
  string filename = "world.txt";
  int width, height;

  cout << "Загрузка из файла..." << endl;
  world.load_from_file(filename);
  world.display();
  world.clean_map();

  cout << "\nВведите ширину и высоту мира: ";
  cin >> width >> height;

  cout << "\nГенерация мира...\n";
  world.generator(width, height, seed);
  world.generate();
  world.display();

  world.save_to_file(filename);
  cout << "Мир сохранен в файл " << filename << "\n";

  return 0;
}