//
// Created by Denis Beloshitskiy on 21/11/22.
//

#include "bdd.h"
#include <iostream>
#include <cmath>

constexpr int log2(int x) {
  return x < 3 ? 1 : 1 + log2(x - x / 2);
}

constexpr int N = 9;                             // число объектов
constexpr int M = 4;                             // число свойств
constexpr int LOG_N = log2(N);                // логарифм по основанию 2
constexpr int N_VAR = N * M * LOG_N;             // число булевых переменных
char var[N_VAR];                                 // массив булевых переменных
const int DIMENSION = (int) std::sqrt(N); // размерность сетки
bdd p[M][N][N];                                  // вводим функцию p(property, object, value)

void fill();                                     // заполняем функцию p
void limit1(bdd &);                              // выставляем ограничения 1-го типа
void limit2(bdd &);                              // выставляем ограничения 2-го типа
void limit3(bdd &);                              // выставляем ограничения 3-го типа
void limit4(bdd &);                              // выставляем ограничения 4-го типа
void limit5(bdd &);                              // выставляем ограничения на уникальность значений свойства
void limit6(bdd &);                              // выставляем ограничения на значения свойства
void print_func(char *, int);                    // функция для вывода решений

int main() {
  // инициализация BuDDy
  bdd_init(10000000, 100000);
  bdd_setvarnum(N_VAR);

  // БДД, являющаяся решением задачи
  bdd solution = bddtrue;
  fill();
  limit1(solution);
  limit2(solution);
  limit3(solution);
  limit4(solution);
  limit5(solution);
  limit6(solution);

  auto satcount = (unsigned) bdd_satcount(solution);
  std::cout << "Found " << satcount << " solution(s)\n";
  if (satcount) {
    bdd_allsat(solution, print_func);
  }

  // Завершение работы с библиотекой BuDDy
  bdd_done();
  return 0;
}

void fill() {
  for (unsigned property = 0; property < M; ++property) {
    for (unsigned object = 0; object < N; ++object) {
      for (unsigned value = 0; value < N; ++value) {
        p[property][object][value] = bddtrue;
        for (unsigned t = 0; t < LOG_N; ++t) {
          const unsigned index = LOG_N * M * object + LOG_N * property + t;
          p[property][object][value] &= (((value >> t) & 1) ? bdd_ithvar((int) index) : bdd_nithvar((int) index));
        }
      }
    }
  }
}

// Ограничения 1-ого типа
void limit1(bdd &solution) {
  solution &= p[2][1][6];
  solution &= p[1][4][1];
  solution &= p[3][3][2];
  solution &= p[0][8][0];
  solution &= p[1][8][6];
  solution &= p[0][6][2];
  solution &= p[2][5][1];

  // Дополнительные ограничения для продвинутого уровня
  solution &= p[1][7][7];
  solution &= p[3][1][5];
}

// Ограничения 2-ого типа
void limit2(bdd &solution) {
  for (unsigned i = 0; i < N; i++) {
    solution &= !(p[0][i][1] ^ p[2][i][5]);
    solution &= !(p[1][i][7] ^ p[3][i][7]);
    solution &= !(p[2][i][3] ^ p[3][i][3]);
    solution &= !(p[0][i][8] ^ p[3][i][0]);

    // Дополнительные ограничения для продвинутого уровня
    solution &= !(p[0][i][6] ^ p[1][i][4]);
    solution &= !(p[0][i][5] ^ p[1][i][7]);
    solution &= !(p[3][i][8] ^ p[1][i][2]);
    solution &= !(p[2][i][7] ^ p[1][i][2]);
  }
}

// Ограничения 3-его типа
void limit3(bdd &solution) {
  unsigned index;
  for (unsigned i = 0; i < N; i++) {
    // Задаем условие слева сверху (без склейки)
    if ((i > DIMENSION) && (i % DIMENSION != 0)) {
      index = i - DIMENSION - 1;
      solution &= !(p[0][index][4] ^ p[2][i][4]);
      solution &= !(p[2][index][0] ^ p[0][i][5]);
      solution &= !(p[3][index][1] ^ p[2][i][2]);
    }
    // Склейка для элементов верхней строки
    if (i >= 1 && i <= DIMENSION - 1) {
      index = N - DIMENSION + (i % DIMENSION - 1);
      solution &= !(p[0][index][4] ^ p[2][i][4]);
      solution &= !(p[2][index][0] ^ p[0][i][5]);
      solution &= !(p[3][index][1] ^ p[2][i][2]);
    }
    // Задаем нижнее условие (без склейки)
    if (i < N - DIMENSION) {
      index = i + DIMENSION;
      solution &= !(p[0][index][7] ^ p[1][i][3]);
      solution &= !(p[0][index][8] ^ p[3][i][6]);
    }
    // Склейка для элементов нижней строки
    if (i >= N - DIMENSION) {
      index = i % DIMENSION;
      solution &= !(p[0][index][7] ^ p[1][i][3]);
      solution &= !(p[0][index][8] ^ p[3][i][6]);
    }
  }
}

// Ограничения 4-ого типа
void limit4(bdd &solution) {
  unsigned left_top_index;
  unsigned bottom_index;
  for (unsigned i = 0; i < N; i++) {
    // Случай, когда 2 соседа уже есть
    if ((i > DIMENSION) && (i % DIMENSION != 0) && (i < N - DIMENSION)) {
      left_top_index = i - DIMENSION - 1;
      bottom_index = i + DIMENSION;
      solution &= !(p[1][left_top_index][8] ^ p[1][i][0]) | !(p[1][bottom_index][8] ^ p[1][i][0]);
      solution &= !(p[2][left_top_index][8] ^ p[3][i][1]) | !(p[2][bottom_index][8] ^ p[3][i][1]);
      solution &= !(p[0][left_top_index][2] ^ p[1][i][5]) | !(p[0][bottom_index][2] ^ p[1][i][5]);
    }
      // Случай, когда есть только нижний сосед
    else if (i <= DIMENSION) {
      bottom_index = i + DIMENSION;
      // Склейка для элементов верхней строки
      if (i % DIMENSION != 0) {
        left_top_index = N - DIMENSION + (i % DIMENSION - 1);
        solution &= !(p[1][left_top_index][8] ^ p[1][i][0]) | !(p[1][bottom_index][8] ^ p[1][i][0]);
        solution &= !(p[2][left_top_index][8] ^ p[3][i][1]) | !(p[2][bottom_index][8] ^ p[3][i][1]);
        solution &= !(p[0][left_top_index][2] ^ p[1][i][5]) | !(p[0][bottom_index][2] ^ p[1][i][5]);
      }
      else {
        solution &= !(p[1][bottom_index][8] ^ p[1][i][0]);
        solution &= !(p[2][bottom_index][8] ^ p[3][i][1]);
        solution &= !(p[0][bottom_index][2] ^ p[1][i][5]);
      }
    }
      // Случай, когда есть только сосед слева сверху
    else if ((i >= N - DIMENSION) && (i % DIMENSION != 0)) {
      left_top_index = i - DIMENSION - 1;
      // Склейка для элементов нижней строки
      if (i % DIMENSION != 0) {
        bottom_index = i % DIMENSION;
        solution &= !(p[1][left_top_index][8] ^ p[1][i][0]) | !(p[1][bottom_index][8] ^ p[1][i][0]);
        solution &= !(p[2][left_top_index][8] ^ p[3][i][1]) | !(p[2][bottom_index][8] ^ p[3][i][1]);
        solution &= !(p[0][left_top_index][2] ^ p[1][i][5]) | !(p[0][bottom_index][2] ^ p[1][i][5]);
      }
      else {
        solution &= !(p[1][left_top_index][8] ^ p[1][i][0]);
        solution &= !(p[2][left_top_index][8] ^ p[3][i][1]);
        solution &= !(p[0][left_top_index][2] ^ p[1][i][5]);
      }
    }
  }
}

// Проверка, что значения одного свойства уникальны
void limit5(bdd &solution) {
  for (unsigned i = 0; i < N; i++) {
    bdd bdd1, bdd2, bdd3, bdd4 = bddfalse;
    for (unsigned j = 0; j < N; j++) {
      bdd1 |= p[0][i][j];
      bdd2 |= p[1][i][j];
      bdd3 |= p[2][i][j];
      bdd4 |= p[3][i][j];
    }
    solution &= bdd1 & bdd2 & bdd3 & bdd4;
  }
}

// Проверка, что все значения свойств находятся в промежутке от 0 до N-1
void limit6(bdd &solution) {
  for (unsigned j = 0; j < N; j++) {
    for (unsigned i = 0; i < N - 1; i++) {
      for (unsigned k = i + 1; k < N; k++) {
        for (auto &m: p) {
          solution &= m[i][j] >> !m[k][j];
        }
      }
    }
  }
}

void print() {
  for (unsigned i = 0; i < N; ++i) {
    std::cout << i << ": ";
    for (unsigned j = 0; j < M; ++j) {
      unsigned J = i * M * LOG_N + j * LOG_N;
      unsigned num = 0;
      for (unsigned k = 0; k < LOG_N; ++k) {
        num += (unsigned) (var[J + k] << k);
      }
      std::cout << num << ' ';
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

void build(const char *varset, const unsigned n, const unsigned I) {
  if (I == n - 1) {
    if (varset[I] >= 0) {
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
  if (varset[I] >= 0) {
    var[I] = varset[I];
    build(varset, n, I + 1);
    return;
  }
  var[I] = 0;
  build(varset, n, I + 1);
  var[I] = 1;
  build(varset, n, I + 1);
}

void print_func(char *varset, int size) {
  build(varset, size, 0);
}
