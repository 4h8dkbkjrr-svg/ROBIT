#define N1 5
#define N2 5

#include <stdlib.h>
#include <stdio.h>

int **pArr = NULL;

void MALLOC_pArr(void);
void SET_Arr(void);
void PRINT_pArr(void);
void FREE_pArr(void);

int main(void)
{
    MALLOC_pArr();
    SET_Arr();
    PRINT_pArr();
    FREE_pArr();

    return 0;
}

void MALLOC_pArr(void)
{
    pArr = (int**)malloc(sizeof(int*) * N1);
    for (int i = 0; i < N1; i++)
    {
        pArr[i] = (int*)malloc(sizeof(int) * N2);
    }
}

void SET_Arr(void)
{
    int value = 1;

    for (int d = 0; d < N1 + N2 - 1; d++)  // d = 대각선 번호 (0~8)
    {
        int row = (d < N2) ? 0 : d - N2 + 1;  // 시작 행
        int col = (d < N2) ? d : N2 - 1;       // 시작 열

        while (row < N1 && col >= 0)
        {
            pArr[row][col] = value;
            value = value + 1;
            row = row + 1;
            col = col - 1;
        }
    }
}

void PRINT_pArr(void)
{
    for (int row = 0; row < N1; row++)
    {
        for (int col = 0; col < N2; col++)
        {
            printf("%3d", pArr[row][col]);
        }
        printf("\n");
    }
}

void FREE_pArr(void)
{
    for (int i = 0; i < N1; i++)
    {
        free(pArr[i]);   // 안쪽(각 행) 먼저 해제
    }
    free(pArr);          // 바깥쪽(포인터 배열) 나중에 해제
    pArr = NULL;         
}

/*
 
 0,0
 0,1  1,0
 0,2  1,1  2,0
 0,3  1,2  2,1  3,0
 
 */
