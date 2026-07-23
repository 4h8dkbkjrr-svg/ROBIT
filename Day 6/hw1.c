//  ROBIT C DAY 6
//  ㄴ hw1
//  ㄴㄴ hw1.c
//
//  <AI 사용 안내>
//  > 1. 코드 제작 과정에서 일부 Claude가 개입했습니다.
//  > 2. Claude 사용 원칙 및 사용 과정은 폴더에 들어있는 README.md 파일을 통해 투명하게 공개됩니다.
//  > 3. Claude의 조언을 받았거나, 개입한 부분을 주석을 통해 투명하게 표기했습니다.
//  > ㄴ 3.1. Claude가 함수에 광범위하게 개입한 경우, [1] 함수 원형 선언에서 Claude가 개입했음을 표기했습니다.
//  > ㄴ 3.2. Claude가 함수에 제한적으로 개입한 경우, 각 함수에서 Claude가 개입한 줄 옆에 주석처리로 Claude가 개입했음을 표기했습니다.
//  > ㄴ 3.3  Claude가 미세하게 개입한 경우, 개입 내역이 생략될 수 있습니다.
//
//  Created by Lee DY on 7/23/26. Xcode

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
//  > 2. 일치하는 번호(1~11)를, 없으면 0을 반환한다.
//
//  > * INPUT_command 함수는 Claude의 도움을 받아 작성되었습니다.

int INPUT_int(int* out);
// ㄴ정수 입력 검사 함수 (음수 허용)
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고 맨 앞의 '-'는 음수로 인정한다.
//  > 2. 성공 시 값을 out 에 담아 1을, 잘못된 입력이면 안내 후 0을 반환한다.
//
//  > * INPUT_int 함수는 Claude의 도움을 받아 작성되었습니다.

int str_equal(const char* a, const char* b);
// ㄴ두 문자열이 같은지 비교하는 함수 (string.h 미사용)
//  > 1. 앞에서부터 한 글자씩 비교하다 다르면 0을, 완전히 같으면 1을 반환한다.
//
//  > * str_equal 함수는 Claude의 도움을 받아 작성되었습니다.

NODE* CREATE_node(int data);
// ㄴ노드 하나를 동적 할당해 값을 채워 주소를 반환하는 함수
//  > 1. malloc 으로 노드를 만들고 data 를 채운 뒤 그 주소를 반환한다.

int GET_length(void);
// ㄴ리스트 전체 길이를 세어 반환하는 함수
//  > 1. head 다음부터 NULL 까지 순회하며 개수를 세어 반환한다.

void INSERT(void);
// ㄴ원하는 위치에 노드를 추가하는 함수 (index 기준 또는 data 기준)
//  > 1. 기준(index/data)과 위치, 넣을 값을 입력받아 그 자리에 노드를 삽입한다.

void INSERT_back(void);
// ㄴ리스트 맨 끝에 노드를 추가하는 함수

void INSERT_first(void);
// ㄴ리스트 맨 앞에 노드를 추가하는 함수

void DELETE(void);
// ㄴ원하는 요소를 삭제하는 함수 (index 기준 또는 data 기준)
//
//  > * DELETE 함수는 Claude의 도움을 받아 작성되었습니다.

void DELETE_first(void);
// ㄴ리스트 맨 앞 노드를 삭제하는 함수

void DELETE_back(void);
// ㄴ리스트 맨 마지막 노드를 삭제하는 함수

void GET_entry(void);
// ㄴ요소를 찾는 함수 (data 로 찾으면 index, index 로 찾으면 data 를 출력)

void PRINT_list(void);
// ㄴ리스트의 모든 요소를 출력하는 함수
//
//  > * PRINT_list 함수는 Claude의 도움을 받아 작성되었습니다.

void REVERSE(void);
// ㄴ리스트를 역순으로 뒤집는 함수
//
//  > * REVERSE 함수는 Claude의 도움을 받아 작성되었습니다.

void CLEANUP(void);
// ㄴ종료 시 남은 모든 노드와 더미 head 를 해제하는 함수
//  > 1. head 부터 끝까지 순회하며 노드를 하나씩 free 해 메모리 누수를 막는다.
//
//  > * CLEANUP 함수의 경우 Claude의 조언에 따라 추가했고, Claude의 도움을 받아 작성되었습니다.

// head 자체에는 값을 넣지 않고, head->next 부터가 실제 스택이다.

/* ---------- [2] main문 ------------ */
/*

 > 1. 더미 head 를 할당해 빈 리스트로 초기화한다.
 > 2. 명령 번호를 입력받아 알맞은 함수를 반복 호출한다.
 > 3. stop 이면 반복을 벗어나 남은 노드를 해제하고 종료한다.

*/
int main(void)
{
    head = (NODE*)malloc(sizeof(NODE));
    head -> next = NULL;

    while (1)
    {
        int cmd = INPUT_command();

        if      (cmd == 1)
            INSERT();
        
        else if (cmd == 2)
            INSERT_back();
        
        else if (cmd == 3)
            INSERT_first();
        
        else if (cmd == 4)
            DELETE();
        
        else if (cmd == 5)
            DELETE_first();
        
        else if (cmd == 6)
            DELETE_back();
        
        else if (cmd == 7)
            GET_entry();
        
        else if (cmd == 8)
            GET_length();
        
        else if (cmd == 9)
            PRINT_list();
        
        else if (cmd == 10)
            REVERSE();
        
        else if (cmd == 11)
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
 > 2. 입력이 없으면(입력의 끝) stop 과 같은 11을 반환한다.
 > 3. 정해진 명령어에 대응되는 번호를, 없으면 0을 반환한다.

*/
int INPUT_command(void)
{
    char cmd_insert[]   = "insert";        // 1
    char cmd_iback[]    = "insert_back";   // 2
    char cmd_ifirst[]   = "insert_first";  // 3
    char cmd_delete[]   = "delete";        // 4
    char cmd_dfirst[]   = "delete_first";  // 5
    char cmd_dback[]    = "delete_back";   // 6
    char cmd_entry[]    = "get_entry";     // 7
    char cmd_length[]   = "get_length";    // 8
    char cmd_print[]    = "print_list";    // 9
    char cmd_reverse[]  = "reverse";       // 10
    char cmd_stop[]     = "stop";          // 11

    char input[20] = "";

    printf("\n명령어를 입력하세요\n");
    printf("(insert, insert_back, insert_first, delete, delete_first, delete_back, get_entry, get_length, print_list, reverse, stop): ");

    scanf("%19s", input);   // 최대 19글자까지만 읽고 끝에 '\0'을 붙인다. 본 기능은 Claude 조언에 따라 추가되었습니다.

    if (input[0] == '\0')
        return 11;   // 입력의 끝: stop 과 동일
        // ㄴstop 과 동일하게 종료. 본 기능은 Claude 조언에 따라 추가되었습니다.

    if      (str_equal(input, cmd_insert)  == 1)
        return 1;
    
    else if (str_equal(input, cmd_iback)   == 1)
        return 2;
    
    else if (str_equal(input, cmd_ifirst)  == 1)
        return 3;
    
    else if (str_equal(input, cmd_delete)  == 1)
        return 4;
    
    else if (str_equal(input, cmd_dfirst)  == 1)
        return 5;
    
    else if (str_equal(input, cmd_dback)   == 1)
        return 6;
    
    else if (str_equal(input, cmd_entry)   == 1)
        return 7;
    
    else if (str_equal(input, cmd_length)  == 1)
        return 8;
    
    else if (str_equal(input, cmd_print)   == 1)
        return 9;
    
    else if (str_equal(input, cmd_reverse) == 1)
        return 10;
    
    else if (str_equal(input, cmd_stop)    == 1)
        return 11;
    
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

 > 1. 앞에서부터 한 글자씩 비교하다 다르면 0을 반환한다.
 > 2. 둘 다 '\0'에서 끝나 완전히 같으면 1을 반환한다.

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

/* ---------- [6] 노드 생성 함수 ------------ */
/*

 > 1. malloc 으로 노드를 하나 만들고 data 를 채운다.
 > 2. next 는 NULL 로 두고 그 주소를 반환한다.

*/
NODE* CREATE_node(int data)
{
    NODE* newNode = (NODE*)malloc(sizeof(NODE));
    newNode -> data = data;
    newNode -> next = NULL;
    return newNode;
}

/* ---------- [7] 길이 반환 함수 ------------ */
/*

 > 1. head 다음부터 NULL 까지 순회하며 개수를 센다.

*/
int GET_length(void)
{
    int count = 0;
    NODE* curr = head -> next;

    while (curr != NULL)
    {
        count = count + 1;
        curr = curr -> next;
    }

    return count;
}

/* ---------- [8] 원하는 위치 삽입 함수 ------------ */
/*

 > 1. 기준을 입력받는다. (1: index 기준, 2: data 기준)
 > 2. index 기준이면 그 위치(0부터) 앞에 새 노드를 삽입한다.
 > 3. data 기준이면 그 값을 가진 노드 바로 뒤에 새 노드를 삽입한다.
 > 4. 넣을 값을 입력받아 삽입한다.

*/
void INSERT(void)
{
    int mode;
    int value;

    printf("삽입 기준을 선택하세요 (1: index, 2: data): ");
    if (INPUT_int(&mode) == 0)
        return;

    if (mode == 1)   // index 기준
    {
        int index;
        int i;
        NODE* curr = head;

        printf("삽입할 index 를 입력하세요 (0 ~ %d): ", GET_length());
        if (INPUT_int(&index) == 0)
            return;

        if (index < 0 || index > GET_length())
        {
            printf("index 범위를 벗어났습니다.\n");
            return;
        }

        printf("삽입할 값을 입력하세요: ");
        if (INPUT_int(&value) == 0)
            return;

        // index 번째 노드 앞에 삽입하려면 그 앞(index-1)까지 이동한다.
        for (i = 0; i < index; i++)
            curr = curr -> next;

        NODE* newNode = CREATE_node(value);
        newNode -> next = curr -> next;
        curr -> next = newNode;

        printf("index %d 위치에 %d 를 삽입했습니다.\n", index, value);
    }
    else if (mode == 2)   // data 기준
    {
        int target;
        NODE* curr = head -> next;

        printf("어느 값 뒤에 삽입할지 입력하세요: ");
        if (INPUT_int(&target) == 0)
            return;

        while (curr != NULL && curr -> data != target)
            curr = curr -> next;

        if (curr == NULL)
        {
            printf("%d 값을 찾지 못했습니다.\n", target);
            return;
        }

        printf("삽입할 값을 입력하세요: ");
        if (INPUT_int(&value) == 0)
            return;

        NODE* newNode = CREATE_node(value);
        newNode -> next = curr -> next;
        curr -> next = newNode;

        printf("%d 뒤에 %d 를 삽입했습니다.\n", target, value);
    }
    else
    {
        printf("기준이 잘못되었습니다.\n");
    }
}

/* ---------- [9] 맨 끝 삽입 함수 ------------ */
/*

 > 1. 값을 입력받아 새 노드를 만든다.
 > 2. head 부터 마지막 노드까지 이동해 그 뒤에 연결한다.

*/
void INSERT_back(void)
{
    int value;
    NODE* curr = head;

    printf("맨 끝에 삽입할 값을 입력하세요: ");
    if (INPUT_int(&value) == 0)
        return;

    while (curr -> next != NULL)
        curr = curr -> next;

    curr -> next = CREATE_node(value);
    printf("맨 끝에 %d 를 삽입했습니다.\n", value);
}

/* ---------- [10] 맨 앞 삽입 함수 ------------ */
/*

 > 1. 값을 입력받아 새 노드를 만든다.
 > 2. head 바로 뒤(맨 앞)에 연결한다.

*/
void INSERT_first(void)
{
    int value;

    printf("맨 앞에 삽입할 값을 입력하세요: ");
    if (INPUT_int(&value) == 0)
        return;

    NODE* newNode = CREATE_node(value);
    newNode -> next = head -> next;
    head -> next = newNode;
    printf("맨 앞에 %d 를 삽입했습니다.\n", value);
}

/* ---------- [11] 원하는 요소 삭제 함수 ------------ */
/*

 > 1. 기준을 입력받는다. (1: index 기준, 2: data 기준)
 > 2. 해당 노드의 앞 노드를 찾아 연결을 잇고 노드를 free 한다.

*/
void DELETE(void)
{
    int mode;
    NODE* prev = head;

    if (head -> next == NULL)
    {
        printf("리스트가 비어 있습니다.\n");
        return;
    }

    printf("삭제 기준을 선택하세요 (1: index, 2: data): ");
    if (INPUT_int(&mode) == 0)
        return;

    if (mode == 1)   // index 기준
    {
        int index;
        int i;

        printf("삭제할 index 를 입력하세요 (0 ~ %d): ", GET_length() - 1);
        if (INPUT_int(&index) == 0)
            return;

        if (index < 0 || index > GET_length() - 1)
        {
            printf("index 범위를 벗어났습니다.\n");
            return;
        }

        for (i = 0; i < index; i++)
            prev = prev -> next;
    }
    else if (mode == 2)   // data 기준
    {
        int target;

        printf("삭제할 값을 입력하세요: ");
        if (INPUT_int(&target) == 0)
            return;

        while (prev -> next != NULL && prev -> next -> data != target)
            prev = prev -> next;

        if (prev -> next == NULL)
        {
            printf("%d 값을 찾지 못했습니다.\n", target);
            return;
        }
    }
    else
    {
        printf("기준이 잘못되었습니다.\n");
        return;
    }

    NODE* delNode = prev -> next;
    printf("%d 를 삭제했습니다.\n", delNode -> data);
    prev -> next = delNode -> next;
    free(delNode);
}

/* ---------- [12] 맨 앞 삭제 함수 ------------ */
/*

 > 1. 비어 있으면 안내한다.
 > 2. head 다음 노드를 떼어내 free 한다.

*/
void DELETE_first(void)
{
    if (head -> next == NULL)
    {
        printf("리스트가 비어 있습니다.\n");
        return;
    }

    NODE* delNode = head -> next;
    printf("맨 앞 %d 를 삭제했습니다.\n", delNode -> data);
    head -> next = delNode -> next;
    free(delNode);
}

/* ---------- [13] 맨 끝 삭제 함수 ------------ */
/*

 > 1. 비어 있으면 안내한다.
 > 2. 마지막 노드의 앞까지 이동해 마지막 노드를 free 한다.

*/
void DELETE_back(void)
{
    NODE* prev = head;

    if (head -> next == NULL)
    {
        printf("리스트가 비어 있습니다.\n");
        return;
    }

    while (prev -> next -> next != NULL)
        prev = prev -> next;

    NODE* delNode = prev -> next;
    printf("맨 끝 %d 를 삭제했습니다.\n", delNode -> data);
    prev -> next = NULL;
    free(delNode);
}

/* ---------- [14] 요소 찾기 함수 ------------ */
/*

 > 1. 기준을 입력받는다. (1: data 로 찾아 index 출력, 2: index 로 찾아 data 출력)
 > 2. data 기준이면 처음 일치하는 노드의 index 를 출력한다.
 > 3. index 기준이면 그 위치 노드의 data 를 출력한다.

*/
void GET_entry(void)
{
    int mode;

    printf("찾기 기준을 선택하세요 (1: data로 index 찾기, 2: index로 data 찾기): ");
    if (INPUT_int(&mode) == 0)
        return;

    if (mode == 1)   // data 로 index 찾기
    {
        int target;
        int index = 0;
        NODE* curr = head -> next;

        printf("찾을 값을 입력하세요: ");
        if (INPUT_int(&target) == 0)
            return;

        while (curr != NULL)
        {
            if (curr -> data == target)
            {
                printf("값 %d 는 index %d 에 있습니다.\n", target, index);
                return;
            }
            curr = curr -> next;
            index = index + 1;
        }
        printf("%d 값을 찾지 못했습니다.\n", target);
    }
    else if (mode == 2)   // index 로 data 찾기
    {
        int index;
        int i;
        NODE* curr = head -> next;

        printf("찾을 index 를 입력하세요: ");
        if (INPUT_int(&index) == 0)
            return;

        if (index < 0 || index > GET_length() - 1)
        {
            printf("index 범위를 벗어났습니다.\n");
            return;
        }

        for (i = 0; i < index; i++)
            curr = curr -> next;

        printf("index %d 의 값은 %d 입니다.\n", index, curr -> data);
    }
    else
    {
        printf("기준이 잘못되었습니다.\n");
    }
}

/* ---------- [15] 전체 출력 함수 ------------ */
/*

 > 1. 비어 있으면 안내한다.
 > 2. head 다음부터 끝까지 순회하며 값을 출력한다.

*/
void PRINT_list(void)
{
    NODE* curr = head -> next;

    if (curr == NULL)
    {
        printf("리스트가 비어 있습니다.\n");
        return;
    }

    printf("[ ");
    while (curr != NULL)
    {
        printf("%d ", curr -> data);
        curr = curr -> next;
    }
    printf("] (길이: %d)\n", GET_length());
}

/* ---------- [16] 역순 함수 ------------ */
/*

 > 1. 세 개의 포인터(prev, curr, next)로 링크 방향을 하나씩 뒤집는다.
 > 2. 마지막에 head->next 가 뒤집힌 첫 노드를 가리키게 한다.

*/
void REVERSE(void)
{
    NODE* prev = NULL;
    NODE* curr = head -> next;
    NODE* next = NULL;

    while (curr != NULL)
    {
        next = curr -> next;   // 다음 노드를 먼저 저장하고
        curr -> next = prev;   // 링크 방향을 뒤로 돌린 뒤
        prev = curr;           // prev, curr 를 한 칸씩 전진시킨다
        curr = next;
    }

    head -> next = prev;
    printf("리스트를 역순으로 뒤집었습니다.\n");
}

/* ---------- [17] 메모리 정리 함수 ------------ */
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
}
