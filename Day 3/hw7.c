#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>

char *input = NULL;      // 입력 문자열 전체 (동적 할당)
int inputLen = 0;        // 입력 문자열의 길이

int *startStack = NULL;  // 열린 태그 이름의 '시작' 위치를 저장하는 스택 (동적 할당)
int *endStack = NULL;    // 열린 태그 이름의 '끝(다음)' 위치를 저장하는 스택
int top = 0;             // 스택에 쌓인 개수 (다음에 넣을 자리)

int  READ_input(void);   // 성공 1, 실패 0
int  INIT_stack(void);   // 성공 1, 실패 0
int  SAME_range(int a1, int a2, int b1, int b2);   // 두 구간이 같은 문자열이면 1
void PRINT_range(int start, int end);
void PRINT_indent(int depth);
int  PARSE(void);        // 성공 1, 실패(잘못된 구조) 0
void FREE_all(void);

int main(void)
{
    if (READ_input() == 0)   // 입력을 못 읽으면 종료
        return 0;

    if (INIT_stack() == 0)   // 스택 준비에 실패하면 종료
    {
        free(input);
        return 0;
    }

    PARSE();     // 태그 구조를 출력 (잘못되면 내부에서 메시지 출력)
    FREE_all();

    return 0;
}

// 한 줄 전체를 한 글자씩 읽어 동적 배열에 저장한다.
int READ_input(void)
{
    int capacity = 16;
    int len = 0;
    char ip;

    input = (char*)malloc(sizeof(char) * capacity);
    if (input == NULL)
    {
        printf("메모리 할당에 실패했습니다.\n");
        return 0;
    }

    scanf("%c", &ip);
    while (ip != '\n')
    {
        if (len + 1 >= capacity)   // 공간이 부족하면 두 배로 늘림
        {
            capacity = capacity * 2;
            input = (char*)realloc(input, sizeof(char) * capacity);
            if (input == NULL)
            {
                printf("메모리 할당에 실패했습니다.\n");
                return 0;
            }
        }
        input[len] = ip;
        len = len + 1;
        scanf("%c", &ip);
    }
    input[len] = '\0';
    inputLen = len;
    return 1;
}

// 스택을 동적 할당한다. 태그 개수는 입력 길이를 넘을 수 없으므로 그 크기로 잡는다.
int INIT_stack(void)
{
    startStack = (int*)malloc(sizeof(int) * (inputLen + 1));
    endStack = (int*)malloc(sizeof(int) * (inputLen + 1));

    if (startStack == NULL || endStack == NULL)
    {
        printf("메모리 할당에 실패했습니다.\n");
        free(startStack);   // NULL이면 free(NULL)이라 안전
        free(endStack);
        return 0;
    }

    top = 0;
    return 1;
}

// input의 [a1,a2) 구간과 [b1,b2) 구간이 같은 문자열이면 1, 아니면 0
int SAME_range(int a1, int a2, int b1, int b2)
{
    if (a2 - a1 != b2 - b1)   // 길이가 다르면 다른 문자열
        return 0;

    for (int i = 0; i < a2 - a1; i++)
    {
        if (input[a1 + i] != input[b1 + i])
            return 0;
    }
    return 1;
}

// input의 [start,end) 구간을 그대로 출력한다.
void PRINT_range(int start, int end)
{
    for (int i = start; i < end; i++)
    {
        printf("%c", input[i]);
    }
}

// 깊이만큼 들여쓰기(한 단계에 공백 4칸)를 출력한다.
void PRINT_indent(int depth)
{
    for (int i = 0; i < depth * 4; i++)
    {
        printf(" ");
    }
}

// 입력 문자열을 훑으며 태그/텍스트를 들여쓰기해 출력한다. 구조가 틀리면 0을 반환.
int PARSE(void)
{
    int depth = 0;
    int i = 0;

    while (input[i] != '\0')
    {
        if (input[i] == '<')
        {
            if (input[i + 1] == '/')   // 닫는 태그 </...>
            {
                int start = i + 2;
                int j = start;
                while (input[j] != '>' && input[j] != '\0')
                    j = j + 1;

                if (input[j] == '\0')   // '>'로 안 닫힘
                {
                    printf("잘못된 태그 구조입니다.\n");
                    return 0;
                }

                // 스택이 비었거나 맨 위 여는 태그와 이름이 다르면 잘못된 구조
                if (top == 0 || SAME_range(startStack[top - 1], endStack[top - 1], start, j) == 0)
                {
                    printf("잘못된 태그 구조입니다.\n");
                    return 0;
                }
                top = top - 1;   // 짝이 맞은 여는 태그를 스택에서 제거

                depth = depth - 1;
                PRINT_indent(depth);
                printf("</");
                PRINT_range(start, j);
                printf(">\n");

                i = j + 1;
            }
            else   // 여는 태그 <...>
            {
                int start = i + 1;
                int j = start;
                while (input[j] != '>' && input[j] != '\0')
                    j = j + 1;

                if (input[j] == '\0')   // '>'로 안 닫힘
                {
                    printf("잘못된 태그 구조입니다.\n");
                    return 0;
                }
                if (j == start)   // 이름이 빈 태그 <>
                {
                    printf("잘못된 태그 구조입니다.\n");
                    return 0;
                }

                PRINT_indent(depth);
                printf("<");
                PRINT_range(start, j);
                printf(">\n");

                // 스택에 태그 이름의 위치를 저장 (닫힐 때 비교에 사용)
                startStack[top] = start;
                endStack[top] = j;
                top = top + 1;

                depth = depth + 1;
                i = j + 1;
            }
        }
        else   // 텍스트 (다음 '<'가 나오기 전까지)
        {
            int start = i;
            int j = i;
            while (input[j] != '<' && input[j] != '\0')
                j = j + 1;

            PRINT_indent(depth);
            PRINT_range(start, j);
            printf("\n");

            i = j;
        }
    }

    if (top != 0)   // 닫히지 않은 태그가 남아 있으면 잘못된 구조
    {
        printf("잘못된 태그 구조입니다.\n");
        return 0;
    }

    return 1;
}

void FREE_all(void)
{
    free(startStack);
    free(endStack);
    free(input);
}
