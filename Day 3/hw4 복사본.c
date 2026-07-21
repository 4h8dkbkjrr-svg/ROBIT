#include <stdlib.h>
#include <stdio.h>

int  inputNatural(void);   // 자연수(1 이상) 입력용: 성공 1, 실패 0
int  inputInteger(void);   // 정수(음수·0 허용) 입력용: 성공 1, 실패 0
int  SCAN_size(void);      // 성공 1, 실패 0
int  MALLOC_pArr(void);    // 성공 1, 실패 0
int  SET_arr(void);        // 성공 1, 실패 0
void FIND_info(void);
void PRINT_info(void);
void FREE_pArr(void);

int size;
int* pArr = NULL;
int ipValue = 0;   // 입력 함수가 읽어들인 값을 저장할 전역 변수

int max, min, sum;
float avg;

int main(void)
{
    if (SCAN_size() == 0)      // 원소 개수 입력이 잘못되면 종료
        return 0;

    if (MALLOC_pArr() == 0)    // 메모리 할당 실패하면 종료
        return 0;

    if (SET_arr() == 0)        // 데이터 입력이 잘못되면
    {
        FREE_pArr();           // 이미 할당한 메모리를 정리하고 종료
        return 0;
    }

    FIND_info();
    PRINT_info();
    FREE_pArr();               // 사용이 끝난 메모리 해제

    return 0;
}

// 자연수(1 이상) 하나를 한 글자씩 읽어 검사한다.
int inputNatural(void)
{
    char ip;
    int intVal = 0;
    int count = 0;

    while (1)
    {
        scanf("%c", &ip);

        if (ip >= '0' && ip <= '9')   // '0'~'9' 아스키 범위면 숫자로 인식
        {
            intVal = intVal * 10 + (int)(ip - '0');   // 형변환
            count++;
        }
        else if (ip == '\n' || ip == ' ')   // 줄바꿈, 띄어쓰기면 입력 종료
        {
            break;
        }
        else
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

    if (count == 0)    // 아무 숫자도 입력하지 않음
    {
        printf("입력이 없습니다.\n");
        return 0;
    }
    if (intVal == 0)   // 0은 자연수가 아님
    {
        printf("0은 자연수가 아닙니다.\n");
        return 0;
    }

    ipValue = intVal;
    return 1;
}

// 정수(음수·0 허용) 하나를 한 글자씩 읽어 검사한다.
int inputInteger(void)
{
    char ip;
    int intVal = 0;
    int count = 0;
    int sign = 1;
    int isFirst = 1;   // 첫 글자인지 여부 ('-' 부호는 맨 앞에서만 허용)

    while (1)
    {
        scanf("%c", &ip);

        if (isFirst && ip == '-')   // 맨 앞의 '-'만 음수 부호로 인정
        {
            sign = -1;
            isFirst = 0;
            continue;
        }
        isFirst = 0;

        if (ip >= '0' && ip <= '9')
        {
            intVal = intVal * 10 + (int)(ip - '0');
            count++;
        }
        else if (ip == '\n' || ip == ' ')
        {
            break;
        }
        else
        {
            if (ip == '.')
                printf("실수는 입력할 수 없습니다.\n");
            else
                printf("잘못된 입력입니다.\n");
            return 0;
        }
    }

    if (count == 0)   // 숫자가 하나도 없음 (빈 입력이거나 '-'만 입력)
    {
        printf("입력이 없습니다.\n");
        return 0;
    }

    ipValue = intVal * sign;   // 정수이므로 음수·0도 그대로 저장
    return 1;
}

int SCAN_size(void)
{
    printf("몇 개의 원소를 할당하겠습니까?: ");

    if (inputNatural() == 0)   // 개수는 자연수여야 함
        return 0;

    size = ipValue;
    return 1;
}

int MALLOC_pArr(void)
{
    pArr = (int*)malloc(sizeof(int) * size);   // int 배열이므로 sizeof(int)
    if (pArr == NULL)                          // 할당 실패 검사
    {
        printf("메모리 할당에 실패했습니다.\n");
        return 0;
    }
    return 1;
}

int SET_arr(void)
{
    for (int i = 0; i < size; i++)
    {
        printf("정수형 데이터 입력: ");

        if (inputInteger() == 0)   // 데이터는 정수(음수·0 허용)
            return 0;

        pArr[i] = ipValue;
    }
    return 1;
}

void FIND_info(void)
{
    max = pArr[0];
    min = pArr[0];
    sum = 0;

    for (int i = 0; i < size; i++)
    {
        if (pArr[i] > max)
        {
            max = pArr[i];   
        }

        if (pArr[i] < min)
        {
            min = pArr[i];
        }

        sum = sum + pArr[i];
    }

    avg = (float)sum / size;   // 형변환으로 소수점까지 계산
}

void PRINT_info(void)
{
    printf("최대값: %d\n", max);
    printf("최소값: %d\n", min);
    printf("전체합: %d\n", sum);
    printf("평균: %f\n", avg);
}

void FREE_pArr(void)
{
    free(pArr);
}
