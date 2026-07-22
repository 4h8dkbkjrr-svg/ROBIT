//  ROBIT C PROJECT DAY 4
//  ㄴ hw2
//  ㄴㄴ hw2.c
//
//  Created by Lee DY on 7/21/26.
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct _Coordinate
{
    int x;
    int y;
} Coordinate;

/* ------------ [1] 함수 원형 선언 ------------ */

int INPUT_natural(int* out);
// ㄴ자연수 입력 검사 함수
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고
//  > 2. 실수(.), 음수(-), 문자, 빈 입력, 0을 각각 걸러낸다.
//  > 3. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int INPUT_integer(int* out);
// ㄴ정수 입력 검사 함수
//  > 1. 맨 앞의 '-' 부호를 인정해 음수, 0까지 허용하며
//  > 2. 실수(.), 문자, 빈 입력을 걸러낸다.
//  > 3. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int INPUT_num(int* out);
// ㄴ좌표 개수 입력 함수
//  > 1. 사용자로부터 좌표의 개수를 입력받고
//  > 2. 개수는 자연수여야 하므로 INPUT_natural 로 검사한다.
//  > 3. 성공 시 개수를 out 에 담고 1을, 실패 시 0을 반환한다.

Coordinate* MALLOC_pArr(int num);
// ㄴ동적 할당 함수
//  > 1. 좌표의 개수에 기반한 크기만큼 주소를 동적 할당하며
//  > 2. 해당 주소를 반환한다.

int INPUT_coordinate(Coordinate arr[], int num);
// ㄴ좌표 입력 함수
//  > 1. 사용자로부터 좌표값을 입력받고
//  > 2. 좌표는 음수도 가능하므로 INPUT_integer 로 검사한다.
//  > 3. 검사를 통과한 값만 동적 할당한 주소에 저장하며
//  > 4. 성공 시 1을, 입력이 잘못되면 0을 반환한다.

Coordinate CALCULATE(Coordinate arr[], int num, double* maxSum);
// ㄴ계산 함수
//  > 1. 동적 할당한 주소에 저장된 값들을 바탕으로
//  > 2. 표적 좌표를 한개 정한 뒤, 표적 좌표와 나머지 다른 좌표와의 거리합을 구한다.
//  > 3. 그렇게 구한 거리합들 중 가장 큰 거리합을 포인터를 이용해 반환하고,
//  > 4. return 으로 표적 좌표를 반환한다.

void PRINT_result(Coordinate farthest, double maxSum);
// ㄴ결과 출력 함수
//  > 1. 표적 좌표와 해당 표적 좌표의 거리합을 출력한다.

/* ---------- [2] main문 ------------ */
/*

 > 1. 개수 입력 -> 메모리 할당 -> 좌표 입력 -> 계산 -> 출력 순으로 진행하며
 > 2. 각 단계에서 예외가 발생하면(반환값 0 또는 NULL) 그 즉시 종료한다.
 > 3. 좌표 입력 단계에서 실패하면 이미 할당한 메모리를 먼저 해제한 뒤 종료한다.

*/
int main(void)
{
    int num;
    if (INPUT_num(&num) == 0)
        return 0;
    // ㄴ좌표 개수 입력이 잘못되면 종료한다.

    Coordinate* pCoordinate = MALLOC_pArr(num);
    if (pCoordinate == NULL)
    {
        return 0;
    }
    // ㄴ메모리 할당 실패시 종료한다.(안내 문구는 MALLOC_pArr 에서 이미 출력됨)

    if (INPUT_coordinate(pCoordinate, num) == 0)
    {
        free(pCoordinate);
        return 0;
    }
    // ㄴ좌표 입력이 잘못되면 이미 할당한 메모리를 해제한 뒤 종료한다.

    double maxSum = 0.0;
    Coordinate farthest = CALCULATE(pCoordinate, num, &maxSum);

    PRINT_result(farthest, maxSum);

    free(pCoordinate);
    // ㄴ동적 할당된 메모리 해제

    return 0;
}

/* ---------- [1-1] 자연수 입력 검사 함수 ------------ */
/*

 > 1. scanf("%c") 로 글자를 하나씩 받아 '0'~'9' 이면 숫자로 인정하고
 > 2. value = value * 10 + (ip - '0') 로 자릿수를 누적한다.
 > 3. 줄바꿈이나 공백을 만나면 한 숫자의 입력이 끝난 것으로 보고 반복을 멈춘다.
 > 4. 실수(.), 음수(-), 그 외 문자, 빈 입력, 0은 자연수가 아니므로 안내 후 0을 반환한다.
 > 5. 검사를 모두 통과하면 값을 out 에 담고 1을 반환한다.

*/
int INPUT_natural(int* out)
{
    char ip;
    int value = 0;
    int count = 0;

    while (1)
    {
        scanf("%c", &ip);

        if (ip >= '0' && ip <= '9')   // '0'~'9' 아스키 범위면 숫자로 인식
        {
            value = value * 10 + (int)(ip - '0');   // 형변환
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
    if (value == 0)    // 0은 자연수가 아님
    {
        printf("0은 자연수가 아닙니다.\n");
        return 0;
    }

    *out = value;
    return 1;
    // ㄴ검사를 통과한 값 전달
}

/* ---------- [1-2] 정수 입력 검사 함수 ------------ */
/*

 > 1. 맨 앞 글자가 '-' 이면 부호(sign)를 음수로 기록하고 다음 글자로 넘어간다.
 > 2. '0'~'9' 이면 숫자로 인정해 자릿수를 누적하고,
 > 3. 줄바꿈이나 공백을 만나면 한 숫자의 입력이 끝난 것으로 보고 반복을 멈춘다.
 > 4. 실수(.), 그 외 문자, 빈 입력은 정수가 아니므로 안내 후 0을 반환한다.
 > 5. 통과하면 value * sign 을(음수, 0 포함) out 에 담고 1을 반환한다.

*/
int INPUT_integer(int* out)
{
    char ip;
    int value = 0;
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
            value = value * 10 + (int)(ip - '0');
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

    if (count == 0)    // 숫자가 하나도 없음 (빈 입력이거나 '-'만 입력)
    {
        printf("입력이 없습니다.\n");
        return 0;
    }

    *out = value * sign;   // 정수이므로 음수, 0도 그대로 저장
    return 1;
}

/* ---------- [2] 좌표 개수 입력 함수 ------------ */
/*

 > 1. "입력: " 안내를 출력한 뒤
 > 2. 개수는 1개 이상이어야 하므로 INPUT_natural 로 검사한다.
 > 3. 통과하면 개수가 out 에 담긴 채 1을, 실패하면 0을 반환한다.

*/
int INPUT_num(int* out)
{
    printf("입력: ");

    if (INPUT_natural(out) == 0)   // 좌표 개수는 자연수여야 함
        return 0;

    return 1;
    // ㄴ입력한 좌표 개수 전달
}

/* ---------- [3] 동적 할당 함수 ------------ */
/*

 > 동적 할당 함수는 Coordinate형의 주소를 반환하므로,
 > Coordinate* 로 선언한다.
 > 1. sizeof(Coordinate) * num 크기만큼 malloc 으로 할당하고
 > 2. 할당에 실패(NULL)하면 안내를 출력한 뒤 NULL 을 반환하며,
 > 3. 성공하면 할당한 주소를 반환한다.

*/
Coordinate* MALLOC_pArr(int num)
{
    Coordinate* pCoordinate = (Coordinate*)malloc(sizeof(Coordinate) * num);

    if (pCoordinate == NULL)
    {
        printf("메모리 할당에 실패했습니다.\n");
        return NULL;
    }
    // ㄴ실패: NULL 반환

    return pCoordinate;
    // ㄴ성공: 할당한 주소 반환
}

/* ------------ [4] 좌표 입력 함수 ------------ */
/*

 > 1. num 번 반복하며 각 좌표의 x, y 를 INPUT_integer 로 검사한다.
 > 2. 좌표는 평면 위 점이라 음수도 정상값이므로 정수 검사기를 사용한다.
 > 3. 하나라도 잘못된 입력이면 즉시 0을 반환하고,
 > 4. 모두 정상이면 배열에 저장한 뒤 1을 반환한다.

*/
int INPUT_coordinate(Coordinate arr[], int num)
{
    for (int i = 0; i < num; i++)
    {
        if (INPUT_integer(&arr[i].x) == 0)   // x좌표: 정수(음수, 0 허용)
            return 0;
        if (INPUT_integer(&arr[i].y) == 0)   // y좌표: 정수(음수, 0 허용)
            return 0;
    }

    return 1;
    // ㄴ모든 좌표를 정상적으로 입력받음
}

/* ------------ [5] 계산 함수 ------------ */
/*

 > 1. 각 좌표 i 를 표적으로 삼아, 자기 자신을 뺀 나머지 좌표 j 까지의 거리를
 > 2. 피타고라스 정리 sqrt(xdiff^2 + ydiff^2) 로 구해 sum 에 모두 더한다.
 > 3. 지금까지의 최대 거리합보다 크면 maxDistance, maxIndex 를 갱신한다.
 > 4. 최대 거리합은 maxSum 포인터로, 그 표적 좌표는 return 으로 돌려준다.

*/
Coordinate CALCULATE(Coordinate arr[], int num, double* maxSum)
{
    int maxIndex = 0;
    double maxDistance = 0.0;

    for (int i = 0; i < num; i++)
    {
        double sum = 0.0;

        for (int j = 0; j < num; j++)
        {
            if (i == j)
                continue;   // 자기 자신은 제외

            int xdiff = arr[i].x - arr[j].x;
            int ydiff = arr[i].y - arr[j].y;

            sum  = sum + sqrt((double)(xdiff * xdiff + ydiff * ydiff));
        }

        // 첫 좌표이거나 지금까지의 최대보다 크면 갱신
        if (i == 0 || sum > maxDistance)
        {
            maxDistance = sum;
            maxIndex = i;
        }
    }

    *maxSum = maxDistance;     // 최대 거리합 값 전달
    return arr[maxIndex];      // 거리합이 최대인 좌표 반환
}

/* ------------ [6] 결과 출력 함수 ------------ */
/*

 > 1. 거리합이 가장 큰 좌표와 그 거리 총합을 소수 첫째 자리까지 출력한다.

*/
void PRINT_result(Coordinate farthest, double maxSum)
{
    printf("가장 거리가 먼 좌표는 (%d, %d)이며, 다른 좌표의 거리 총합은 약 %.1f입니다.\n",
           farthest.x, farthest.y, maxSum);
}
