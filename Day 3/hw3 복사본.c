#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>

int N1, N2;
int **pArr = NULL;
int ipValue = 0;   // inputInt()로 입력받은 자연수를 저장할 전역 변수

int  inputInt(void);
int  INPUT_SIZE(void);   // 성공 1, 실패 0
int  MALLOC_pArr(void);  // 성공 1, 실패 0
void SET_Arr(void);
void PRINT_pArr(void);
void FREE_pArr(void);

int main(void)
{
    if (INPUT_SIZE() == 0)   // 입력이 잘못되면 여기서 종료
        return 0;

    if (MALLOC_pArr() == 0)  // 메모리 할당에 실패하면 여기서 종료
        return 0;

    SET_Arr();
    PRINT_pArr();
    FREE_pArr();

    return 0;
}

// 자연수 하나를 한 글자씩 읽어 검사한다. 성공하면 1, 잘못된 입력이면 0을 반환.
int inputInt(void)
{
    char ip;
    int intVal = 0;
    int count = 0;   // 숫자를 몇 개 읽었는지 (빈 입력 판별용)

    while (1)
    {
        scanf("%c", &ip);   // 한 글자씩 char 형으로 읽기

        if (ip >= '0' && ip <= '9')   // '0'~'9' 아스키 범위면 숫자로 인식
        {
            intVal *= 10;
            intVal += (int)(ip - '0');   // 형변환: 문자를 실제 정수 값으로
            count++;
        }
        else if (ip == '\n' || ip == ' ')   // 줄바꿈, 띄어쓰기면 입력 종료
        {
            break;
        }
        else   // 그 외의 문자는 잘못된 입력으로 판단
        {
            if (ip == '.')
                printf("실수는 입력할 수 없습니다.\n");
            else if (ip == '-')
                printf("음수는 자연수가 아닙니다.\n");
            else
                printf("잘못된 입력입니다.\n");
            return 0;
        }
    }

    if (count == 0)   // 아무 숫자도 입력하지 않은 경우 (그냥 엔터 등)
    {
        printf("입력이 없습니다.\n");
        return 0;
    }
    if (intVal == 0)   // 0은 자연수가 아님
    {
        printf("0은 자연수가 아닙니다.\n");
        return 0;
    }

    ipValue = intVal;   // 제대로 입력받은 자연수를 전역 변수에 저장
    return 1;
}

// N1, N2를 차례로 입력받는다. 하나라도 잘못되면 0을 반환.
int INPUT_SIZE(void)
{
    printf("N1: ");
    if (inputInt() == 0)
        return 0;
    N1 = ipValue;

    printf("N2: ");
    if (inputInt() == 0)
        return 0;
    N2 = ipValue;

    return 1;
}

// 2차원 배열을 동적 할당한다. 실패하면 0을 반환.
int MALLOC_pArr(void)
{
    pArr = (int**)malloc(sizeof(int*) * N1);
    if (pArr == NULL)   // 행 포인터 배열 할당 실패
    {
        printf("메모리 할당에 실패했습니다.\n");
        return 0;
    }

    for (int i = 0; i < N1; i++)
    {
        pArr[i] = (int*)malloc(sizeof(int) * N2);
        if (pArr[i] == NULL)   // i번째 행 할당 실패
        {
            printf("메모리 할당에 실패했습니다.\n");

            // 이미 할당한 행들(0 ~ i-1)과 포인터 배열을 정리
            for (int j = 0; j < i; j++)
                free(pArr[j]);
            free(pArr);

            return 0;
        }
    }

    return 1;   // 모두 성공
}

void SET_Arr(void)
{
    int value = 1;
    int top = 0;            // 아직 안 채운 맨 위 행
    int bottom = N1 - 1;    // 아직 안 채운 맨 아래 행
    int left = 0;           // 아직 안 채운 맨 왼쪽 열
    int right = N2 - 1;     // 아직 안 채운 맨 오른쪽 열

    while (top <= bottom && left <= right)
    {
        // 위쪽 행: 왼쪽 -> 오른쪽
        for (int col = left; col <= right; col ++)
        {
            pArr[top][col] = value;
            value = value + 1;
        }
        top = top + 1;

        // 오른쪽 열: 위 -> 아래
        for (int row = top; row <= bottom; row ++)
        {
            pArr[row][right] = value;
            value = value + 1;
        }
        right = right - 1;

        // 아래쪽 행: 오른쪽 -> 왼쪽 (남은 행이 있을 때만)
        if (top <= bottom)
        {
            for (int col = right; col >= left; col --)
            {
                pArr[bottom][col] = value;
                value = value + 1;
            }
            bottom = bottom - 1;
        }

        // 왼쪽 열: 아래 -> 위 (남은 열이 있을 때만)
        if (left <= right)
        {
            for (int row = bottom; row >= top; row --)
            {
                pArr[row][left] = value;
                value = value + 1;
            }
            left = left + 1;
        }
    }
}

void PRINT_pArr(void)
{
    for (int row = 0; row < N1; row ++)
    {
        for (int col = 0; col < N2; col ++)
        {
            printf("%3d ", pArr[row][col]);
        }
        printf("\n");
    }
}

void FREE_pArr(void)
{
    for (int i = 0; i < N1; i++)
    {
        free(pArr[i]);
    }
    free(pArr);
}
