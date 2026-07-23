//  ROBIT C DAY 6
//  ㄴ hw4
//  ㄴㄴ hw4.c
//
//  <AI 사용 안내>
//  > 1. 코드 제작 과정에서 일부 Claude가 개입했습니다.
//  > 2. Claude 사용 원칙 및 사용 과정은 폴더에 들어있는 README.md 파일을 통해 투명하게 공개됩니다.
//  > 3. Claude의 조언을 받았거나, 개입한 부분을 주석을 통해 투명하게 표기했습니다.
//  > ㄴ 3.1. Claude가 함수에 광범위하게 개입한 경우, [1] 함수 원형 선언에서 Claude가 개입했음을 표기했습니다.
//  > ㄴ 3.2. Claude가 함수에 제한적으로 개입한 경우, 각 함수에서 Claude가 개입한 줄 옆에 주석처리로 Claude가 개입했음을 표기했습니다.
//  > ㄴ 3.3. Claude가 미세하게 개입한 경우, 개입 내역이 생략될 수 있습니다.
//  > ㄴ 3.4. 이전 과제에서 Claude가 개입한 부분을 재사용하거나, 부분 수정해서 사용하는 경우, 개입 내역이 생략될 수 있습니다.
//  > ㄴ 3.5. 일부 개입 내역의 경우, 누락될 수 있습니다.
//
//  <풀이 아이디어 정리>
//  > 1. 문자를 스택(LIFO)과 큐(FIFO)에 동시에 넣으면
//  > 2. 스택에서 꺼낸 순서(뒤->앞)와 큐에서 꺼낸 순서(앞->뒤)가 나온다.
//  > 3. 두 순서를 한 글자씩 비교해 모두 같으면 회문이다.
//
//  > * 풀이 아이디어 정리 과정에서 Claude에게 궁금한 점을 질문하고, 도움을 받았습니다.
//  > * 이중 포인터 사용 과정에서 Claude에게 궁금한 점을 질문하고, 도움을 받았습니다.
//
//  Created by Lee DY on 7/23/26. Xcode
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE 200   // 한 줄에 받을 수 있는 최대 글자 수

typedef struct _NODE
{
    char            data;   // 저장할 문자
    struct _NODE*   next;   // 다음 노드의 주소

}NODE;

/* ------------ [1] 함수 원형 선언 ------------ */
int  INPUT_line(char* line);
// ㄴ한 줄을 문자 단위로 입력받아 line 에 담는 함수
//  > 1. '\n' 을 만날 때까지 한 글자씩 읽어 line 에 저장한다.
//  > 2. 글자가 하나라도 있으면 1, 아무것도 없으면 0을 반환한다.
//
//  > * INPUT_command 함수는 Claude의 도움을 받아 작성되었습니다.

int  str_equal(const char* a, const char* b);
// ㄴ두 문자열이 같은지 비교하는 함수

void PUSH_stack(NODE** top, char c);
// ㄴ스택의 맨 위에 문자를 넣는 함수 (LIFO)

char POP_stack(NODE** top);
// ㄴ스택의 맨 위 문자를 꺼내 반환하는 함수 (LIFO)

void ENQUEUE(NODE** front, NODE** rear, char c);
// ㄴ큐의 rear 에 문자를 넣는 함수 (FIFO)

char DEQUEUE(NODE** front, NODE** rear);
// ㄴ큐의 front 에서 문자를 꺼내 반환하는 함수 (FIFO)

int  IS_palindrome(const char* line);
// ㄴ한 줄이 회문인지 판별하는 함수
//  > 1. 공백을 뺀 글자들을 스택과 큐에 함께 넣는다.
//  > 2. 스택에서 꺼낸 문자와 큐에서 꺼낸 문자를 하나씩 비교한다.
//  > 3. 모두 같으면 1(회문), 하나라도 다르면 0을 반환한다.
//
//  > * IS_palindrome 함수는 Claude의 도움을 받아 작성되었습니다.


/* ---------- [2] main문 ------------ */
/*

 > 1. "stop" 이 입력될 때까지 한 줄씩 반복해서 입력받는다.
 > 2. 각 줄이 회문인지 판별해 결과를 출력한다.

*/
int main(void)
{
    char line[MAX_LINE];

    printf("문자열을 입력하면 회문인지 판별합니다. (종료: stop)\n");

    while (1)
    {
        printf("\n문자열을 입력하세요: ");

        if (INPUT_line(line) == 0)   // 빈 줄이면 다시 입력받는다
            continue;

        if (str_equal(line, "stop") == 1)
            break;

        if (IS_palindrome(line) == 1)
            printf("결과: \"%s\" 는 회문입니다.\n", line);
        else
            printf("결과: \"%s\" 는 회문이 아닙니다.\n", line);
    }

    printf("프로그램을 종료합니다.\n");
    return 0;
}

/* ---------- [3] 한 줄 입력 함수 ------------ */
/*

 > 1. '\n' 이나 입력의 끝을 만날 때까지 한 글자씩 읽어 line 에 담는다.
 > 2. 최대 길이를 넘으면 더 담지 않고 남은 글자는 흘려보낸다.
 > 3. 끝에 '\0' 을 붙이고, 담긴 글자가 있으면 1을 반환한다.

*/
int INPUT_line(char* line)
{
    char ip;
    int  len = 0;

    while (1)
    {
        scanf("%c", &ip);

        if (ip == '\n')
            break;

        if (len < MAX_LINE - 1)   // 최대 길이 전까지만 저장한다
        {
            line[len] = ip;
            len = len + 1;
        }
    }

    line[len] = '\0';

    if (len == 0)
        return 0;
    else
        return 1;
}

/* ---------- [4] 문자열 비교 함수 ------------ */
/*

 > 1. 앞에서부터 한 글자씩 비교하다 다르면 0을, 완전히 같으면 1을 반환한다.

*/
int str_equal(const char* a, const char* b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
            return 0;

        i++;
    }

    if (a[i] == b[i])
        return 1;
    else
        return 0;
}

/* ---------- [5] 스택 PUSH 함수 ------------ */
/*

 > 1. 새 노드를 만들어 현재 top 앞에 끼워 넣는다.
 > 2. top 이 새 노드를 가리키게 갱신한다. (이중 포인터로 top 자체를 바꾼다)

*/
void PUSH_stack(NODE** top, char c)
{
    NODE* newNode = (NODE*)malloc(sizeof(NODE));
    newNode -> data = c;
    newNode -> next = *top;
    *top = newNode;
}

/* ---------- [6] 스택 POP 함수 ------------ */
/*

 > 1. 현재 top 노드의 문자를 꺼낸다.
 > 2. top 을 다음 노드로 옮기고 꺼낸 노드를 free 한 뒤 문자를 반환한다.

*/
char POP_stack(NODE** top)
{
    NODE* topNode = *top;
    char  c = topNode -> data;
    *top = topNode -> next;
    free(topNode);
    return c;
}

/* ---------- [7] 큐 ENQUEUE 함수 ------------ */
/*

 > 1. 새 노드를 만들어 rear 뒤에 연결한다.
 > 2. 큐가 비어 있었다면 front 도 새 노드를 가리키게 한다.

*/
void ENQUEUE(NODE** front, NODE** rear, char c)
{
    NODE* newNode = (NODE*)malloc(sizeof(NODE));
    newNode -> data = c;
    newNode -> next = NULL;

    if (*rear == NULL)   // 빈 큐라면 front, rear 모두 새 노드
    {
        *front = newNode;
        *rear  = newNode;
    }
    else
    {
        (*rear) -> next = newNode;
        *rear = newNode;
    }
}

/* ---------- [8] 큐 DEQUEUE 함수 ------------ */
/*

 > 1. front 노드의 문자를 꺼낸다.
 > 2. front 를 다음 노드로 옮기고, 비게 되면 rear 도 NULL 로 맞춘다.
 > 3. 꺼낸 노드를 free 한 뒤 문자를 반환한다.

*/
char DEQUEUE(NODE** front, NODE** rear)
{
    NODE* frontNode = *front;
    char  c = frontNode -> data;

    *front = frontNode -> next;
    if (*front == NULL)
        *rear = NULL;

    free(frontNode);
    return c;
}

/* ---------- [9] 회문 판별 함수 ------------ */
/*

 > 1. 공백을 제외한 글자를 스택과 큐에 동시에 넣는다. (대문자는 소문자로 변환)
 > 2. 스택에서 꺼낸 문자(뒤->앞)와 큐에서 꺼낸 문자(앞->뒤)를 하나씩 비교한다.
 > 3. 하나라도 다르면 0, 끝까지 같으면 1을 반환한다.
 > 4. 판별이 끝나면 남은 노드를 모두 정리한다.

*/
int IS_palindrome(const char* line)
{
    NODE* stackTop  = NULL;   // 스택의 top
    NODE* queFront  = NULL;   // 큐의 front
    NODE* queRear   = NULL;   // 큐의 rear
    int   i = 0;
    int   result = 1;

    // 1) 글자를 스택과 큐에 함께 넣는다.
    while (line[i] != '\0')
    {
        char c = line[i];

        if (c >= 'A' && c <= 'Z')   // 대문자는 소문자로 변환
            c = c - 'A' + 'a';

        if (c != ' ')   // 공백은 무시한다
        {
            PUSH_stack(&stackTop, c);
            ENQUEUE(&queFront, &queRear, c);
        }

        i = i + 1;
    }

    // 2) 스택(뒤->앞)과 큐(앞->뒤)를 하나씩 비교한다.
    while (stackTop != NULL)
    {
        char fromStack = POP_stack(&stackTop);
        char fromQueue = DEQUEUE(&queFront, &queRear);

        if (fromStack != fromQueue)
            result = 0;
    }

    return result;
}
