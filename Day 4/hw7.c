//  ROBIT C PROJECT DAY 4
//  ㄴ hw7
//  ㄴㄴ hw7.c
//
//  Created by Lee DY on 7/22/26.
//

#include <stdio.h>
#include <stdlib.h>

typedef struct _Node
{
    int data;              // 자료
    struct _Node* next;    // 다음 노드를 가리키는 포인터
} Node;

typedef struct _LinkedList
{
    Node* head;   // 첫 번째 노드
    Node* tail;   // 마지막 노드
    Node* cur;    // 순회에 쓰는 현재 위치 노드
    int size;     // 노드 개수
} LinkedList;

/* ------------ [1] 함수 원형 선언 ------------ */

Node* MALLOC_node(int data);
// ㄴ노드 동적 할당 함수
//  > 1. 노드 하나 크기만큼 주소를 동적 할당하고 자료를 담으며
//  > 2. 해당 주소를 반환한다.

int INSERT_first(LinkedList* pList, int data);
// ㄴ맨 처음 삽입 함수
//  > 1. 연결 리스트의 맨 처음에 새 노드를 추가하고
//  > 2. 성공 시 1을, 할당 실패 시 0을 반환한다.

int INSERT_back(LinkedList* pList, int data);
// ㄴ맨 끝 삽입 함수
//  > 1. 연결 리스트의 맨 끝에 새 노드를 추가하고
//  > 2. 성공 시 1을, 할당 실패 시 0을 반환한다.

int INSERT_byIndex(LinkedList* pList, int index, int data);
// ㄴ원하는 위치(index) 삽입 함수
//  > 1. index 위치에 새 노드를 추가한다. (0 ~ size 허용)
//  > 2. 범위를 벗어나거나 할당에 실패하면 안내 후 0을, 성공 시 1을 반환한다.

int INSERT_byData(LinkedList* pList, int targetData, int data);
// ㄴ원하는 위치(data) 삽입 함수
//  > 1. targetData 를 가진 노드를 찾아 그 앞에 새 노드를 추가한다.
//  > 2. 해당 자료가 없거나 할당에 실패하면 안내 후 0을, 성공 시 1을 반환한다.

int DELETE_first(LinkedList* pList);
// ㄴ맨 처음 삭제 함수
//  > 1. 연결 리스트의 맨 처음 노드를 삭제하고
//  > 2. 성공 시 1을, 리스트가 비어 있으면 0을 반환한다.

int DELETE_back(LinkedList* pList);
// ㄴ맨 끝 삭제 함수
//  > 1. 연결 리스트의 맨 마지막 노드를 삭제하고
//  > 2. 성공 시 1을, 리스트가 비어 있으면 0을 반환한다.

int DELETE_byIndex(LinkedList* pList, int index);
// ㄴ원하는 요소(index) 삭제 함수
//  > 1. index 위치의 노드를 삭제한다. (0 ~ size - 1 허용)
//  > 2. 범위를 벗어나면 안내 후 0을, 성공 시 1을 반환한다.

int DELETE_byData(LinkedList* pList, int data);
// ㄴ원하는 요소(data) 삭제 함수
//  > 1. data 를 가진 노드를 찾아 삭제한다.
//  > 2. 해당 자료가 없으면 안내 후 0을, 성공 시 1을 반환한다.

int GET_entryByData(LinkedList* pList, int data);
// ㄴ요소 찾기 함수 (data -> index)
//  > 1. 리스트를 앞에서부터 순회하며 data 를 가진 노드를 찾고
//  > 2. 찾으면 그 위치(index)를, 없으면 -1을 반환한다.

int GET_entryByIndex(LinkedList* pList, int index, int* out);
// ㄴ요소 찾기 함수 (index -> data)
//  > 1. index 위치의 노드를 찾아 자료를 out 포인터에 담고
//  > 2. 성공 시 1을, 범위를 벗어나면 안내 후 0을 반환한다.

int GET_length(LinkedList* pList);
// ㄴ길이 반환 함수
//  > 1. 리스트 전체 길이(노드 개수)를 반환한다.

void PRINT_list(LinkedList* pList);
// ㄴ리스트 출력 함수
//  > 1. 리스트의 모든 요소를 head 부터 순서대로 출력한다.

void REVERSE(LinkedList* pList);
// ㄴ역순 함수
//  > 1. 각 노드의 next 방향을 반대로 뒤집어 리스트를 역순으로 만든다.

void FREE_list(LinkedList* pList);
// ㄴ리스트 해제 함수
//  > 1. 리스트의 모든 노드를 앞에서부터 free 로 해제한다.

/* ---------- [2] main문 ------------ */
/*

 > 요구된 모든 함수의 동작을 보여주는 데모 시나리오로 구성한다.
 > 1. 빈 리스트에서 시작해 삽입 4종 -> 찾기, 길이 -> 삭제 4종 -> 역순 순서로 실행하며
 > 2. 각 단계마다 리스트 상태를 출력해 결과를 확인한다.
 > 3. 삽입 단계에서 할당 실패(반환값 0)가 발생하면
 >    이미 할당한 노드를 모두 해제한 뒤 그 즉시 종료한다.
 > 4. 마지막에 FREE_list 로 남은 노드를 모두 해제하고 종료한다.

*/
int main(void)
{
    LinkedList list = { NULL, NULL, NULL, 0 };   // 빈 리스트로 초기화

    printf("[1] INSERT_back 100, 200, 300\n");
    if (INSERT_back(&list, 100) == 0 || INSERT_back(&list, 200) == 0 || INSERT_back(&list, 300) == 0)
    {
        FREE_list(&list);
        return 0;
    }
    PRINT_list(&list);

    printf("[2] INSERT_first 50\n");
    if (INSERT_first(&list, 50) == 0)
    {
        FREE_list(&list);
        return 0;
    }
    PRINT_list(&list);

    printf("[3] INSERT_byIndex: index 2 에 150 삽입\n");
    if (INSERT_byIndex(&list, 2, 150) == 0)
    {
        FREE_list(&list);
        return 0;
    }
    PRINT_list(&list);

    printf("[4] INSERT_byData: 200 앞에 180 삽입\n");
    if (INSERT_byData(&list, 200, 180) == 0)
    {
        FREE_list(&list);
        return 0;
    }
    PRINT_list(&list);

    printf("[5] GET_entryByData: 200 은 index %d\n", GET_entryByData(&list, 200));

    int value;
    printf("[6] GET_entryByIndex: ");
    if (GET_entryByIndex(&list, 0, &value) == 1)
        printf("index 0 의 data 는 %d\n", value);

    printf("[7] GET_entryByData: 없는 데이터 999 는 index %d\n", GET_entryByData(&list, 999));
    // ㄴ찾지 못하면 -1 이 반환되는 것을 확인

    printf("[8] GET_length: 길이 %d\n", GET_length(&list));

    printf("[9] DELETE_byData: 180 삭제\n");
    DELETE_byData(&list, 180);
    PRINT_list(&list);

    printf("[10] DELETE_byIndex: index 2 삭제\n");
    DELETE_byIndex(&list, 2);
    PRINT_list(&list);

    printf("[11] DELETE_first\n");
    DELETE_first(&list);
    PRINT_list(&list);

    printf("[12] DELETE_back\n");
    DELETE_back(&list);
    PRINT_list(&list);

    printf("[13] REVERSE\n");
    REVERSE(&list);
    PRINT_list(&list);

    FREE_list(&list);
    // ㄴ남은 노드를 모두 해제

    return 0;
}

/* ---------- [1] 노드 동적 할당 함수 ------------ */
/*

 > 노드 할당 함수는 Node형의 주소를 반환하므로, Node* 로 선언한다.
 > 1. sizeof(Node) 크기만큼 malloc 으로 할당하고
 > 2. 할당에 실패(NULL)하면 안내를 출력한 뒤 NULL 을 반환하며,
 > 3. 성공하면 자료를 담고 next 를 NULL 로 초기화한 주소를 반환한다.

*/
Node* MALLOC_node(int data)
{
    Node* pNode = (Node*)malloc(sizeof(Node));

    if (pNode == NULL)
    {
        printf("메모리 할당에 실패했습니다.\n");
        return NULL;
    }
    // ㄴ실패: NULL 반환

    pNode->data = data;
    pNode->next = NULL;

    return pNode;
    // ㄴ성공: 할당한 주소 반환
}

/* ---------- [2] 맨 처음 삽입 함수 ------------ */
/*

 > 1. 새 노드를 할당하고, 실패하면 0을 반환한다.
 > 2. 새 노드의 next 가 기존 head 를 가리키게 한 뒤 head 를 새 노드로 바꾼다.
 > 3. 리스트가 비어 있었다면 새 노드가 tail 도 겸한다.
 > 4. 크기를 1 늘리고 1을 반환한다.

*/
int INSERT_first(LinkedList* pList, int data)
{
    Node* pNew = MALLOC_node(data);
    if (pNew == NULL)
        return 0;

    pNew->next = pList->head;   // 새 노드 -> 기존 첫 노드
    pList->head = pNew;

    if (pList->tail == NULL)   // 빈 리스트였다면 tail 도 새 노드
        pList->tail = pNew;

    pList->size++;
    return 1;
}

/* ---------- [3] 맨 끝 삽입 함수 ------------ */
/*

 > 1. 새 노드를 할당하고, 실패하면 0을 반환한다.
 > 2. 기존 tail 의 next 가 새 노드를 가리키게 한 뒤 tail 을 새 노드로 바꾼다.
 > 3. 리스트가 비어 있었다면 새 노드가 head 도 겸한다.
 > 4. 크기를 1 늘리고 1을 반환한다.

*/
int INSERT_back(LinkedList* pList, int data)
{
    Node* pNew = MALLOC_node(data);
    if (pNew == NULL)
        return 0;

    if (pList->tail == NULL)   // 빈 리스트: 새 노드가 head 이자 tail
    {
        pList->head = pNew;
        pList->tail = pNew;
    }
    else
    {
        pList->tail->next = pNew;   // 기존 마지막 노드 -> 새 노드
        pList->tail = pNew;
    }

    pList->size++;
    return 1;
}

/* ---------- [4] 원하는 위치(index) 삽입 함수 ------------ */
/*

 > 1. index 가 0 ~ size 범위를 벗어나면 안내 후 0을 반환한다.
 > 2. 맨 앞(0)과 맨 끝(size)은 이미 만든 INSERT_first, INSERT_back 을 재사용한다.
 > 3. 그 외에는 cur 를 head 에서 (index - 1)번 옮겨 삽입 위치 앞 노드에 세우고,
 > 4. 새 노드가 cur 의 다음을 가리키게 한 뒤 cur 의 다음을 새 노드로 바꾼다.
 > 5. 크기를 1 늘리고 1을 반환한다.

*/
int INSERT_byIndex(LinkedList* pList, int index, int data)
{
    if (index < 0 || index > pList->size)   // 삽입은 size 위치(맨 끝 다음)까지 허용
    {
        printf("잘못된 위치입니다.\n");
        return 0;
    }

    if (index == 0)
        return INSERT_first(pList, data);
    if (index == pList->size)
        return INSERT_back(pList, data);

    Node* pNew = MALLOC_node(data);
    if (pNew == NULL)
        return 0;

    pList->cur = pList->head;
    for (int i = 0; i < index - 1; i++)   // 삽입 위치 바로 앞 노드까지 이동
        pList->cur = pList->cur->next;

    pNew->next = pList->cur->next;   // 새 노드 -> 원래 index 위치의 노드
    pList->cur->next = pNew;         // 앞 노드 -> 새 노드

    pList->size++;
    return 1;
}

/* ---------- [5] 원하는 위치(data) 삽입 함수 ------------ */
/*

 > 1. GET_entryByData 로 targetData 를 가진 노드의 위치를 찾는다.
 > 2. 찾지 못하면(-1) 안내 후 0을 반환한다.
 > 3. 찾았으면 그 위치에 삽입하도록 INSERT_byIndex 를 재사용한다.
 >    (그 위치에 삽입하면 새 노드가 targetData 노드 앞에 놓인다)

*/
int INSERT_byData(LinkedList* pList, int targetData, int data)
{
    int index = GET_entryByData(pList, targetData);

    if (index == -1)
    {
        printf("해당 데이터가 없습니다.\n");
        return 0;
    }

    return INSERT_byIndex(pList, index, data);
}

/* ---------- [6] 맨 처음 삭제 함수 ------------ */
/*

 > 1. 리스트가 비어 있으면 안내 후 0을 반환한다.
 > 2. head 를 두 번째 노드로 옮기고 원래 첫 노드를 해제한다.
 > 3. 삭제 후 리스트가 비면 tail 도 NULL 로 만든다.
 > 4. 크기를 1 줄이고 1을 반환한다.

*/
int DELETE_first(LinkedList* pList)
{
    if (pList->head == NULL)
    {
        printf("리스트가 비어 있습니다.\n");
        return 0;
    }

    Node* pDelete = pList->head;
    pList->head = pList->head->next;   // head 를 두 번째 노드로

    if (pList->head == NULL)   // 마지막 남은 노드를 지운 경우
        pList->tail = NULL;

    free(pDelete);
    // ㄴ삭제한 노드의 메모리 해제

    pList->size--;
    return 1;
}

/* ---------- [7] 맨 끝 삭제 함수 ------------ */
/*

 > 1. 리스트가 비어 있으면 안내 후 0을 반환한다.
 > 2. 노드가 하나뿐이면 DELETE_first 를 재사용한다.
 > 3. cur 를 head 에서 출발시켜 tail 바로 앞 노드까지 이동시킨다.
 > 4. tail 을 해제하고, 앞 노드를 새 tail 로 만들어 next 를 NULL 로 끊는다.
 > 5. 크기를 1 줄이고 1을 반환한다.

*/
int DELETE_back(LinkedList* pList)
{
    if (pList->head == NULL)
    {
        printf("리스트가 비어 있습니다.\n");
        return 0;
    }

    if (pList->head == pList->tail)   // 노드가 하나뿐인 경우
        return DELETE_first(pList);

    pList->cur = pList->head;
    while (pList->cur->next != pList->tail)   // tail 바로 앞 노드까지 이동
        pList->cur = pList->cur->next;

    free(pList->tail);
    // ㄴ삭제한 노드의 메모리 해제

    pList->tail = pList->cur;    // 앞 노드가 새 tail
    pList->tail->next = NULL;    // 마지막 노드이므로 연결을 끊음

    pList->size--;
    return 1;
}

/* ---------- [8] 원하는 요소(index) 삭제 함수 ------------ */
/*

 > 1. index 가 0 ~ size - 1 범위를 벗어나면 안내 후 0을 반환한다.
 > 2. 맨 앞(0)과 맨 끝(size - 1)은 DELETE_first, DELETE_back 을 재사용한다.
 > 3. 그 외에는 cur 를 삭제할 노드 바로 앞까지 이동시키고,
 > 4. 앞 노드의 next 가 삭제할 노드의 다음을 가리키게 이은 뒤 해제한다.
 > 5. 크기를 1 줄이고 1을 반환한다.

*/
int DELETE_byIndex(LinkedList* pList, int index)
{
    if (index < 0 || index >= pList->size)
    {
        printf("잘못된 위치입니다.\n");
        return 0;
    }

    if (index == 0)
        return DELETE_first(pList);
    if (index == pList->size - 1)
        return DELETE_back(pList);

    pList->cur = pList->head;
    for (int i = 0; i < index - 1; i++)   // 삭제할 노드 바로 앞까지 이동
        pList->cur = pList->cur->next;

    Node* pDelete = pList->cur->next;
    pList->cur->next = pDelete->next;   // 앞 노드 -> 삭제할 노드의 다음 노드

    free(pDelete);
    // ㄴ삭제한 노드의 메모리 해제

    pList->size--;
    return 1;
}

/* ---------- [9] 원하는 요소(data) 삭제 함수 ------------ */
/*

 > 1. GET_entryByData 로 data 를 가진 노드의 위치를 찾는다.
 > 2. 찾지 못하면(-1) 안내 후 0을 반환한다.
 > 3. 찾았으면 그 위치의 노드를 지우도록 DELETE_byIndex 를 재사용한다.

*/
int DELETE_byData(LinkedList* pList, int data)
{
    int index = GET_entryByData(pList, data);

    if (index == -1)
    {
        printf("해당 데이터가 없습니다.\n");
        return 0;
    }

    return DELETE_byIndex(pList, index);
}

/* ---------- [10] 요소 찾기 함수 (data -> index) ------------ */
/*

 > 1. cur 를 head 에 놓고 cur = cur->next 로 한 노드씩 이동하며
 > 2. 자료가 data 와 같은 노드를 만나면 그 위치(index)를 반환한다.
 > 3. 끝(NULL)까지 찾지 못하면 -1을 반환한다.

*/
int GET_entryByData(LinkedList* pList, int data)
{
    int index = 0;

    pList->cur = pList->head;
    while (pList->cur != NULL)
    {
        if (pList->cur->data == data)   // 찾는 자료를 만남
            return index;

        pList->cur = pList->cur->next;
        index++;
    }

    return -1;
    // ㄴ리스트에 없는 자료: -1 반환
}

/* ---------- [11] 요소 찾기 함수 (index -> data) ------------ */
/*

 > 1. index 가 0 ~ size - 1 범위를 벗어나면 안내 후 0을 반환한다.
 > 2. cur 를 head 에서 index 번 이동시켜 해당 노드에 세운다.
 > 3. 그 노드의 자료를 out 포인터에 담고 1을 반환한다.

*/
int GET_entryByIndex(LinkedList* pList, int index, int* out)
{
    if (index < 0 || index >= pList->size)
    {
        printf("잘못된 위치입니다.\n");
        return 0;
    }

    pList->cur = pList->head;
    for (int i = 0; i < index; i++)   // index 위치의 노드까지 이동
        pList->cur = pList->cur->next;

    *out = pList->cur->data;
    return 1;
    // ㄴ찾은 자료 전달
}

/* ---------- [12] 길이 반환 함수 ------------ */
/*

 > 1. 삽입, 삭제 때마다 갱신해 온 size 를 그대로 반환한다.

*/
int GET_length(LinkedList* pList)
{
    return pList->size;
}

/* ---------- [13] 리스트 출력 함수 ------------ */
/*

 > 1. 리스트가 비어 있으면 비어 있다고 출력한다.
 > 2. cur 를 head 에 놓고 끝(NULL)까지 이동하며 자료를 순서대로 출력한다.
 > 3. 마지막에 NULL 을 함께 출력해 리스트의 끝을 표시한다.

*/
void PRINT_list(LinkedList* pList)
{
    if (pList->head == NULL)
    {
        printf("리스트가 비어 있습니다.\n");
        return;
    }

    pList->cur = pList->head;
    while (pList->cur != NULL)
    {
        printf("%d -> ", pList->cur->data);
        pList->cur = pList->cur->next;
    }
    printf("NULL\n");
}

/* ---------- [14] 역순 함수 ------------ */
/*

 > 1. prev(이전 노드)를 NULL 로 두고 cur 를 head 에서 출발시킨다.
 > 2. 다음 노드를 미리 저장한 뒤, cur 의 next 를 prev 로 뒤집는다.
 > 3. prev 와 cur 를 한 칸씩 앞으로 옮기며 끝까지 반복한다.
 > 4. 원래 head 가 tail 이 되고, 마지막 prev 가 새 head 가 된다.

*/
void REVERSE(LinkedList* pList)
{
    Node* prev = NULL;

    pList->tail = pList->head;   // 원래 첫 노드가 마지막 노드가 됨
    pList->cur = pList->head;

    while (pList->cur != NULL)
    {
        Node* pNext = pList->cur->next;   // 다음 노드를 미리 저장
        pList->cur->next = prev;          // 연결 방향을 반대로 뒤집음
        prev = pList->cur;
        pList->cur = pNext;
    }

    pList->head = prev;   // 마지막으로 처리한 노드가 새 head
}

/* ---------- [15] 리스트 해제 함수 ------------ */
/*

 > 1. cur 를 head 에 놓고, 다음 노드를 미리 저장한 뒤 현재 노드를 해제한다.
 > 2. 끝(NULL)까지 반복해 모든 노드를 해제하고
 > 3. head, tail, cur 를 NULL 로, size 를 0으로 되돌린다.

*/
void FREE_list(LinkedList* pList)
{
    pList->cur = pList->head;

    while (pList->cur != NULL)
    {
        Node* pNext = pList->cur->next;   // 다음 노드를 미리 저장
        free(pList->cur);
        // ㄴ동적 할당된 노드 해제
        pList->cur = pNext;
    }

    pList->head = NULL;
    pList->tail = NULL;
    pList->cur = NULL;
    pList->size = 0;
}
