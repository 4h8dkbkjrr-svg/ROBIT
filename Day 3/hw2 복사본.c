#include <stdio.h>

#define N1 4
#define N2 3

int arrA[N1][N2];
int arrB[N2][N1];

void SET_arrA(void);
void SET_arrB(void);
void PRINT_arrB(void);

int main(void)
{
    SET_arrA();
    SET_arrB();
    
    PRINT_arrB();
    
    return 0;
}

void SET_arrA(void)
{
    int value = 1;
    
    for (int row = 0; row < N1; row ++)
    {
        for (int col = 0; col < N2; col ++)
        {
            arrA[row][col] = value;
            value = value + 1;
        }
    }
}

void SET_arrB(void)
{
    for (int row = 0; row < N2; row ++)
    {
        for (int col = 0; col < N1; col ++)
        {
            arrB[row][col] = arrA[col][row];
        }
    }
    
}

void PRINT_arrB(void)
{
    for (int row = 0; row < N2; row ++)
    {
        for (int col = 0; col < N1; col ++)
        {
            printf("%d  ", arrB[row][col]);
        }
        printf("\n");
    }
}

