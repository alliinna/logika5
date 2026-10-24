#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

typedef struct {
    int** data;
    int rows;
    int cols;
} Matrix;

// Создание матрицы
Matrix createMatrix(int rows, int cols) {
    Matrix m;
    m.rows = rows;
    m.cols = cols;
    m.data = (int**)malloc(rows * sizeof(int*));
    if (m.data == NULL) {
        printf("Ошибка выделения памяти\n");
        exit(1);
    }
    for (int i = 0; i < rows; ++i) {
        m.data[i] = (int*)calloc(cols, sizeof(int));
        if (m.data[i] == NULL) {
            printf("Ошибка выделения памяти\n");
            exit(1);
        }
    }
    return m;
}

// Освобождение памяти
void freeMatrix(Matrix* m) {
    if (m->data == NULL) return;
    for (int i = 0; i < m->rows; ++i) {
        free(m->data[i]);
    }
    free(m->data);
    m->data = NULL;
    m->rows = 0;
    m->cols = 0;
}

// Генерация матрицы смежности ОРИЕНТИРОВАННОГО графа (с петлями)
Matrix generateAdjacencyMatrix(int n, double probability) {
    Matrix matrix = createMatrix(n, n);

    // Рассматриваем все пары (i, j), включая i == j (петли)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            double r = (double)rand() / RAND_MAX;
            if (r < probability) {
                matrix.data[i][j] = 1; /* дуга i -> j (при i==j — петля) */
            }
        }
    }
    return matrix;
}

// Вывод матрицы
void printMatrix(const Matrix* matrix, int cols_to_print) {
    if (matrix->rows == 0) return;

    int cols = (cols_to_print == -1) ? matrix->cols : cols_to_print;

    for (int i = 0; i < matrix->rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%4d ", matrix->data[i][j]);
        }
        printf("\n");
    }
}

// Вспомогательная функция вывода списка вершин
void printVertices(const char* label, const int* vertices, int count) {
    printf("%s", label);
    if (count == 0) {
        printf("Нет");
    }
    else {
        for (int i = 0; i < count; ++i) {
            printf("%d ", vertices[i]);
        }
    }
    printf("\n");
}

// Анализ ОРИЕНТИРОВАННОГО графа по матрице смежности (с учётом петель)
void analyzeAdjacencyMatrix(const Matrix* matrix) {
    printf("\n   Анализ по матрице смежности (ориентированный граф)   \n");
    int n = matrix->rows;
    int arcs_count = 0;

    int* isolated =(int*)malloc(n * sizeof(int));
    int* end_vertices =(int*)malloc(n * sizeof(int));
    int* dominating =(int*)malloc(n * sizeof(int));
    int iso_count =0, end_count =0, dom_count =0;

    for (int i = 0; i < n; ++i) {
        int out_degree =0;    /* полустепень исхода (с петлями) */
        int in_degree =0;     /* полустепень захода (с петлями) */
        int out_to_others =0; /* исходящие дуги в другие вершины (без петель) */

        for (int j = 0; j < n; ++j) {
            if (matrix->data[i][j] == 1) {
                out_degree++;
                arcs_count++;
                if (i != j) out_to_others++; // петли не считаем
            }
            if (matrix->data[j][i] == 1) {
                in_degree++;
            }
        }

        int total_degree = out_degree + in_degree;

        if (total_degree ==0) {
            isolated[iso_count++] =i + 1;
        }
        else if (total_degree ==1) {
            end_vertices[end_count++] =i + 1;
        }

        if (out_to_others ==n - 1) {
            dominating[dom_count++] =i + 1;
        }
    }

    printf("Размер графа (количество дуг, включая петли): %d\n", arcs_count);

    printVertices("Изолированные вершины: ", isolated, iso_count);
    printVertices("Концевые вершины: ", end_vertices, end_count);
    printVertices("Доминирующие вершины: ", dominating, dom_count);

    free(isolated);
    free(end_vertices);
    free(dominating);
}

// Построение матрицы инцидентности для ОРИЕНТИР графа (с петлями)
// Петля в столбце имеет +1 
Matrix buildIncidenceMatrix(const Matrix* adjMatrix) {
    int n = adjMatrix->rows;

    // Собираем список дуг (включая петли)
    int max_arcs = n * n;
    int* arc_u =(int*)malloc(max_arcs * sizeof(int));
    int* arc_v =(int*)malloc(max_arcs * sizeof(int));
    int m = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (adjMatrix->data[i][j] == 1) {
                arc_u[m] =i;
                arc_v[m] =j;
                m++;
            }
        }
    }
    // Строки = вершины, столбцы = дуги.
    Matrix incMatrix = createMatrix(n, m);

    for (int e = 0; e < m; ++e) {
        if (arc_u[e] == arc_v[e]) {
            // Петля ставим +1
            incMatrix.data[arc_u[e]][e] =1;
        }
        else {
            // Начало дуги: +1, конец: -1
            incMatrix.data[arc_u[e]][e] =1;
            incMatrix.data[arc_v[e]][e] =-1;
        }
    }

    free(arc_u);
    free(arc_v);

    return incMatrix;
}

// Анализ ОРИЕНТИР. графа по матрице инцидентности (с учётом петель)
void analyzeIncidenceMatrix(const Matrix* incMatrix) {
    printf("\n   Анализ по матрице инцидентности (ориентированный граф)   \n");
    if (incMatrix->rows == 0) return;

    int n = incMatrix->rows;
    int m = incMatrix->cols;

    printf("Размер графа (количество дуг, включая петли): %d\n", m);

    int* isolated =(int*)malloc(n * sizeof(int));
    int* end_vertices =(int*)malloc(n * sizeof(int));
    int* dominating =(int*)malloc(n * sizeof(int));
    int iso_count =0, end_count =0, dom_count =0;

    for (int i = 0; i < n; ++i) {
        int out_degree =0;
        int in_degree =0;
        int out_to_others =0;

        for (int j = 0; j < m; ++j) {
            // Считаем общее количество +1 и -1 для степени
            if (incMatrix->data[i][j] ==1)  out_degree++;
            if (incMatrix->data[i][j] ==-1) in_degree++;

            // Определяем, является ли дуга петлёй: если в столбце нет -1,то это петля (так как в buildIncidenceMatrix для петли ставится только +1)
            if (incMatrix->data[i][j] ==1) {
                int is_loop =1;
                for (int k = 0; k < n; ++k) {
                    if (incMatrix->data[k][j] == -1) { is_loop = 0; break; }
                }
                if (!is_loop) out_to_others++;
            }
        }

        int total_degree =out_degree + in_degree;

        if (total_degree ==0) {
            isolated[iso_count++] =i + 1;
        }
        else if (total_degree ==1 ) {
            end_vertices[end_count++] =i + 1;
        }

        // Доминирующая: есть дуги во ВСЕ остальные n-1 вершин (петли не в счёт)
        if (out_to_others ==n - 1) {
            dominating[dom_count++] =i + 1;
        }
    }

    printVertices("Изолированные вершины: ", isolated, iso_count);
    printVertices("Концевые вершины: ", end_vertices, end_count);
    printVertices("Доминирующие вершины: ", dominating, dom_count);

    free(isolated);
    free(end_vertices);
    free(dominating);
}

int main(void) {
    setlocale(LC_ALL, "");

    srand((unsigned int)time(NULL));

    int n = 0;
    double probability = 0.0;

    // Ввод размера графа
    printf("Введите количество вершин графа (n > 0): ");
    while (scanf("%d", &n) != 1 || n <= 0) {
        printf("Некорректный ввод. Введите целое число > 0: ");
        while (getchar() != '\n');
    }

    // Ввод вероятности
    printf("Введите вероятность появления дуги/петли (от 0.0 до 1.0): ");
    while (scanf("%lf", &probability) != 1 || probability < 0.0 || probability > 1.0) {
        printf("Некорректный ввод. Введите число от 0.0 до 1.0: ");
        while (getchar() != '\n');
    }

    printf("\nГенерация ОРИЕНТИРОВАННОГО графа на %d вершин (с петлями)...\n", n);

    // ЗАДАНИЕ 1
    Matrix adjMatrix = generateAdjacencyMatrix(n, probability);

    printf("\nМатрица смежности G (ориентированный граф, с петлями):\n");
    printMatrix(&adjMatrix, -1);

    analyzeAdjacencyMatrix(&adjMatrix);

    // ЗАДАНИЕ 2
    printf("\n========================================\n");
    printf("ВЫПОЛНЕНИЕ ЗАДАНИЯ 2\n");
    printf("========================================\n");

    Matrix incMatrix = buildIncidenceMatrix(&adjMatrix);

    printf("\nМатрица инцидентности (Строки = Вершины, Столбцы = Дуги):\n");
    printMatrix(&incMatrix, -1);

    analyzeIncidenceMatrix(&incMatrix);

    freeMatrix(&adjMatrix);
    freeMatrix(&incMatrix);

    return 0;
}