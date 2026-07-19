#include <stdio.h>

int main(void) {
    char inputStr[100];   // 입력 문자열 (최대 99자)
    char subStr[100];     // 찾을 문자열
    int inLen = 0, subLen = 0, i, j;
    char ch;

    // ---- 입력 문자열 읽기 (공백/엔터 전까지 한 글자씩) ----
    printf("최대 99개 문자 입력 <inputStr> : ");
    scanf(" %c", &ch);
    while (ch != ' ' && ch != '\n' && ch != '\t') {
        inputStr[inLen++] = ch;
        scanf("%c", &ch);
    }

    // ---- 찾을 문자열 읽기 ----
    printf("찾는 문자열 <subStr> : ");
    scanf(" %c", &ch);
    while (ch != ' ' && ch != '\n' && ch != '\t') {
        subStr[subLen++] = ch;
        scanf("%c", &ch);
    }

    // ---- "subStr의 위치 : " 출력 ----
    for (j = 0; j < subLen; j++) printf("%c", subStr[j]);
    printf("의 위치 : ");

    // 입력 문자열의 모든 시작 위치 i에서 subStr과 같은지 비교
    for (i = 0; i + subLen <= inLen; i++) {
        int match = 1;
        for (j = 0; j < subLen; j++) {
            if (inputStr[i + j] != subStr[j]) { match = 0; break; }
        }
        if (match) printf("%d ", i + 1);   // 위치는 1부터 시작
    }
    printf("\n");
    return 0;
}