//  ROBIT C PROJECT DAY 5
//  ㄴ hw3
//  ㄴㄴ hw3.c
//
//  Created by Lee DY on 7/23/26. Xcode
//

#include <stdio.h>
#include <stdlib.h>

/* ------------ [1] 함수 원형 선언 ------------ */

int INPUT_natural(int* out);
// ㄴ자연수 입력 검사 함수
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고
//  > 2. 실수(.), 음수(-), 문자, 빈 입력, 0을 각각 걸러낸다.
//  > 3. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int** MALLOC_p2DArr(int row, int col);
// ㄴ2차원 동적 할당 함수
//  > 1. 행의 개수(row)만큼 행 포인터 배열을 할당하고
//  > 2. 각 행마다 열의 개수(col)만큼 int 배열을 할당한다.
//  > 3. 중간에 실패하면 이미 할당한 행들을 해제하고 NULL 을, 성공하면 주소를 반환한다.

void arr_ij(int* sizeRow, int* sizeCol, int** pArr);
// ㄴ달팽이 채우기 함수
//  > 1. 왼쪽 위(1, 1)부터 시계 방향으로 안쪽으로 돌며 1부터 차례로 수를 채운다.
//  > 2. 위 행(왼->오), 오른쪽 열(위->아래), 아래 행(오->왼), 왼쪽 열(아래->위) 순으로 진행한다.
//  > 3. 채운 결과를 pArr 에 저장한다.

void print(int* row, int* col, int** pArr);
// ㄴ출력 함수
//  > 1. row x col 크기의 2차원 배열을 오른쪽 정렬(폭 4)로 한 행씩 출력한다.

void FREE_p2DArr(int** pArr, int row);
// ㄴ2차원 동적 할당 해제 함수
//  > 1. 각 행을 먼저 해제한 뒤 행 포인터 배열을 해제한다.

/* ---------- [2] main문 ------------ */
/*

 > 1. 열의 수, 행의 수를 입력받아 각각 자연수인지 검사하고,
 > 2. 그 크기로 2차원 배열을 동적 할당한다.
 > 3. arr_ij 로 달팽이 모양을 채운 뒤 print 로 출력한다.
 > 4. 입력이 잘못되면(반환값 0) 즉시 종료하고, 할당에 실패하면(NULL) 그 즉시 종료한다.
 > 5. 사용이 끝나면 FREE_p2DArr 로 모든 메모리를 해제한다.

*/
int main(void)
{
    int sizeRow, sizeCol;
    int row, col;

    printf("열의 수를 입력하세요: ");
    if (INPUT_natural(&sizeCol) == 0)
        return 0;
    // ㄴ열의 수 입력이 잘못되면 종료한다.

    printf("행의 수를 입력하세요: ");
    if (INPUT_natural(&sizeRow) == 0)
        return 0;
    // ㄴ행의 수 입력이 잘못되면 종료한다.

    row = sizeRow;
    col = sizeCol;

    int** arr = MALLOC_p2DArr(row, col);
    if (arr == NULL)
        return 0;
    // ㄴ메모리 할당 실패 시 종료한다.(안내 문구는 MALLOC_p2DArr 에서 이미 출력됨)

    arr_ij(&sizeRow, &sizeCol, arr);   // 달팽이 모양으로 채움
    print(&row, &col, arr);            // 결과 출력

    FREE_p2DArr(arr, row);
    // ㄴ동적 할당된 2차원 메모리 해제

    return 0;
}

/* ---------- [1] 자연수 입력 검사 함수 ------------ */
/*

 > 1. scanf("%c") 로 글자를 하나씩 받아 '0'~'9' 이면 숫자로 인정하고
 > 2. value = value * 10 + (ip - '0') 로 자릿수를 누적한다.
 > 3. 줄바꿈이나 공백을 만나면 한 숫자의 입력이 끝난 것으로 보고 반복을 멈춘다.
 > 4. 실수(.), 음수(-), 그 외 문자, 빈 입력, 0은 자연수가 아니므로 안내 후 0을 반환한다.
 > 5. 검사를 모두 통과하면 값을 out 에 담고 1을 반환한다.

*/
int INPUT_natural(int* out)
{
    char ip;
    int value = 0;
    int count = 0;

    while (1)
    {
        scanf("%c", &ip);

        if (ip >= '0' && ip <= '9')   // '0'~'9' 아스키 범위면 숫자로 인식
        {
            value = value * 10 + (int)(ip - '0');   // 형변환
            count++;
        }
        else if (ip == '\n' || ip == ' ')   // 줄바꿈, 띄어쓰기면 입력 종료
        {
            break;
        }
        else
        {
            if (ip == '.')
                printf("실수는 입력할 수 없습니다.\n");
            else if (ip == '-')
                printf("음수는 자연수가 아닙니다.\n");
            else
                printf("잘못된 입력입니다.\n");
            return 0;
        }
    }

    if (count == 0)    // 아무 숫자도 입력하지 않음
    {
        printf("입력이 없습니다.\n");
        return 0;
    }
    if (value == 0)    // 0은 자연수가 아님
    {
        printf("0은 자연수가 아닙니다.\n");
        return 0;
    }

    *out = value;
    return 1;
    // ㄴ검사를 통과한 값 전달
}

/* ---------- [2] 2차원 동적 할당 함수 ------------ */
/*

 > 함수는 int형 배열들의 주소를 담는 배열, 즉 int** 를 반환한다.
 > 1. 먼저 sizeof(int*) * row 크기로 행 포인터 배열을 할당한다.
 > 2. 각 행마다 sizeof(int) * col 크기로 한 줄씩 할당한다.
 > 3. 도중에 실패(NULL)하면 그때까지 할당한 행들과 행 포인터 배열을 해제하고 NULL 을 반환한다.
 > 4. 모두 성공하면 할당한 주소를 반환한다.

*/
int** MALLOC_p2DArr(int row, int col)
{
    int** pArr = (int**)malloc(sizeof(int*) * row);

    if (pArr == NULL)
    {
        printf("메모리 할당에 실패했습니다.\n");
        return NULL;
    }
    // ㄴ실패: NULL 반환

    for (int i = 0; i < row; i++)
    {
        pArr[i] = (int*)malloc(sizeof(int) * col);

        if (pArr[i] == NULL)
        {
            printf("메모리 할당에 실패했습니다.\n");

            for (int k = 0; k < i; k++)   // 이미 할당한 행들을 해제
                free(pArr[k]);
            free(pArr);

            return NULL;
        }
    }

    return pArr;
    // ㄴ성공: 할당한 주소 반환
}

/* ---------- [3] 달팽이 채우기 함수 ------------ */
/*

 > 위쪽(top), 아래쪽(bottom), 왼쪽(left), 오른쪽(right) 네 경계를 두고
 > 경계 안쪽을 시계 방향으로 돌며 1부터 차례로 채운 뒤 경계를 한 칸씩 좁힌다.
 > 1. 위 행을 왼쪽에서 오른쪽으로 채우고 top 을 한 칸 내린다.
 > 2. 오른쪽 열을 위에서 아래로 채우고 right 를 한 칸 당긴다.
 > 3. (아직 행이 남았으면) 아래 행을 오른쪽에서 왼쪽으로 채우고 bottom 을 한 칸 올린다.
 > 4. (아직 열이 남았으면) 왼쪽 열을 아래에서 위로 채우고 left 를 한 칸 민다.
 > 5. 경계가 서로 엇갈릴 때까지 반복하면 안쪽까지 달팽이 모양이 채워진다.

*/
void arr_ij(int* sizeRow, int* sizeCol, int** pArr)
{
    int row = *sizeRow;
    int col = *sizeCol;

    int top = 0;
    int bottom = row - 1;
    int left = 0;
    int right = col - 1;
    int num = 1;   // 채워 넣을 수

    while (top <= bottom && left <= right)
    {
        for (int j = left; j <= right; j++)   // 위 행: 왼 -> 오
            pArr[top][j] = num++;
        top++;

        for (int i = top; i <= bottom; i++)   // 오른쪽 열: 위 -> 아래
            pArr[i][right] = num++;
        right--;

        if (top <= bottom)   // 아직 채울 아래 행이 남은 경우
        {
            for (int j = right; j >= left; j--)   // 아래 행: 오 -> 왼
                pArr[bottom][j] = num++;
            bottom--;
        }

        if (left <= right)   // 아직 채울 왼쪽 열이 남은 경우
        {
            for (int i = bottom; i >= top; i--)   // 왼쪽 열: 아래 -> 위
                pArr[i][left] = num++;
            left++;
        }
    }
}

/* ---------- [4] 출력 함수 ------------ */
/*

 > 1. 포인터로 받은 행(*row), 열(*col) 크기만큼 이중 반복하며
 > 2. 각 원소를 폭 4의 오른쪽 정렬(%4d)로 출력해 자릿수를 맞춘다.
 > 3. 한 행을 출력할 때마다 줄을 바꾼다.

*/
void print(int* row, int* col, int** pArr)
{
    for (int i = 0; i < *row; i++)
    {
        for (int j = 0; j < *col; j++)
        {
            printf("%4d", pArr[i][j]);
        }
        printf("\n");
    }
}

/* ---------- [5] 2차원 동적 할당 해제 함수 ------------ */
/*

 > 1. 각 행(pArr[i])을 먼저 해제한 뒤
 > 2. 행 포인터 배열(pArr) 자체를 해제한다.

*/
void FREE_p2DArr(int** pArr, int row)
{
    for (int i = 0; i < row; i++)
    {
        free(pArr[i]);
        // ㄴ각 행의 동적 할당 메모리 해제
    }
    free(pArr);
    // ㄴ행 포인터 배열 해제
}
