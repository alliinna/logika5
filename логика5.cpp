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

//Создание матрицы
Matrix createMatrix(int rows, int cols) {
    Matrix m;
    m.rows =rows;
    m.cols =cols;
    m.data =(int**)malloc(rows * sizeof(int*));
    if (m.data ==NULL) {
        printf("Ошибка выделения памяти\n");
        exit(1);
    }
    for (int i = 0; i < rows; ++i) {
        m.data[i] =(int*)calloc(cols, sizeof(int));
        if (m.data[i] ==NULL) {
            printf("Ошибка выделения памяти\n");
            exit(1);
        }
    }
    return m;
}

//Освобождение памяти
void freeMatrix(Matrix* m) {
    if (m->data ==NULL) return;
    for (int i = 0; i < m->rows; ++i) {
        free(m->data[i]);
    }
    free(m->data);
    m->data =NULL;
    m->rows =0;
    m->cols =0;
}

//Генерация матр смежности
Matrix generateAdjacencyMatrix(int n, double probability) {
    Matrix matrix = createMatrix(n, n);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double r=(double)rand() / RAND_MAX;
            if (r < probability) {
                matrix.data[i][j]=1;
                matrix.data[j][i]=1; /* Граф неориентир */
            }
        }
    }
    return matrix;
}

//Вывод
void printMatrix(const Matrix* matrix, int cols_to_print) {
    if (matrix->rows == 0) return;

    int cols = (cols_to_print ==-1) ? matrix->cols : cols_to_print;

    for (int i = 0; i < matrix->rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%d ", matrix->data[i][j]);
        }
        printf("\n");
    }
}

//Вспомогательная функция вывода вершин
void printVertices(const char* label, const int* vertices, int count) {
    printf("%s", label);
    if (count ==0) {
        printf("Нет");
    }
    else {
        for (int i = 0; i < count; ++i) {
            printf("%d ", vertices[i]);
        }
    }
    printf("\n");
}

//Анализ графа смеж
void analyzeAdjacencyMatrix(const Matrix* matrix) {
    printf("\n   Анализ по матрице смежности   \n");
    int n = matrix->rows;
    int edges_count =0;

    int* isolated =(int*)malloc(n * sizeof(int));
    int* end_vertices =(int*)malloc(n * sizeof(int));
    int* dominating =(int*)malloc(n * sizeof(int));
    int iso_count =0, end_count =0, dom_count=0;

    for (int i = 0; i < n; ++i) {
        int degree =0;
        for (int j = 0; j < n; ++j) {
            if (matrix->data[i][j] == 1) {
                degree++;
                edges_count++; //число 1 в строке= степень 
            }
        }

        if (degree ==0) {
            isolated[iso_count++] =i + 1;
        }
        else if (degree == 1) {
            end_vertices[end_count++] =i + 1;
        }

        if (degree == n - 1) {
            dominating[dom_count++] =i + 1;
        }
    }

    printf("Размер графа (количество ребер): %d\n", edges_count / 2);

    printVertices("Изолированные вершины: ", isolated, iso_count);
    printVertices("Концевые вершины: ", end_vertices, end_count);
    printVertices("Доминирующие вершины: ", dominating, dom_count);

    free(isolated);
    free(end_vertices);
    free(dominating);
}

//Построение матрицы инциден
Matrix buildIncidenceMatrix(const Matrix* adjMatrix) {
    int n = adjMatrix->rows;

    /* Собираем список ребер */
    int max_edges = n * (n - 1) / 2;
    int* edge_u =(int*)malloc(max_edges * sizeof(int));
    int* edge_v =(int*)malloc(max_edges * sizeof(int));
    int m = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (adjMatrix->data[i][j]==1) {
                edge_u[m] =i;
                edge_v[m] =j;
                m++;
            }
        }
    }

    Matrix incMatrix =createMatrix(n, m);

    for (int e = 0; e < m; ++e) {
        incMatrix.data[edge_u[e]][e] =1; // по две 1 с концов= кольцев
        incMatrix.data[edge_v[e]][e] =1;
    }

    free(edge_u);
    free(edge_v);

    return incMatrix;
}

//Анализ графа по матрице инцидентности
void analyzeIncidenceMatrix(const Matrix* incMatrix) {
    printf("\n   Анализ по матрице инцидентности   \n");
    if (incMatrix->rows == 0) return;

    int n = incMatrix->rows;
    int m = incMatrix->cols;

    printf("Размер графа (количество ребер): %d\n", m);

    int* isolated =(int*)malloc(n * sizeof(int));
    int* end_vertices =(int*)malloc(n * sizeof(int));
    int* dominating =(int*)malloc(n * sizeof(int));
    int iso_count =0, end_count =0, dom_count =0;

    for (int i = 0; i < n; ++i) {
        int degree =0;
        for (int j = 0; j < m; ++j) {
            if (incMatrix->data[i][j] ==1) {
                degree++;
            }
        }

        if (degree == 0) {
            isolated[iso_count++] =i + 1;
        }
        else if (degree == 1) {
            end_vertices[end_count++] =i + 1;
        }

        if (degree == n - 1) {
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

    int n = 6;                 
    double probability = 0.35;

    printf("Генерация графа на %d вершин...\n", n);

    //ЗАДАНИЕ 1
    Matrix adjMatrix = generateAdjacencyMatrix(n, probability);

    printf("\nМатрица смежности G:\n");
    printMatrix(&adjMatrix, -1);

    analyzeAdjacencyMatrix(&adjMatrix);

    //ЗАДАНИЕ 2
    printf("\n========================================\n");
    printf("ВЫПОЛНЕНИЕ ЗАДАНИЯ 2*\n");
    printf("========================================\n");

    Matrix incMatrix = buildIncidenceMatrix(&adjMatrix);

    printf("\nМатрица инцидентности (Строки = Вершины, Столбцы = Ребра):\n");
    printMatrix(&incMatrix, -1);

    analyzeIncidenceMatrix(&incMatrix);

    freeMatrix(&adjMatrix);
    freeMatrix(&incMatrix);

    return 0;
}