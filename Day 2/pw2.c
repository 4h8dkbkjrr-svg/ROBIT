#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

void INPUT_arr(void);       // 배열에 값을 입력받는 함수
void PRINT_arr_now(void);   // 현재 배열 상태를 한 줄로 출력하는 함수
void OUTPUT_arr(void);      // 정렬이 끝난 배열을 인덱스와 함께 출력하는 함수
int  SORTING_arr(int i);    // 인덱스 i에 최댓값을 배치(교환)하는 함수, 교환했으면 1 / 안 했으면 0 반환
int  FIND_max_loc(int i);   // arr[0]~arr[i] 구간에서 최댓값이 있는 위치(인덱스)를 찾는 함수

int arr[8];   // 정렬 대상 배열 (전역 변수라 모든 함수에서 바로 접근 가능)

int main(void)
{
    INPUT_arr();       // 사용자로부터 정수 8개를 입력받아 arr에 저장
    PRINT_arr_now();    // 정렬 시작 전 초기 상태 출력

    // i = 7부터 0까지, 매 반복마다 arr[0..i] 구간의 최댓값을 arr[i] 자리로 보냄
    // (선택 정렬: 뒤에서부터 한 자리씩 확정해 나가는 방식)
    for (int i = 7; i >= 0; i--)
    {
        if (SORTING_arr(i))    // 실제로 교환이 일어난 경우에만 (1을 반환한 경우)
        {
            PRINT_arr_now();   // 중간 과정으로 배열 상태 출력
            // 값이 바뀌지 않은 pass(이미 제자리인 경우, 0 반환)는
            // 출력하지 않으므로 동일한 배열 상태가 중복 출력되지 않음
        }
    }

    OUTPUT_arr();   // 최종 정렬 결과 출력

    return 0;
}

void INPUT_arr(void)
{
    printf("입력\n\n");

    for (int i = 0; i < 8; i++)
    {
        printf("%d : ", i);
        scanf("%d", &arr[i]);   // 인덱스 i에 값 입력
        printf("\n");
    }
}

void OUTPUT_arr(void)
{
    printf("\n\n출력\n\n");

    for (int i = 0; i < 8; i++)
    {
        // "인덱스 : 값" 형태로 한 번에 출력 (줄바꿈 두 번으로 한 줄씩 띄움)
        printf("%d : %d\n\n", i, arr[i]);
    }
}

void PRINT_arr_now(void)
{
    printf("\n");
    for (int i = 0; i < 8; i++)
    {
        printf("%d  ", arr[i]);   // 배열 전체를 한 줄로 나열
    }
    printf("\n");
}

int SORTING_arr(int i)
{
    int max_loc = FIND_max_loc(i);   // arr[0..i] 구간에서 최댓값의 위치를 한 번의 순회로 찾음

    if (max_loc == i)   // 최댓값이 이미 arr[i] 자리에 있다면
    {
        return 0;        // 교환할 필요 없음 -> 변경 없음을 알림 (main에서 출력 생략)
    }

    // 최댓값(arr[max_loc])과 현재 arr[i]를 맞교환
    int temp = arr[i];
    arr[i] = arr[max_loc];
    arr[max_loc] = temp;

    return 1;   // 실제로 값이 바뀌었음을 알림 (main에서 출력하도록)
}

int FIND_max_loc(int i)
{
    int max_loc = 0;   // 일단 arr[0]을 최댓값 위치로 가정

    // j = 1부터 i까지 비교하면서, 더 큰 값을 발견할 때마다 위치 갱신
    // (값과 위치를 따로 두 번 찾지 않고, 한 번의 순회로 위치만 바로 구함)
    for (int j = 1; j <= i; j++)
    {
        if (arr[j] > arr[max_loc])
        {
            max_loc = j;
        }
    }

    return max_loc;   // 최댓값이 있던 인덱스 반환
}
