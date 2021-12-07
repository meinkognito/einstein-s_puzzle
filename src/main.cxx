#include "bdd.h"
#include <iostream>
#include <string.h>
#include <fstream>
#include <cmath>

constexpr int log2(int x)
{
  return x < 3 ? 1 : 1 + log2(x - x / 2);
}
// число объектов
constexpr int N = 9;
// число свойств
constexpr int M = 4;
// логарифм по основанию 2, взятый с потолком
constexpr int LOG_N = log2(N);
// число булевых переменных
constexpr int N_VAR = N * M * LOG_N;
// корень квадратный из N
const int SQRT_N = std::sqrt(N);
// входной файл
const char* INPUT_FILE = "C:\\Users\\Admin\\CLionProjects\\BuDDy\\src\\in.txt";
char var[N_VAR];
std::ostream* out_stream;

// p-property o-object v-propVal
struct pov
{
  pov() : propNum(-1), objNum(-1), propVal(-1) {}
  //номер свойства
  int propNum;
  // номер объекта
  int objNum;
  //значение свойства
  int propVal;
};

void limit1(bdd&, bdd p[M][N][N], pov);
void limit2(bdd&, bdd p[M][N][N], pov, pov);
bdd limit3_UpLeft(bdd p[M][N][N], pov, pov);
bdd limit3_DownLeft(bdd p[M][N][N], pov, pov);
void limit4(bdd&, bdd p[M][N][N], pov, pov);
void limit5(bdd&, bdd p[M][N][N]);
void limit6(bdd&, bdd p[M][N][N]);
void print_func(char*, int);

int main()
{
  // инициализация BuDDy
  bdd_init(10000000, 1000000);
  bdd_setvarnum(N_VAR);

  // Введение функции p(prop, obj, val)
  bdd p[M][N][N];
  for (unsigned propNum = 0; propNum < M; ++propNum)
  {
    for (unsigned objNum = 0; objNum < N; ++objNum)
    {
      for (unsigned propVal = 0; propVal < N; ++propVal)
      {
        p[propNum][objNum][propVal] = bddtrue;
        for (unsigned t = 0; t < LOG_N; ++t)
        {
          const unsigned index = LOG_N * M * objNum + LOG_N * propNum + t;
          p[propNum][objNum][propVal] &= (((propVal >> t) & 1) ? bdd_ithvar(index) : bdd_nithvar(index));
        }
      }
    }
  }

  // БДД, являющаяся решением задачи
  bdd solution = bddtrue;

  // Ввод и применение ограничений
  try
  {
    std::ifstream in;
    in.open(INPUT_FILE);
    if(!in.is_open())
    {
      throw std::invalid_argument("ERROR: can`t open file.");
    }
    else
    {
      char command[3];
      while(!in.eof())
      {
        pov prop1;
        pov prop2;
        in >> command;
        if(in.bad())
        {
          throw std::invalid_argument("ERROR: bad file.");
        }
          // Ограничение 1 типа
        if(strcmp(command, "L1") == 0)
        {
          in >> prop1.propNum >> prop1.objNum >> prop1.propVal;
          if(!in.bad())
          {
            limit1(solution, p, prop1);
          }
          else
          {
            throw std::invalid_argument("ERROR: bad file.");
          }
        }
          // Ограничение 2 типа
        else if (strcmp(command, "L2") == 0)
        {
          in >> prop1.propNum >> prop1.propVal;
          in >> prop2.propNum >> prop2.propVal;
          if(!in.bad())
          {
            limit2(solution, p, prop1, prop2);
          }
          else
          {
            throw std::invalid_argument("ERROR: bad file.");
          }
        }
          // Ограничение 3 типа
        else if (strcmp(command, "L3") == 0)
        {
          char dir[5];
          in >> prop1.propNum >> prop1.propVal;
          in >> dir;
          in >> prop2.propNum >> prop2.propVal;
          if(!in.bad())
          {
            if(strcmp(dir, "UP") == 0)
            {
              solution &= limit3_UpLeft(p, prop1, prop2);
            }
            else if (strcmp(dir, "DOWN") == 0)
            {
              solution &= limit3_DownLeft(p, prop1, prop2);
            }
            else
            {
              throw std::invalid_argument("ERROR: bad file.");
            }
          }
        }
          // Ограничение 4 типа
        else if (strcmp(command, "L4") == 0)
        {
          in >> prop1.propNum >> prop1.propVal;
          in >> prop2.propNum >> prop2.propVal;
          if(!in.bad())
          {
            limit4(solution, p, prop1, prop2);
          }
          else
          {
            throw std::invalid_argument("ERROR: bad file.");
          }
        }
          // Конец ввода ограничений
        else if (strcmp(command, "EOF") == 0)
        {
          break;
        }
        else
        {
          throw std::invalid_argument("ERROR: bad file.");
        }
        if(in.get() != '\n')
        {
          throw std::invalid_argument("ERROR: bad file.");
        }
      }
    }
    limit6(solution, p);
    limit5(solution, p);
  }
  catch (std::invalid_argument& e)
  {
    std::cerr << e.what();
    return 1;
  }

  // Вывод результатов
  std::ofstream out("output.txt");
  auto satCount = bdd_satcount(solution);
  out << satCount << " solution(s):\n";
  std::cout << satCount << " solution(s):\n";
  out_stream = &out;
  if (satCount)
  {
    bdd_allsat(solution, print_func);
  }
  out_stream = &std::cout;
  if (satCount)
  {
    bdd_allsat(solution, print_func);
  }
  out.close();
  if(satCount) std::cout << "Possible solution(s) in \"output.txt\"\n";

  // Завершение работы с библиотекой BuDDy
  bdd_done();
  return 0;
}

void limit1(bdd& solution, bdd p[M][N][N], pov prop)
{
  solution  &= p[prop.propNum][prop.objNum][prop.propVal];
}

void limit2(bdd& solution, bdd p[M][N][N], pov lProp, pov rProp)
{
  for (unsigned obj = 0; obj < N; ++obj)
  {
    bdd l = p[lProp.propNum][obj][lProp.propVal];
    bdd r = p[rProp.propNum][obj][rProp.propVal];
    solution &= (!l & !r) | (l & r);
  }
}

bdd limit3_DownLeft(bdd p[M][N][N], pov lProp, pov rProp)
{
  bdd temp = bddtrue;
  for (unsigned obj = 0; obj < N; ++obj)
  {
    // Все, кроме левого столбца и последнего объекта
    if (obj % SQRT_N != 0 && obj != N - 1)
    {
      // dlObj - номер объекта снизу слева от рассматриваемого объекта
      auto dlObj = (obj == (N - SQRT_N + 1) ? 0 :
                    (obj / SQRT_N + 1) * SQRT_N + (obj % SQRT_N - 1));
      bdd l = p[lProp.propNum][obj][lProp.propVal];
      bdd r = p[rProp.propNum][dlObj][rProp.propVal];
      temp &= ((!l) & (!r)) | (l & r);
    }
  }
  return temp;
}

bdd limit3_UpLeft(bdd p[M][N][N], pov lProp, pov rProp)
{
  bdd temp = bddtrue;
  for (unsigned obj = 0; obj < N; ++obj)
  {
    // Все, кроме левого столбца и последнего объекта в первой строке
    if (obj % SQRT_N != 0 && obj != SQRT_N - 1)
    {
      // ulObj - номер объекта сверху слева от рассматриваемого объекта
      auto ulObj = (obj == 1 ? N - SQRT_N :
                    (obj / SQRT_N - 1) * SQRT_N + (obj % SQRT_N - 1));
      bdd l = p[lProp.propNum][obj][lProp.propVal];
      bdd r = p[rProp.propNum][ulObj][rProp.propVal];
      temp &= ((!l) & (!r)) | (l & r);
    }
  }
  return temp;
}

void limit4(bdd& solution, bdd p[M][N][N], pov lProp, pov rProp)
{
  solution &= (limit3_UpLeft(p, lProp, rProp)
               | limit3_DownLeft(p, lProp, rProp));
}

// Проверка, что никакое значение одного свойства не встерчается дважды
void limit5(bdd& solution, bdd p[M][N][N])
{
  for (unsigned prop = 0; prop < M; ++prop)
  {
    for (unsigned obj1 = 0; obj1 < N - 1; ++obj1)
    {
      for (unsigned obj2 = obj1 + 1; obj2 < N; ++obj2)
      {
        for (unsigned val = 0; val < N; ++val)
        {
          solution &= (!p[prop][obj1][val] | !p[prop][obj2][val]);
        }
      }
    }
  }
}

// Проверка, что все propVal меньше N
void limit6(bdd& solution, bdd p[M][N][N])
{
  for (unsigned obj = 0; obj < N; ++obj)
  {
    bdd temp = bddtrue;
    for (unsigned prop = 0; prop < M; ++prop)
    {
      bdd tempSecond = bddfalse;
      for (unsigned val = 0; val < N; ++val)
      {
        tempSecond |= p[prop][obj][val];
      }
      temp &= tempSecond;
    }
    solution &= temp;
  }
}

void print()
{
  for (unsigned i = 0; i < N; ++i)
  {
    (*out_stream) << i << ": ";
    for (unsigned j = 0; j < M; ++j)
    {
      unsigned J = i * M * LOG_N + j * LOG_N;
      unsigned num = 0;
      for (unsigned k = 0; k < LOG_N; ++k)
      {
        num += (unsigned)(var[J + k] << k);
      }
      (*out_stream) << num << ' ';
    }
    (*out_stream) << '\n';
  }
  (*out_stream) << '\n';
}

void build(char* varset, unsigned n, unsigned I)
{
  if (I == n - 1)
  {
    if (varset[I] >= 0)
    {
      var[I] = varset[I];
      print();
      return;
    }
    var[I] = 0;
    print();
    var[I] = 1;
    print();
    return;
  }
  if (varset[I] >= 0)
  {
    var[I] = varset[I];
    build(varset, n, I + 1);
    return;
  }
  var[I] = 0;
  build(varset, n, I + 1);
  var[I] = 1;
  build(varset, n, I + 1);
}

void print_func(char* varset, int size)
{
  build(varset, size, 0);
}
