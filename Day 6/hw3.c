//  ROBIT C DAY 6
//  ㄴ hw3
//  ㄴㄴ hw3.c
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
//  과제의 투명성을 보장하기 위해
//  AI 주요 디버깅 내역과, 코드 설명 주석은 기본적으로 Claude에 의해 작성되지 않았습니다.
//
//  Created by Lee DY on 7/23/26. Xcode
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

typedef struct _NODE
{
    int             data;   // 저장할 정수 값
    struct _NODE*   next;   // 다음 노드의 주소를 가리키는 포인터

}NODE;

/* ------------ [1] 함수 원형 선언 ------------ */
int INPUT_command(void);
// ㄴ명령어 문자열을 입력받아 대응되는 번호를 반환하는 함수
//  > 1. 문자열을 입력받아 정해진 명령어와 비교하고
//  > 2. 일치하는 번호(1~7)를, 없으면 0을 반환한다.

int INPUT_int(int* out);
// ㄴ정수 입력 검사 함수 (음수 허용)
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고 맨 앞의 '-'는 음수로 인정한다.
//  > 2. 성공 시 값을 out 에 담아 1을, 잘못된 입력이면 안내 후 0을 반환한다.

int str_equal(const char* a, const char* b);
// ㄴ두 문자열이 같은지 비교하는 함수

int ENQUEUE(void);
// ㄴQueue 의 rear(뒤)에 새 값을 넣는 함수
//  > 1. 정수를 입력받아 새 노드를 만들어 rear 에 연결하고 tail 을 갱신한다.
//  > 2. 입력이 잘못되면 0을 반환한다. (종료 신호)

void DEQUEUE(void);
// ㄴQueue 의 front(앞)에서 값을 꺼내 출력하고 삭제하는 함수
//  > 1. 비어 있으면 안내하고, 아니면 front 값을 출력한 뒤 그 노드를 free 한다.

void SIZE(void);
// ㄴQueue 크기를 세어 출력하는 함수

void FRONT(void);
// ㄴfront(맨 앞) 값을 삭제하지 않고 출력하는 함수

void REAR(void);
// ㄴrear(맨 뒤) 값을 삭제하지 않고 출력하는 함수

void isEMPTY(void);
// ㄴQueue 가 비었는지 판별해 출력하는 함수

void printQUEUE(void);
// ㄴQueue 의 모든 값을 front 부터 출력하는 함수

void CLEANUP(void);
// ㄴ종료 시 남은 모든 노드와 더미 head 를 해제하는 함수

// head 노드를 사용한다. front = head->next, rear = tail 이 가리키는 마지막 노드.

NODE* head = NULL;
NODE* tail = NULL;

/* ---------- [2] main문 ------------ */
/*

 > 1. head 를 할당하고 tail 을 head 로 맞춰 빈 큐로 초기화한다.
 > 2. 명령 번호를 입력받아 알맞은 함수를 반복 호출한다.
 > 3. stop 이거나 Enqueue 입력 오류면 반복을 벗어난다.
 > 4. 남은 노드를 모두 해제하고 종료한다.

*/
int main(void)
{
    head = (NODE*)malloc(sizeof(NODE));
    head -> next = NULL;
    tail = head;   // 빈 큐에서는 tail 이 head 를 가리킨다

    while (1)
    {
        int cmd = INPUT_command();

        if (cmd == 1)
        {
            if (ENQUEUE() == 0)   // 잘못된 정수 입력이면 종료
                break;
        }
        else if (cmd == 2)
            DEQUEUE();
        
        else if (cmd == 3)
            SIZE();
        
        else if (cmd == 4)
            FRONT();
        
        else if (cmd == 5)
            REAR();
        
        else if (cmd == 6)
            isEMPTY();
        
        else if (cmd == 7)
            printQUEUE();
        
        else if (cmd == 8)
            break;   // stop
        else
            
            printf("명령어가 잘못되었습니다.\n");
    }

    CLEANUP();
    printf("프로그램을 종료합니다.\n");

    return 0;
}

/* ---------- [3] 명령어 입력 판별 함수 ------------ */
/*

 > 1. 안내 문구를 출력하고 명령어 문자열을 입력받는다.
 > 2. 입력이 없으면(입력의 끝) stop 과 같은 8을 반환한다.
 > 3. 정해진 명령어에 대응되는 번호를, 없으면 0을 반환한다.

*/
int INPUT_command(void)
{
    char cmd_enq[]   = "Enqueue";     // 1
    char cmd_deq[]   = "Dequeue";     // 2
    char cmd_size[]  = "size";        // 3
    char cmd_front[] = "front";       // 4
    char cmd_rear[]  = "rear";        // 5
    char cmd_empty[] = "isEmpty";     // 6
    char cmd_print[] = "printQueue";  // 7
    char cmd_stop[]  = "stop";        // 8

    char input[20] = "";

    printf("\n명령어를 입력하세요\n");
    printf("(Enqueue, Dequeue, size, front, rear, isEmpty, printQueue, stop): ");

    scanf("%19s", input);

    if (input[0] == '\0')
        return 8;   // 입력의 끝: stop 과 동일

    if      (str_equal(input, cmd_enq)   == 1)
        return 1;
    else if (str_equal(input, cmd_deq)   == 1)
        return 2;
    else if (str_equal(input, cmd_size)  == 1)
        return 3;
    else if (str_equal(input, cmd_front) == 1)
        return 4;
    else if (str_equal(input, cmd_rear)  == 1)
        return 5;
    else if (str_equal(input, cmd_empty) == 1)
        return 6;
    else if (str_equal(input, cmd_print) == 1)
        return 7;
    else if (str_equal(input, cmd_stop)  == 1)
        return 8;
    else
        return 0;
}

/* ---------- [4] 정수 입력 검사 함수 ------------ */
/*

 > 1. 한 글자씩 읽으며 앞쪽의 공백, 개행은 건너뛴다.
 > 2. '0'~'9' 는 자릿수로 누적하고 맨 앞 '-' 는 음수로 인정한다.
 > 3. 공백이나 개행을 만나면 입력의 끝으로 본다.
 > 4. 잘못된 문자나 숫자가 없는 입력은 안내 후 0을 반환한다.

*/
int INPUT_int(int* out)
{
    char ip;
    int value    = 0;
    int sign     = 1;
    int isFirst  = 1;
    int hasDigit = 0;

    while (1)
    {
        scanf("%c", &ip);

        if (isFirst == 1 && (ip == ' ' || ip == '\n' || ip == '\t'))
            continue;

        if (ip >= '0' && ip <= '9')
        {
            value = value * 10 + (int)(ip - '0');
            hasDigit = 1;
            isFirst  = 0;
        }
        else if (isFirst == 1 && ip == '-')
        {
            sign    = -1;
            isFirst = 0;
        }
        else if (ip == ' ' || ip == '\n' || ip == '\t')
        {
            break;
        }
        else
        {
            printf("정수가 아닙니다.\n");
            return 0;
        }
    }

    if (hasDigit == 0)
    {
        printf("정수가 입력되지 않았습니다.\n");
        return 0;
    }

    *out = value * sign;
    return 1;
}

/* ---------- [5] 문자열 비교 함수 ------------ */
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

/* ---------- [6] ENQUEUE 함수 ------------ */
/*

 > 1. 안내 후 INPUT_int 로 정수를 검사받는다.
 > 2. 입력이 잘못되면 0을 반환한다.
 > 3. 새 노드를 만들어 rear(tail 뒤)에 연결하고 tail 을 갱신한다.

*/
int ENQUEUE(void)
{
    int num;

    printf("Enqueue 할 정수를 입력하세요: ");
    if (INPUT_int(&num) == 0)
        return 0;

    NODE* newNode = (NODE*)malloc(sizeof(NODE));
    newNode -> data = num;
    newNode -> next = NULL;

    tail -> next = newNode;   // 현재 마지막 노드 뒤에 연결하고
    tail = newNode;           // tail 을 새 마지막 노드로 갱신한다

    return 1;
}

/* ---------- [7] DEQUEUE 함수 ------------ */
/*

 > 1. 비어 있으면 안내하고 끝낸다.
 > 2. front(head->next) 노드를 붙잡아 값을 출력한다.
 > 3. head 가 그 다음 노드를 새 front 로 가리키게 한 뒤 free 한다.
 > 4. 큐가 비게 되면 tail 을 다시 head 로 맞춘다.

*/
void DEQUEUE(void)
{
    if (head -> next == NULL)
    {
        printf("Queue 가 비어 있습니다.\n");
        return;
    }

    NODE* frontNode = head -> next;
    printf("Dequeue: %d\n", frontNode -> data);

    head -> next = frontNode -> next;
    free(frontNode);

    if (head -> next == NULL)   // 마지막 노드를 빼서 비었으면
        tail = head;            // tail 을 다시 head 로 되돌린다
}

/* ---------- [8] SIZE 함수 ------------ */
/*

 > 1. head 다음부터 NULL 까지 순회하며 개수를 세어 출력한다.

*/
void SIZE(void)
{
    int count = 0;
    NODE* curr = head -> next;

    while (curr != NULL)
    {
        count = count + 1;
        curr = curr -> next;
    }

    printf("SIZE: %d\n", count);
}

/* ---------- [9] FRONT 함수 ------------ */
/*

 > 1. 비어 있으면 안내하고, 아니면 front 값을 출력한다.

*/
void FRONT(void)
{
    if (head -> next == NULL)
    {
        printf("Queue 가 비어 있습니다.\n");
        return;
    }

    printf("front: %d\n", head -> next -> data);
}

/* ---------- [10] REAR 함수 ------------ */
/*

 > 1. 비어 있으면 안내하고, 아니면 tail 이 가리키는 rear 값을 출력한다.

*/
void REAR(void)
{
    if (head -> next == NULL)
    {
        printf("Queue 가 비어 있습니다.\n");
        return;
    }

    printf("rear: %d\n", tail -> data);
}

/* ---------- [11] isEMPTY 함수 ------------ */
/*

 > 1. head->next 가 NULL 이면 true, 아니면 false 를 출력한다.

*/
void isEMPTY(void)
{
    if (head -> next == NULL)
        printf("isEmpty: true\n");
    else
        printf("isEmpty: false\n");
}

/* ---------- [12] printQUEUE 함수 ------------ */
/*

 > 1. 비어 있으면 안내한다.
 > 2. front 부터 rear 까지 순회하며 모든 값을 출력한다.

*/
void printQUEUE(void)
{
    if (head -> next == NULL)
    {
        printf("Queue 가 비어 있습니다.\n");
        return;
    }

    NODE* curr = head -> next;
    printf("front -> [ ");
    while (curr != NULL)
    {
        printf("%d ", curr -> data);
        curr = curr -> next;
    }
    printf("] <- rear\n");
}

/* ---------- [13] 메모리 정리 함수 ------------ */
/*

 > 1. head 부터 끝까지 순회하며 노드를 하나씩 free 한다.
 > 2. free 전에 다음 노드 주소를 먼저 저장해 둔다.

*/
void CLEANUP(void)
{
    NODE* curr = head;
    while (curr != NULL)
    {
        NODE* nextNode = curr -> next;
        free(curr);
        curr = nextNode;
    }
    head = NULL;
    tail = NULL;
}
