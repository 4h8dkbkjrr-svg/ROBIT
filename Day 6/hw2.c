//  ROBIT C DAY 6
//  ㄴ hw2
//  ㄴㄴ hw2.c
//
//  <AI 사용 안내>
//  > 1. 코드 제작 과정에서 일부 Claude가 개입했습니다.
//  > 2. Claude 사용 원칙 및 사용 과정은 폴더에 들어있는 README.md 파일을 통해 투명하게 공개됩니다.
//  > 3. Claude의 조언을 받았거나, 개입한 부분을 주석을 통해 투명하게 표기했습니다.
//  > ㄴ 3.1. Claude가 함수에 광범위하게 개입한 경우, [1] 함수 원형 선언에서 Claude가 개입했음을 표기했습니다.
//  > ㄴ 3.2. Claude가 함수에 제한적으로 개입한 경우, 각 함수에서 Claude가 개입한 줄 옆에 주석처리로 Claude가 개입했음을 표기했습니다.
//  > ㄴ 3.3  Claude가 함수에 미세하게 개입한 경우, 개입 내역이 생략될 수 있습니다.
//  > ㄴ 3.4  이전 과제에서 Claude가 개입한 부분을 재사용하거나, 부분 수정해서 사용하는 경우, 개입 내역이 생략될 수 있습니다.
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
// ㄴ명령어 문자열을 입력받아 입력받는 명령어에 대응되는 숫자를 반환하는 함수
//  > 1. 문자열을 입력받아 정해진 명령어와 비교하고
//  > 2. 일치하는 명령과 대응되는 숫자(1~7)를, 없으면 0을 반환한다.
//  > 3. 이후 main문에서 조건식을 이용해 명령어 함수를 실행시킨다.
//
//  > * INPUT_command 함수는 Claude의 도움을 받아 작성되었습니다.

int INPUT_int(int* out);
// ㄴ정수 입력 검사 함수 (음수 허용)
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고 맨 앞의 '-'는 음수로 판별한다.
//  > 2. 성공 시 값을 out 에 담아 1을, 잘못된 입력이면 안내 후 0을 반환한다.
//
//  > * INPUT_int 함수는 Claude의 도움을 받아 작성되었습니다.

int str_equal(const char* a, const char* b);
// ㄴ두 문자열이 같은지 비교하는 함수
//  > 1. 앞에서부터 한 글자씩 비교하다 다르면 0을 반환하고
//  > 2. 둘 다 '\0'에서 끝나 완전히 같으면 1을 반환한다.
//
//  > * str_equal 함수는 Claude의 도움을 받아 작성되었습니다.

int PUSH(void);
// ㄴ정수를 입력받아 스택 맨 위에 넣는 함수
//  > 1. INPUT_int 로 정수를 검사받아 새 노드를 맨 앞에 연결하고 1을 반환한다.
//  > 2. 정수가 아닌 경우 0을 반환한다. (종료 신호)

void POP(void);
// ㄴ맨 위 값을 꺼내 출력하고 그 노드를 삭제하는 함수
//  > 1. 비어 있으면 안내하고, 아니면 top 값을 출력한 뒤 그 노드를 삭제한다. (free로 삭제 구현)

void SIZE(void);
// ㄴ스택에 든 값의 개수를 세어 출력하는 함수
//  > 1. head 다음부터 NULL 까지 순회하며 개수를 세어 출력한다. (C Day 6 30p 처럼 curr 구현)

void TOP(void);
// ㄴ맨 위 값을 삭제하지 않고 확인만 하는 함수
//  > 1. 비어 있으면 안내하고, 아니면 top 값을 출력한다.

void isEMPTY(void);
// ㄴ스택이 비었는지 판별해 출력하는 함수
//  > 1. head->next 가 NULL 이면 true, 아니면 false 를 출력한다.

void printSTACK(void);
// ㄴ스택의 모든 값을 top 부터 순서대로 출력하는 함수
//  > 1. 비어 있으면 안내하고, 아니면 top 부터 끝까지 값을 출력한다.

void CLEANUP(void);
// ㄴ종료 시 남은 모든 노드와 더미 head 를 해제하는 함수
//  > 1. head 부터 끝까지 순회하며 노드를 하나씩 free 해 메모리 누수를 막는다.
//
//  > * CLEANUP 함수의 경우 Claude의 조언에 따라 추가했고, Claude의 도움을 받아 작성되었습니다.

// head 자체에는 값을 넣지 않고, head->next 부터가 실제 스택이다.
// 빈 스택: head -> NULL, 스택의 top(맨 위) = head->next
NODE* head = NULL;

/* ---------- [2] main문 ------------ */
/*

 > 1. 더미 head 노드를 할당해 빈 스택으로 초기화한다.
 > 2. 명령 번호를 입력받아 알맞은 함수를 반복 호출한다.
 > 3. stop(7)이거나 PUSH 입력 오류면 반복을 벗어난다.
 > 4. 남은 노드를 모두 해제하고 종료한다.

*/
int main(void)
{
    head = (NODE*)malloc(sizeof(NODE));   // head 동적 할당
    head -> next = NULL;                  // 빈 스택 상태로 초기화

    while (1)
    {
        int cmd = INPUT_command();

        if (cmd == 1)
        {
            if (PUSH() == 0)   // 잘못된 정수 입력이면 종료
                break;
        }

        else if (cmd == 2)
            POP();

        else if (cmd == 3)
            SIZE();

        else if (cmd == 4)
            TOP();

        else if (cmd == 5)
            isEMPTY();

        else if (cmd == 6)
            printSTACK();

        else if (cmd == 7)
            break;

        else if (cmd == 0)
            printf("명령어가 잘못되었습니다.\n");
    }

    CLEANUP();
    // ㄴ남은 모든 노드와 head 동적할당 해제
    
    printf("프로그램을 종료합니다.\n");

    return 0;
}

/* ---------- [3] 명령어 입력 판별 함수 ------------ */
/*

 > 1. "명령어를 입력하세요: " 안내를 출력한 뒤 문자열을 입력받는다.
 > 2. 입력이 끝나면 stop 과 같은 7을 반환해 종료한다.
 > 3. 정해진 명령어에 대응되는 번호를, 없으면 0을 반환한다.

*/
int INPUT_command(void)
{
    // 명령어 문자열. 배열 크기를 비워 두면 '\0'까지 포함한 크기를 자동으로 잡는다.
    char cmd_push[]  = "push";        // 1
    char cmd_pop[]   = "pop";         // 2
    char cmd_size[]  = "size";        // 3
    char cmd_top[]   = "top";         // 4
    char cmd_empty[] = "isEmpty";     // 5
    char cmd_print[] = "printStack";  // 6
    char cmd_stop[]  = "stop";        // 7

    char input[20] = "";   // 빈 문자열로 초기화해 둔다.

    printf("명령어를 입력하세요: ");

    scanf("%19s", input);   // 최대 19글자까지만 읽고 끝에 '\0'을 붙인다. 본 기능은 Claude 조언에 따라 추가되었습니다.

    // 입력이 들어오지 않았으면(입력의 끝 등) input 이 빈 문자열 그대로다.
    if (input[0] == '\0')
        return 7;
        // ㄴstop 과 동일하게 종료. 본 기능은 Claude 조언에 따라 추가되었습니다.

    if      (str_equal(input, cmd_push)  == 1)
        return 1;

    else if (str_equal(input, cmd_pop)   == 1)
        return 2;

    else if (str_equal(input, cmd_size)  == 1)
        return 3;

    else if (str_equal(input, cmd_top)   == 1)
        return 4;

    else if (str_equal(input, cmd_empty) == 1)
        return 5;

    else if (str_equal(input, cmd_print) == 1)
        return 6;

    else if (str_equal(input, cmd_stop)  == 1)
        return 7;

    else
        return 0;
        // ㄴ일치하는 명령 없음
}

/* ---------- [4] 정수 입력 검사 함수 ------------ */
/*

 > 1. 한 글자씩 scanf("%c") 로 읽는다. 앞의 공백, 개행은 건너뛴다.
 >    (명령어를 %19s 로 읽을 때 버퍼에 남은 개행을 흘려보내기 위함)
 > 2. '0'~'9' 아스키 범위면 value = value * 10 + (ip - '0') 로 자릿수를 누적한다.
 > 3. 맨 앞의 '-' 는 음수로 인정한다. (정수용)
 > 4. 공백이나 개행을 만나면 입력이 끝난 것으로 본다.
 > 5. 소수점 '.' 이나 그 외 문자, 숫자가 하나도 없는 입력은 안내 후 0을 반환한다.
 > 6. 통과하면 부호를 곱한 값을 out 에 담고 1을 반환한다.

*/
int INPUT_int(int* out)
{
    char ip;
    int value    = 0;   // 누적할 절댓값
    int sign     = 1;   // 부호: 기본 양수
    int isFirst  = 1;   // 아직 유효한 글자를 만나기 전인지
    int hasDigit = 0;   // 숫자를 하나라도 읽었는지

    while (1)
    {
        scanf("%c", &ip);   // 한 글자 읽기

        // 시작 전이라면 앞쪽의 공백, 개행은 건너뛴다.
        if (isFirst == 1 && (ip == ' ' || ip == '\n' || ip == '\t'))
            continue;

        if (ip >= '0' && ip <= '9')   // 아스키 범위면 숫자로 인식
        {
            value = value * 10 + (int)(ip - '0');   // 형변환으로 자릿수 누적
            hasDigit = 1;
            isFirst  = 0;
        }
        else if (isFirst == 1 && ip == '-')   // 맨 앞의 '-' 는 음수 부호
        {
            sign    = -1;
            isFirst = 0;
        }
        else if (ip == ' ' || ip == '\n' || ip == '\t')   // 입력의 끝
        {
            break;
        }
        else if (ip == '.')   // 소수점: 정수가 아님
        {
            printf("정수만 입력할 수 있습니다. (소수점 불가)\n");
            return 0;
        }
        else   // 그 외 문자: 잘못된 입력
        {
            printf("정수가 아닙니다.\n");
            return 0;
        }
    }

    if (hasDigit == 0)   // 숫자를 하나도 못 읽음(빈 입력, '-'만 입력 등)
    {
        printf("정수가 입력되지 않았습니다.\n");
        return 0;
    }

    *out = value * sign;
    return 1;
    // ㄴ검사를 통과한 값 전달
}

/* ---------- [5] 문자열 비교 함수 ------------ */
/*

 > 1. 앞에서부터 한 글자씩 비교하다 다른 글자를 만나면 0을 반환한다.
 > 2. 한쪽이 '\0'에 닿아 루프를 벗어난 뒤, 양쪽 모두 '\0'이면 완전히 같으므로 1을 반환한다.

*/
int str_equal(const char* a, const char* b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
            return 0;
        // ㄴ다른 글자 발견: 거짓

        i++;
    }

    if (a[i] == b[i])   // 둘 다 '\0'이어야 완전히 같은 문자열
        return 1;
        // ㄴ참

    else
        return 0;
        // ㄴ길이가 다름: 거짓
}

/* ---------- [6] PUSH 함수 ------------ */
/*

 > 1. "PUSH 하고자 하는 정수를 입력하세요: " 안내 후 INPUT_int 로 정수를 검사받는다.
 > 2. 입력이 잘못되면 0을 반환한다.
 > 3. 새 노드를 만들어 맨 앞(top)에 끼워 넣고 1을 반환한다.

*/
int PUSH(void)
{
    int num;

    printf("PUSH 하고자 하는 정수를 입력하세요: ");

    if (INPUT_int(&num) == 0)
        return 0;
    // ㄴ잘못된 입력

    NODE* newNode = (NODE*)malloc(sizeof(NODE));
    newNode -> data = num;

    // 맨 앞에 삽입한다. 순서가 중요하다.
    newNode -> next = head -> next;   // 새 노드가 기존 top 을 먼저 가리키게 하고
    head -> next    = newNode;        // head 가 새 노드를 새 top 으로 가리킨다

    return 1;   // ㄴ성공
}

/* ---------- [7] POP 함수 ------------ */
/*

 > 1. 스택이 비어 있으면 안내하고 끝낸다.
 > 2. 맨 앞(top) 노드를 붙잡아 값을 출력한다.
 > 3. head 가 그 다음 노드를 새 top 으로 가리키게 한 뒤 떼어낸 노드를 free 한다.

*/
void POP(void)
{
    if (head -> next == NULL)
    {
        printf("스택이 비어 있습니다.\n");
        return;
    }

    NODE* topNode = head -> next;   // 현재 top 노드를 먼저 붙잡아 둔다

    printf("POP: %d\n", topNode -> data);

    head -> next = topNode -> next;   // 다음 노드를 새 top 으로 연결
    free(topNode);
    // ㄴ떼어낸 노드 해제
}

/* ---------- [8] SIZE 함수 ------------ */
/*

 > 1. head 다음(top)부터 NULL 까지 순회하며 개수를 센다.
 > 2. head 는 세지 않으려고 head->next 부터 시작한다.

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

/* ---------- [9] TOP 함수 ------------ */
/*

 > 1. 비어 있으면 안내하고 끝낸다.
 > 2. 삭제하지 않고 맨 위 값만 출력한다.

*/
void TOP(void)
{
    if (head -> next == NULL)
    {
        printf("스택이 비어 있습니다.\n");
        return;
    }

    printf("TOP: %d\n", head -> next -> data);
}

/* ---------- [10] isEMPTY 함수 ------------ */
/*

 > 1. head->next 가 NULL 이면 스택이 비었으므로 true 를 출력한다.
 > 2. 아니면 값이 있으므로 false 를 출력한다.

*/
void isEMPTY(void)
{
    if (head -> next == NULL)
        printf("isEmpty: true\n");
    else
        printf("isEmpty: false\n");
}

/* ---------- [11] printSTACK 함수 ------------ */
/*

 > 1. 비어 있으면 안내하고 끝낸다.
 > 2. 맨 앞(top)부터 NULL 까지 순회하며 모든 값을 출력한다.

*/
void printSTACK(void)
{
    if (head -> next == NULL)
    {
        printf("스택이 비어 있습니다.\n");
        return;
    }

    NODE* curr = head -> next;   // 맨 앞(top)부터 출력
    printf("top -> [ ");
    while (curr != NULL)
    {
        printf("%d ", curr -> data);
        curr = curr -> next;
    }
    printf("]\n");
}

/* ---------- [12] CLEANUP 함수 ------------ */
/*

 > 1. head 부터 시작해 노드를 하나씩 해제한다.
 > 2. free 하기 전에 다음 노드 주소를 먼저 저장해 둬야 사슬을 계속 따라갈 수 있다.

*/
void CLEANUP(void)
{
    NODE* curr = head;
    while (curr != NULL)
    {
        NODE* nextNode = curr -> next;   // 다음 노드 주소를 먼저 저장하고
        free(curr);
        // ㄴ현재 노드 해제
        
        curr = nextNode;                 // 저장해 둔 다음 노드로 이동
    }
    head = NULL;
}
