#pragma comment(lib, "bdd_debug.lib")

#include "bdd.h"
#include <fstream>
#include <math.h>

const unsigned N = 9;      // число объектов
const unsigned SIDE = sqrt(N);  // размерность таблицы
const unsigned M = 4;      // число свойств
const unsigned LOG_N = 4;    // максимальное изменение индекса, отвечающего за бит свойства
const unsigned VAR_NUM = 144;  // количество булевых переменных
char var[VAR_NUM];          // массив булевых переменных
bdd p1[N][N];            // свойство 1 название
bdd p2[N][N];            // свойство 2 цена
bdd p3[N][N];            // свойство 3 цвет
bdd p4[N][N];            // свойство 4 принт

bdd p[M][N][N];

void print() {
  for (unsigned i = 0; i < N; i++) {
    std::cout << i << ": ";
    for (unsigned j = 0; j < M; j++) {
      int J = i * M * LOG_N + j * LOG_N;
      int num = 0;
      for (unsigned k = 0; k < LOG_N; k++) {
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

void fun(char *varset, int size) {
  build(varset, size, 0);
}

void fill() {
  unsigned I = 0;
  for (unsigned i = 0; i < N; i++) {
    for (unsigned j = 0; j < N; j++) {
      p[0][i][j] = bddtrue;
      for (unsigned k = 0; k < LOG_N; k++) {
        p[0][i][j] &= ((j >> k) & 1) ? bdd_ithvar(I + k) : bdd_nithvar(I + k);
      }
      p[1][i][j] = bddtrue;
      for (unsigned k = 0; k < LOG_N; k++) {
        p[1][i][j] &= ((j >> k) & 1) ? bdd_ithvar(I + LOG_N + k) : bdd_nithvar(I + LOG_N + k);
      }
      p[2][i][j] = bddtrue;
      for (unsigned k = 0; k < LOG_N; k++) {
        p[2][i][j] &= ((j >> k) & 1) ? bdd_ithvar(I + LOG_N * 2 + k) : bdd_nithvar(I + LOG_N * 2 + k);
      }
      p[3][i][j] = bddtrue;
      for (unsigned k = 0; k < LOG_N; k++) {
        p[3][i][j] &= ((j >> k) & 1) ? bdd_ithvar(I + LOG_N * 3 + k) : bdd_nithvar(I + LOG_N * 3 + k);
      }
    }
    I += LOG_N * M;
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

  //Доп для сложного уровня
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

    //Доп для сложного уровня
    solution &= !(p[0][i][6] ^ p[1][i][4]);
    solution &= !(p[0][i][5] ^ p[1][i][7]);
    solution &= !(p[3][i][8] ^ p[1][i][2]);
    solution &= !(p[2][i][7] ^ p[1][i][2]);
  }
}

// Ограничения 3-его типа
void limit3(bdd &solution) {
  int index;
  for (unsigned i = 0; i < N; i++) {
    if ((i > SIDE) && (i % SIDE != 0)) // левое-верхнее условие без склейки
    {
      index = i - SIDE - 1;
      solution &= !(p[0][index][4] ^ p[2][i][4]);
      solution &= !(p[2][index][0] ^ p[0][i][5]);
      solution &= !(p[3][index][1] ^ p[2][i][2]);
    }
    if (i >= 1 && i <= SIDE - 1) // склейка для элементов верхней строкм
    {
      index = N - SIDE + (i % SIDE - 1);
      solution &= !(p[0][index][4] ^ p[2][i][4]);
      solution &= !(p[2][index][0] ^ p[0][i][5]);
      solution &= !(p[3][index][1] ^ p[2][i][2]);
    }
    if (i < N - SIDE) // нижнее условие без склейки
    {
      index = i + SIDE;
      solution &= !(p[0][index][7] ^ p[1][i][3]);
      solution &= !(p[0][index][8] ^ p[3][i][6]);
    }
    if (i >= N - SIDE) // склейка для элементов нижней строкм
    {
      index = i % SIDE;
      solution &= !(p[0][index][7] ^ p[1][i][3]);
      solution &= !(p[0][index][8] ^ p[3][i][6]);
    }
  }
}

// Ограничения 4-ого типа
void limit4(bdd &solution) {
  int left_top_index;
  int bottom_index;
  for (unsigned i = 0; i < N; i++) {
    if ((i > SIDE) && (i % SIDE != 0) && (i < N - SIDE)) // 2 соседа уже есть
    {
      left_top_index = i - SIDE - 1;
      bottom_index = i + SIDE;
      solution &= !(p[1][left_top_index][8] ^ p[1][i][0]) | !(p[1][bottom_index][8] ^ p[1][i][0]);
      solution &= !(p[2][left_top_index][8] ^ p[3][i][1]) | !(p[2][bottom_index][8] ^ p[3][i][1]);
      solution &= !(p[0][left_top_index][2] ^ p[1][i][5]) | !(p[0][bottom_index][2] ^ p[1][i][5]);
    }
    else if (i <= SIDE) // есть только нижний сосед
    {
      bottom_index = i + SIDE;
      if (i % SIDE != 0) // склейка для элементов верхней строкм
      {
        left_top_index = N - SIDE + (i % SIDE - 1);
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
    else if ((i >= N - SIDE) && (i % SIDE != 0)) // есть только левый-верхний сосед
    {
      left_top_index = i - SIDE - 1;
      if (i % SIDE != 0) // склейка для элементов нижней строкм
      {
        bottom_index = i % SIDE;
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

// Ограничение количества разных свойств 8
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

int main() {
  bdd_init(100000000, 100000);
  bdd_setvarnum(VAR_NUM);

  bdd my_bdd = bddtrue;
  fill();
  limit1(my_bdd);
  limit2(my_bdd);
  limit3(my_bdd);
  limit4(my_bdd);
  limit5(my_bdd);
  limit6(my_bdd);

  double satcount = bdd_satcount(my_bdd);
  std::cout << "Found " << satcount << " solution(s)\n\n";
  if (satcount != 0) {
    bdd_allsat(my_bdd, fun);
  }
  bdd_done();
  return 0;
}
