//  ROBIT C PROJECT DAY 4
//  ㄴ hw5
//  ㄴㄴ hw5.c
//
//  Created by Lee DY on 7/22/26.
//

#define MAX_LEN 256

#include <stdio.h>
#include <stdlib.h>

/* ------------ [1] 함수 원형 선언 ------------ */

int INPUT_sentence(char sentence[], int size, int* pLen);
// ㄴ문장 입력 검사 함수
//  > 1. 한 글자씩 읽어 출력 가능한 아스키 범위(' '~'~')의 문자만 인정하고
//  > 2. 줄바꿈을 만나면 문장의 입력이 끝난 것으로 본다.
//  > 3. 범위 밖의 문자, 빈 입력, 너무 긴 문장을 각각 걸러낸다.
//  > 4. 성공 시 문장을 sentence 에, 길이를 pLen 포인터에 담고 1을, 실패 시 0을 반환한다.

void PRINT_reverse(char* pSentence, int len);
// ㄴ거꾸로 출력 함수
//  > 1. 문장의 마지막 글자를 가리키는 포인터부터 시작해
//  > 2. 포인터를 한 칸씩 앞으로 옮기며 글자를 출력한다.

char FIND_maxChar(char* pSentence, int len);
// ㄴ최다 등장 문자 탐색 함수
//  > 1. 포인터로 문장을 순회하며 아스키 코드를 첨자로 써서 문자별 등장 횟수를 센다.
//  > 2. 가장 많이 등장한 문자를 찾아 반환한다. (공백은 제외, 횟수가 같으면 먼저 나온 문자)

/* ---------- [2] main문 ------------ */
/*

 > 1. 문장 입력 -> 거꾸로 출력 -> 최다 등장 문자 출력 순으로 진행하며
 > 2. 입력 단계에서 예외가 발생하면(반환값 0) 그 즉시 종료한다.

*/
int main(void)
{
    char sentence[MAX_LEN];
    int len;

    printf("입력: ");
    if (INPUT_sentence(sentence, MAX_LEN, &len) == 0)
        return 0;
    // ㄴ문장 입력이 잘못되면 종료한다.

    printf("출력: ");
    PRINT_reverse(sentence, len);

    printf("최다등장문자: %c\n", FIND_maxChar(sentence, len));

    return 0;
}

/* ---------- [1] 문장 입력 검사 함수 ------------ */
/*

 > 1. scanf("%c") 로 글자를 하나씩 받아 줄바꿈이 나올 때까지 반복한다.
 > 2. 공백을 포함해 출력 가능한 아스키 범위(' '(32) ~ '~'(126))의 문자만 배열에 저장한다.
 > 3. '\0' 자리를 남기기 위해 size - 1 글자를 넘으면 안내 후 0을 반환한다.
 > 4. 범위 밖의 문자(제어 문자 등)가 들어오면 안내 후 0을 반환한다.
 > 5. 공백을 뺀 글자가 하나도 없으면 빈 입력이므로 안내 후 0을 반환한다.
 > 6. 통과하면 끝에 '\0' 을 붙이고, 길이를 pLen 에 담은 채 1을 반환한다.

*/
int INPUT_sentence(char sentence[], int size, int* pLen)
{
    char ip;
    int len = 0;
    int letterCount = 0;   // 공백을 뺀 글자 수

    while (1)
    {
        if (scanf("%c", &ip) != 1)   // 더 읽을 입력이 없으면 문장 종료로 처리
            break;

        if (ip == '\n')   // 줄바꿈이면 문장 입력 종료
            break;

        if (ip >= ' ' && ip <= '~')   // 출력 가능한 아스키 범위면 문장의 글자로 인식
        {
            if (len >= size - 1)   // '\0' 자리가 없을 만큼 긴 문장
            {
                printf("문장이 너무 깁니다.\n");
                return 0;
            }
            sentence[len] = ip;
            len++;

            if (ip != ' ')   // 공백이 아닌 글자만 따로 센다
                letterCount++;
        }
        else
        {
            printf("잘못된 입력입니다.\n");
            return 0;
        }
    }

    sentence[len] = '\0';   // 문자열의 끝 표시

    if (letterCount == 0)   // 공백뿐이거나 아무것도 입력하지 않음
    {
        printf("입력이 없습니다.\n");
        return 0;
    }

    *pLen = len;
    return 1;
    // ㄴ검사를 통과한 문장과 길이 전달
}

/* ---------- [2] 거꾸로 출력 함수 ------------ */
/*

 > 1. 포인터 p 를 문장의 마지막 글자(pSentence + len - 1)에 놓는다.
 > 2. p 가 문장의 시작(pSentence) 앞으로 갈 때까지
 > 3. p 가 가리키는 글자를 출력하고 p 를 한 칸 앞으로 옮긴다.

*/
void PRINT_reverse(char* pSentence, int len)
{
    for (char* p = pSentence + len - 1; p >= pSentence; p--)
    {
        printf("%c", *p);   // 뒤에서부터 한 글자씩 출력
    }
    printf("\n");
}

/* ---------- [3] 최다 등장 문자 탐색 함수 ------------ */
/*

 > string.h 없이 등장 횟수를 세기 위해 아스키 코드를 첨자로 쓰는 배열을 만든다.
 > 1. counts[128] 을 0으로 초기화하고, 포인터로 문장을 순회하며
 > 2. 글자의 아스키 코드를 첨자로 써서 counts[(int)*p] 를 1씩 늘린다. (공백은 제외)
 > 3. 다시 문장을 앞에서부터 순회하며 등장 횟수가 지금까지의 최대보다 큰 글자를 기록한다.
 > 4. 큰 경우에만 갱신하므로 횟수가 같으면 먼저 나온 글자가 유지된다.

*/
char FIND_maxChar(char* pSentence, int len)
{
    int counts[128] = { 0 };   // 아스키 코드별 등장 횟수

    for (char* p = pSentence; p < pSentence + len; p++)
    {
        if (*p != ' ')   // 공백은 등장 횟수에서 제외
            counts[(int)*p] = counts[(int)*p] + 1;
    }

    int maxCount = 0;
    char maxChar = 0;

    for (char* p = pSentence; p < pSentence + len; p++)
    {
        if (*p != ' ' && counts[(int)*p] > maxCount)   // 최대 횟수 갱신
        {
            maxCount = counts[(int)*p];
            maxChar = *p;
        }
    }

    return maxChar;
    // ㄴ가장 많이 등장한 문자 반환
}
