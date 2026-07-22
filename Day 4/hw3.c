//  ROBIT C PROJECT DAY 4
//  ㄴ hw3
//  ㄴㄴ hw3.c
//
//  Created by Lee DY on 7/21/26.
//

#include <stdlib.h>
#include <stdio.h>

typedef struct _Item
{
    char name[10];   // 물품 이름
    int price;       // 물품 금액
} Item;

/* ------------ [1] 함수 원형 선언 ------------ */

int INPUT_int(int* out);
// ㄴ자연수 입력 검사 함수
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고
//  > 2. 숫자가 아닌 문자, 빈 입력, 0을 걸러낸다.
//  > 3. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int INPUT_name(char name[], int size);
// ㄴ이름 입력 검사 함수
//  > 1. 한 글자씩 읽어 'a'~'z', 'A'~'Z' 아스키 범위로 알파벳을 판별하고
//  > 2. 알파벳이 아닌 문자, 빈 입력, 배열 크기를 넘는 이름을 걸러낸다.
//  > 3. 성공 시 이름을 name 배열에 담고 1을, 실패 시 0을 반환한다.

int INPUT_num(int* out);
// ㄴ물품 개수 입력 함수
//  > 1. 사용자로부터 물품의 개수를 입력받고
//  > 2. 개수는 자연수여야 하므로 INPUT_int 로 검사한다.
//  > 3. 성공 시 개수를 out 에 담고 1을, 실패 시 0을 반환한다.

Item* MALLOC_pItemlist(int num);
// ㄴ동적 할당 함수
//  > 1. 물품의 개수에 기반한 크기만큼 주소를 동적 할당하며
//  > 2. 해당 주소를 반환한다.

int INPUT_item(Item plist[], int num);
// ㄴ물품 목록 입력 함수
//  > 1. num 개만큼 물품의 이름과 금액을 입력받고
//  > 2. 검사를 통과한 값만 동적 할당한 주소에 저장하며
//  > 3. 성공 시 1을, 입력이 잘못되면 0을 반환한다.

int CALC_BuyItemPrice(Item plist[], int num, char BuyItem[]);
// ㄴ금액 합계 계산 함수
//  > 1. 구매할 물품 이름(BuyItem)과 목록의 각 물품 이름을 직접 비교하고
//  > 2. 이름이 같은 물품의 금액을 모두 더해 반환한다.

void OUTPUT_BuyItemPrice(int BuyItemPrice);
// ㄴ결과 출력 함수
//  > 1. 계산된 금액의 합계를 출력한다.

/* ---------- [2] main문 ------------ */
/*

 > 1. 개수 입력 -> 메모리 할당 -> 물품 입력 -> 구매할 이름 입력 -> 계산 -> 출력 순으로 진행하며
 > 2. 각 단계에서 예외가 발생하면(반환값 0 또는 NULL) 그 즉시 종료한다.
 > 3. 메모리 할당 이후 단계에서 실패하면 이미 할당한 메모리를 먼저 해제한 뒤 종료한다.

*/
int main(void)
{
    int num;
    if (INPUT_num(&num) == 0)      // 개수 입력이 잘못되면 종료
    {
        return 0;
    }

    Item* plist = MALLOC_pItemlist(num);
    if (plist == NULL)             // 메모리 할당 실패하면 종료
    {
        return 0;
    }
    // ㄴ안내 문구는 MALLOC_pItemlist 에서 이미 출력됨

    if (INPUT_item(plist, num) == 0)   // 물품 입력이 잘못되면
    {
        free(plist);                   // 이미 할당한 메모리 정리 후 종료
        return 0;
    }

    char BuyItem[10];
    if (INPUT_name(BuyItem, 10) == 0)  // 구매할 이름이 잘못되면
    {
        free(plist);
        return 0;
    }

    int BuyItemPrice = CALC_BuyItemPrice(plist, num, BuyItem);
    OUTPUT_BuyItemPrice(BuyItemPrice);

    free(plist);
    // ㄴ사용이 끝난 동적 할당 메모리 해제

    return 0;
}

/* ---------- [1-1] 자연수 입력 검사 함수 ------------ */
/*

 > 1. scanf("%c") 로 글자를 하나씩 받아 '0'~'9' 이면 숫자로 인정하고
 > 2. value = value * 10 + (ip - '0') 로 자릿수를 누적한다.
 > 3. 줄바꿈이나 공백을 만나면 한 숫자의 입력이 끝난 것으로 보고 반복을 멈춘다.
 > 4. 숫자가 아닌 문자, 빈 입력, 0은 안내 후 0을 반환한다.
 > 5. 검사를 모두 통과하면 값을 out 에 담고 1을 반환한다.

*/
int INPUT_int(int* out)
{
    char ip;
    int value = 0;
    int count = 0;   // 읽어들인 숫자 글자 수

    while (1)
    {
        scanf("%c", &ip);

        // 아스키로 '0'(48) ~ '9'(57) 안이면 숫자
        if (ip >= '0' && ip <= '9')
        {
            value = value * 10 + (int)(ip - '0');   // 형변환으로 자릿수 누적
            count = count + 1;
        }
        else if (ip == ' ' || ip == '\n')   // 구분자를 만나면 끝
        {
            break;
        }
        else
        {
            printf("잘못된 입력입니다: 숫자만 입력하세요.\n");
            return 0;
        }
    }

    if (count == 0)   // 숫자가 하나도 안 들어옴
    {
        printf("숫자가 입력되지 않았습니다.\n");
        return 0;
    }

    if (value == 0)   // 0은 개수, 금액으로 부적절
    {
        printf("0보다 큰 수를 입력하세요.\n");
        return 0;
    }

    *out = value;
    return 1;
    // ㄴ검사를 통과한 값 전달
}

/* ---------- [1-2] 이름 입력 검사 함수 ------------ */
/*

 > 1. scanf("%c") 로 글자를 하나씩 받아 'a'~'z', 'A'~'Z' 이면 알파벳으로 인정하고
 > 2. '\0' 자리를 남기기 위해 size - 1 개까지만 name 배열에 저장한다.
 > 3. 자리가 꽉 찼는데 글자가 더 들어오면 이름이 너무 긴 것이므로 안내 후 0을 반환한다.
 > 4. 줄바꿈이나 공백을 만나면 입력이 끝난 것으로 보고 끝에 '\0' 을 붙인다.
 > 5. 알파벳이 아닌 문자, 빈 입력은 안내 후 0을 반환한다.

*/
int INPUT_name(char name[], int size)
{
    char ip;
    int len = 0;

    while (1)
    {
        scanf("%c", &ip);

        // 아스키로 'a'~'z' 또는 'A'~'Z' 이면 알파벳
        if ((ip >= 'a' && ip <= 'z') || (ip >= 'A' && ip <= 'Z'))
        {
            if (len < size - 1)   // '\0' 자리를 남기고 저장
            {
                name[len] = ip;
                len = len + 1;
            }
            else   // 자리가 꽉 찼는데 글자가 더 들어옴
            {
                printf("잘못된 입력입니다: 이름이 너무 깁니다.\n");
                return 0;
            }
        }
        else if (ip == ' ' || ip == '\n')   // 구분자를 만나면 끝
        {
            break;
        }
        else
        {
            printf("잘못된 입력입니다: 이름은 알파벳만 가능합니다.\n");
            return 0;
        }
    }

    name[len] = '\0';   // 문자열의 끝 표시

    if (len == 0)   // 이름이 하나도 안 들어옴
    {
        printf("이름이 입력되지 않았습니다.\n");
        return 0;
    }

    return 1;
    // ㄴ검사를 통과한 이름 전달
}

/* ---------- [2] 물품 개수 입력 함수 ------------ */
/*

 > 1. "입력: " 안내를 출력한 뒤
 > 2. 개수는 1개 이상이어야 하므로 INPUT_int 로 검사한다.
 > 3. 통과하면 개수가 out 에 담긴 채 1을, 실패하면 0을 반환한다.

*/
int INPUT_num(int* out)
{
    printf("입력: ");
    return INPUT_int(out);   // 개수는 자연수
}

/* ---------- [3] 동적 할당 함수 ------------ */
/*

 > 동적 할당 함수는 Item형의 주소를 반환하므로, Item* 로 선언한다.
 > 1. sizeof(Item) * num 크기만큼 malloc 으로 할당하고
 > 2. 할당에 실패(NULL)하면 안내를 출력한 뒤 NULL 을 반환하며,
 > 3. 성공하면 할당한 주소를 반환한다.

*/
Item* MALLOC_pItemlist(int num)
{
    Item* pItem = (Item*)malloc(sizeof(Item) * num);

    if (pItem == NULL)
    {
        printf("메모리 할당에 실패했습니다.\n");
        return NULL;
    }
    // ㄴ실패: NULL 반환

    return pItem;
    // ㄴ성공: 할당한 주소 반환
}

/* ---------- [4] 물품 목록 입력 함수 ------------ */
/*

 > 1. num 번 반복하며 물품의 이름을 INPUT_name 으로, 금액을 INPUT_int 로 검사한다.
 > 2. 하나라도 잘못된 입력이면 즉시 0을 반환하고,
 > 3. 모두 정상이면 동적 할당한 배열에 저장한 뒤 1을 반환한다.

*/
int INPUT_item(Item plist[], int num)
{
    for (int i = 0; i < num; i++)
    {
        if (INPUT_name(plist[i].name, 10) == 0)   // 이름 먼저
        {
            return 0;
        }
        if (INPUT_int(&plist[i].price) == 0)      // 그다음 금액
        {
            return 0;
        }
    }
    return 1;
    // ㄴ모든 물품을 정상적으로 입력받음
}

/* ---------- [5] 금액 합계 계산 함수 ------------ */
/*

 > string.h 를 쓰지 않으므로 문자열 비교를 반복문으로 직접 구현한다.
 > 1. 목록의 각 물품마다 isName 을 1(같다고 가정)로 두고 비교를 시작한다.
 > 2. 두 이름 중 하나라도 '\0' 에 도달하지 않은 동안 한 글자씩 비교하며,
 > 3. 다른 글자가 나오면 isName 을 0으로 바꾸고 비교를 멈춘다.
 >    (길이가 다르면 한쪽만 '\0' 이라 다른 글자로 판정되어 자연스럽게 걸러진다)
 > 4. 끝까지 같으면(isName == 1) 그 물품의 금액을 합계에 더한다.

*/
int CALC_BuyItemPrice(Item plist[], int num, char BuyItem[])
{
    int BuyItemPrice = 0;

    for (int i = 0; i < num; i++)
    {
        int isName = 1;   // 같다고 가정
        int j = 0;        // 아이템마다 0부터 비교

        while (BuyItem[j] != '\0' || plist[i].name[j] != '\0')
        {
            if (BuyItem[j] != plist[i].name[j])   // 다른 글자 발견
            {
                isName = 0;
                break;
            }
            j++;
        }

        if (isName == 1)   // 이름이 같을 때만 금액을 더함
        {
            BuyItemPrice = BuyItemPrice + plist[i].price;
        }
    }
    return BuyItemPrice;
    // ㄴ이름이 일치한 물품 금액의 합계 반환
}

/* ---------- [6] 결과 출력 함수 ------------ */
/*

 > 1. 이름이 일치한 물품 금액의 합계를 출력한다.
 > 2. 일치한 물품이 없으면 0이 그대로 출력된다.

*/
void OUTPUT_BuyItemPrice(int BuyItemPrice)
{
    printf("출력: %d\n", BuyItemPrice);
}
