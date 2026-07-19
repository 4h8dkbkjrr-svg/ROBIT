#include <stdio.h>

void INPUT_tone(void);   // 8개의 문자를 입력받는 함수
void OUTPUT_result(void); // 판정 결과를 출력하는 함수
int  SCAN_tone(void);    // 입력받은 문자가 오름차순/내림차순/뒤섞임 중 무엇인지 판별하는 함수

char tone[8];               // 사용자가 입력한 8개의 문자를 저장할 배열
char ASC[8] = "cdefgabC";   
char DES[8] = "Cbagfedc";   

int main(void)
{
    INPUT_tone();     // 음 8개 입력받기
    OUTPUT_result();  // 판정 후 결과 출력

    return 0;
}

void INPUT_tone(void)
{
    printf("음 입력\n");

    for (int i = 0; i < 8; i++)
    {
        // 공백(" ")이 스페이스, 탭, 개행문자(\n) 등을
        // 모두 건너뛰어 주기 때문에, 한 줄로 입력하든
        // 한 글자씩 Enter를 눌러 입력하든 항상 실제 문자만 정확히 읽힘
        // (공백 없이 "%c"만 쓰면 이전 입력의 개행문자가 그대로 읽혀서
        //  값이 한 칸씩 밀리는 문제가 생김)
        scanf(" %c", &tone[i]);
    }
}

void OUTPUT_result(void)
{
    // SCAN_tone()의 반환값(0/1/2)에 따라 알맞은 결과 문구 출력
    switch(SCAN_tone())
    {
        case 0:
            printf("결과: ascending");   // 8개 음이 ASC와 완전히 일치
            break;
        case 1:
            printf("결과: descending");  // 8개 음이 DES와 완전히 일치
            break;
        case 2:
            printf("결과: mixed");       // 둘 다 아닌 경우
            break;
    }
}

int SCAN_tone(void)
{
    int isAsc = 1;   
    int isDes = 1;   

    for (int i = 0; i < 8; i++)
    {
        // tone[i]가 ASC[i]와 다르면, 오름차순이 아님이 확정 -> isAsc를 0으로 내림
        if (tone[i] != ASC[i]) 
        {
            isAsc = 0;
        }

        // tone[i]가 DES[i]와 다르면, 내림차순이 아님이 확정 -> isDes를 0으로 내림
        if (tone[i] != DES[i]) 
        {
            isDes = 0;
        }
    }

    // 8글자를 모두 확인한 뒤, 최종적으로 남은 isAsc/isDes 값으로 판정
    if (isAsc == 1)          // 한 번도 안 틀렸으면(끝까지 1로 유지) 오름차순
    {
        return 0;
    }

    else if (isDes == 1)     // 오름차순은 아니지만 내림차순과는 완전히 일치
    { 
        return 1;
    }

    else                     // 오름차순도 내림차순도 아니면 뒤섞인 것
    {
        return 2;
    }
}