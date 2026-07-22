//  ROBIT C PROJECT DAY 4
//  ㄴ hw6
//  ㄴㄴ hw6.c
//
//  Created by Lee DY on 7/22/26.
//

#define ROW 3
#define COL 4

#include <stdio.h>
#include <stdlib.h>

/* ------------ [1] 함수 원형 선언 ------------ */

int INPUT_digit(int* out, int* pIsEnd);
// ㄴ한 자리 숫자 입력 검사 함수
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고
//  > 2. 두 자리 이상의 수, 숫자가 아닌 문자, 빈 입력을 걸러낸다.
//  > 3. 더 읽을 입력이 없으면(입력 끝) pIsEnd 포인터에 1을 담는다.
//  > 4. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int* REALLOC_pDigits(int* pDigits, int count);
// ㄴ동적 재할당 함수
//  > 1. 지금까지 입력된 숫자 개수(count)에 맞는 크기로 주소를 재할당하며
//  > 2. 해당 주소를 반환한다.

void PRINT_pattern(int* pDigits, int count);
// ㄴ도형 출력 함수
//  > 1. 입력된 숫자들을 처음부터 번갈아 반복하며
//  > 2. ROW x COL 크기의 도형을 채워 출력한다.

/* ---------- [2] main문 ------------ */
/*

 > 1. 한 자리 숫자를 입력받을 때마다 동적 배열을 한 칸 늘려 저장하고,
 > 2. 지금까지의 숫자들이 번갈아 나타나는 도형을 새로 출력한다.
 > 3. 입력이 끝날 때까지(pIsEnd == 1) 반복하며, 끝나면 정상 종료한다.
 > 4. 잘못된 입력이나 할당 실패 등 예외가 발생하면
 >    이미 할당한 메모리를 해제한 뒤 그 즉시 종료한다.

*/
int main(void)
{
    int* pDigits = NULL;   // 입력된 숫자들을 담는 동적 배열
    int count = 0;         // 지금까지 입력된 숫자 개수

    while (1)
    {
        printf("입력: ");

        int digit;
        int isEnd = 0;
        if (INPUT_digit(&digit, &isEnd) == 0)
        {
            free(pDigits);
            return 0;
        }
        // ㄴ입력이 끝났으면(isEnd) 정상 종료, 잘못된 입력이면 안내 후 종료
        //   (free 는 아직 아무것도 입력받지 않은 경우(NULL)에도 안전하다)

        int* pNew = REALLOC_pDigits(pDigits, count + 1);
        if (pNew == NULL)
        {
            free(pDigits);
            return 0;
        }
        // ㄴ재할당 실패 시 기존 메모리를 해제한 뒤 종료한다.

        pDigits = pNew;
        *(pDigits + count) = digit;   // 새 숫자를 배열의 끝에 저장
        count++;

        PRINT_pattern(pDigits, count);
    }
}

/* ---------- [1] 한 자리 숫자 입력 검사 함수 ------------ */
/*

 > 1. scanf("%c") 로 글자를 하나씩 받아 '0'~'9' 이면 숫자로 인정한다.
 > 2. 도형의 한 칸은 숫자 하나이므로, 숫자가 이미 있는데 또 들어오면
 >    두 자리 이상의 수로 보고 안내 후 0을 반환한다.
 > 3. 줄바꿈이나 공백을 만나면 입력이 끝난 것으로 보고 반복을 멈춘다.
 > 4. 더 읽을 입력이 없으면(scanf 실패) pIsEnd 에 1을 담고 반복을 멈춘다.
 > 5. 숫자가 아닌 문자, 빈 입력은 안내 후 0을 반환한다.
 > 6. 검사를 모두 통과하면 값을 out 에 담고 1을 반환한다.

*/
int INPUT_digit(int* out, int* pIsEnd)
{
    char ip;
    int value = -1;   // 아직 숫자를 읽지 못한 상태

    while (1)
    {
        if (scanf("%c", &ip) != 1)   // 더 읽을 입력이 없음
        {
            if (value == -1)
            {
                *pIsEnd = 1;   // 숫자 없이 입력이 끝남 -> 정상 종료 신호
                return 0;
            }
            break;   // 숫자를 읽은 뒤 입력이 끝남 -> 정상 입력으로 처리
        }

        if (ip >= '0' && ip <= '9')   // '0'~'9' 아스키 범위면 숫자로 인식
        {
            if (value != -1)   // 이미 숫자가 있는데 또 들어옴
            {
                printf("한 자리 숫자만 입력할 수 있습니다.\n");
                return 0;
            }
            value = (int)(ip - '0');   // 형변환
        }
        else if (ip == '\n' || ip == ' ')   // 줄바꿈, 띄어쓰기면 입력 종료
        {
            break;
        }
        else
        {
            printf("잘못된 입력입니다.\n");
            return 0;
        }
    }

    if (value == -1)   // 아무 숫자도 입력하지 않음
    {
        printf("입력이 없습니다.\n");
        return 0;
    }

    *out = value;
    return 1;
    // ㄴ검사를 통과한 값 전달
}

/* ---------- [2] 동적 재할당 함수 ------------ */
/*

 > 재할당 함수는 int형 배열의 주소를 반환하므로, int* 로 선언한다.
 > 1. sizeof(int) * count 크기로 realloc 하여 배열을 한 칸 늘린다.
 >    (처음에는 pDigits 가 NULL 이므로 malloc 과 동일하게 동작한다)
 > 2. 재할당에 실패(NULL)하면 안내를 출력한 뒤 NULL 을 반환하며,
 >    이때 기존 메모리는 그대로 남으므로 해제는 호출한 쪽에서 한다.
 > 3. 성공하면 재할당한 주소를 반환한다.

*/
int* REALLOC_pDigits(int* pDigits, int count)
{
    int* pNew = (int*)realloc(pDigits, sizeof(int) * count);

    if (pNew == NULL)
    {
        printf("메모리 할당에 실패했습니다.\n");
        return NULL;
    }
    // ㄴ실패: NULL 반환

    return pNew;
    // ㄴ성공: 재할당한 주소 반환
}

/* ---------- [3] 도형 출력 함수 ------------ */
/*

 > 1. 도형을 왼쪽 위부터 오른쪽 아래로 한 칸씩 채운다고 보고,
 >    k 번째 칸(k = 행 * COL + 열)에 넣을 숫자를 정한다.
 > 2. 입력된 숫자들을 처음부터 번갈아 반복해야 하므로
 >    포인터 연산 *(pDigits + k % count) 로 k 번째 칸의 숫자를 얻는다.
 > 3. 한 행(COL 칸)을 출력할 때마다 줄을 바꾼다.

*/
void PRINT_pattern(int* pDigits, int count)
{
    for (int i = 0; i < ROW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            printf("%d", *(pDigits + (i * COL + j) % count));   // 숫자들을 번갈아 채움
        }
        printf("\n");
    }
}
