#define SIZE 5
#include <stdio.h>

int hourglass[SIZE][SIZE] = {0};
void SET_hourglass(void);
void PRINT_hourglass(void);


int main(void)
{
    SET_hourglass();
    PRINT_hourglass();

    return 0;
}

void SET_hourglass(void)
{
    int value = 1;
    int row, col;
    int start = 0, end = 0;

    for (row = 0; row < SIZE; row ++)
    {
        if (row <= SIZE / 2)
        {
            start = row;
            end = SIZE - row;
        }
        
        else
        {
            start = SIZE - 1 - row;
            end = row + 1;
        }
        
        for (col = start; col < end; col ++)
        {
                hourglass[row][col] = value;
                value = value + 1;
        }
    }
}

void PRINT_hourglass(void)
{
    int row, col;

    for (row = 0; row < 5; row ++)
    {
        for (col = 0; col < 5; col++)
        {
            printf("%2d   ", hourglass[row][col]);
        }
        printf("\n");
    }
}
