//  ROBIT C PROJECT DAY 4
//  ㄴ hw1
//  ㄴㄴ hw1.c
//
//  Created by Lee DY on 7/21/26.
//
#define SIZE 5
#include <stdio.h>

typedef struct _Student
{
    int grade;         // 학년
    double gpa;        // 성적
    char name[10];     // 이름

}Student;

/* ------------ [1] 함수 원형 선언 ------------ */

int  INPUT_int(int* out);
// ㄴ학년(정수) 입력 검사 함수
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고
//  > 2. 숫자가 아닌 문자, 빈 입력을 걸러낸다.
//  > 3. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int  INPUT_double(double* out);
// ㄴ성적(실수) 입력 검사 함수
//  > 1. 소수점 '.'을 기준으로 정수부와 소수부를 나누어 누적하고
//  > 2. 소수점 중복, 숫자가 아닌 문자, 빈 입력을 걸러낸다.
//  > 3. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int  INPUT_name(char name[], int size);
// ㄴ이름(문자열) 입력 검사 함수
//  > 1. 한 글자씩 읽어 'a'~'z', 'A'~'Z' 아스키 범위로 알파벳을 판별하고
//  > 2. 알파벳이 아닌 문자, 빈 입력을 걸러낸다.
//  > 3. 성공 시 이름을 name 배열에 담고 1을, 실패 시 0을 반환한다.

int  INPUT_Student_data(Student arr[], int size);
// ㄴ학생 정보 입력 함수
//  > 1. size 명만큼 학년, 성적, 이름 순서로 입력받고
//  > 2. 하나라도 검사에 실패하면 0을, 모두 통과하면 1을 반환한다.

void SORTING_data(Student arr[], int size);
// ㄴ정렬 함수
//  > 1. 학년 -> 성적 -> 이름 순의 우선순위로
//  > 2. 오름차순으로 정렬한다.

void OUTPUT_Student_data(Student arr[], int size);
// ㄴ결과 출력 함수
//  > 1. 정렬이 끝난 학생 정보를 한 명씩 줄바꿈으로 출력한다.

/* ---------- [2] main문 ------------ */
/*

 > 1. 학생 정보 입력 -> 정렬 -> 출력 순으로 진행하며
 > 2. 입력 단계에서 예외가 발생하면(반환값 0) 정렬, 출력 없이 즉시 종료한다.

*/
int main(void)
{
    Student list[SIZE];

    if (!INPUT_Student_data(list, SIZE))
        return 0;   // 입력 오류 시 정렬, 출력 없이 종료

    SORTING_data(list, SIZE);
    OUTPUT_Student_data(list, SIZE);

    return 0;
}

/* ---------- [1-1] 학년 입력 검사 함수 ------------ */
/*

 > 1. scanf("%c") 로 글자를 하나씩 받아 '0'~'9' 이면 숫자로 인정하고
 > 2. val = val * 10 + (ip - '0') 로 자릿수를 누적한다.
 > 3. 줄바꿈이나 공백을 만나면 한 숫자의 입력이 끝난 것으로 보고 반복을 멈춘다.
 > 4. 숫자가 아닌 문자, 빈 입력은 안내 후 0을 반환한다.
 > 5. 검사를 모두 통과하면 값을 out 에 담고 1을 반환한다.

*/
int INPUT_int(int* out)
{
    char ip;
    int val = 0;
    int count = 0;   // 읽어들인 숫자 글자 수

    while (1)
    {
        scanf("%c", &ip);

        if (ip >= '0' && ip <= '9')   // '0'~'9' 아스키 범위면 숫자로 인식
        {
            val = val * 10 + (int)(ip - '0');   // 형변환
            count++;
        }
        else if (ip == ' ' || ip == '\n')   // 줄바꿈, 띄어쓰기면 입력 종료
        {
            break;
        }
        else
        {
            printf("잘못된 입력입니다: 학년은 정수여야 합니다.\n");
            return 0;
        }
    }

    if (count == 0)   // 아무 숫자도 입력하지 않음
    {
        printf("학년이 입력되지 않았습니다.\n");
        return 0;
    }

    *out = val;
    return 1;
    // ㄴ검사를 통과한 값 전달
}

/* ---------- [1-2] 성적 입력 검사 함수 ------------ */
/*

 > 1. 소수점을 만나기 전까지는 정수부(intPart)에 자릿수를 누적하고
 > 2. 소수점 '.' 을 만나면 dotSeen 을 켜서 이후 숫자는 소수부(fracPart)로 처리한다.
 > 3. 소수부는 divisor 를 10배씩 키워 (숫자 / divisor) 를 더하는 방식으로 자릿값을 맞춘다.
 > 4. 소수점이 두 번 나오거나, 숫자가 아닌 문자, 빈 입력은 안내 후 0을 반환한다.
 > 5. 통과하면 정수부 + 소수부를 out 에 담고 1을 반환한다.

*/
int INPUT_double(double* out)
{
    char ip;
    double intPart = 0;
    double fracPart = 0;
    double divisor = 1;
    int count = 0;
    int dotSeen = 0;   // 소수점을 만났는지 여부

    while (1)
    {
        scanf("%c", &ip);

        if (ip >= '0' && ip <= '9')
        {
            if (dotSeen == 0)   // 소수점 전: 정수부 누적
            {
                intPart = intPart * 10 + (int)(ip - '0');
            }
            else                // 소수점 후: 소수부 누적
            {
                divisor = divisor * 10;
                fracPart = fracPart + (double)(ip - '0') / divisor;
            }
            count++;
        }
        else if (ip == '.')
        {
            if (dotSeen == 1)   // 소수점이 두 번 나옴
            {
                printf("잘못된 입력입니다: 소수점이 여러 개입니다.\n");
                return 0;
            }
            dotSeen = 1;
        }
        else if (ip == ' ' || ip == '\n')
        {
            break;
        }
        else
        {
            printf("잘못된 입력입니다: 성적은 실수여야 합니다.\n");
            return 0;
        }
    }

    if (count == 0)   // 아무 숫자도 입력하지 않음
    {
        printf("성적이 입력되지 않았습니다.\n");
        return 0;
    }

    *out = intPart + fracPart;
    return 1;
    // ㄴ검사를 통과한 값 전달
}

/* ---------- [1-3] 이름 입력 검사 함수 ------------ */
/*

 > 1. scanf("%c") 로 글자를 하나씩 받아 'a'~'z', 'A'~'Z' 이면 알파벳으로 인정하고
 > 2. 배열 크기를 넘지 않는 범위(size - 1)까지만 name 배열에 저장한다.
 > 3. 줄바꿈이나 공백을 만나면 입력이 끝난 것으로 보고 끝에 '\0' 을 붙인다.
 > 4. 알파벳이 아닌 문자, 빈 입력은 안내 후 0을 반환한다.

*/
int INPUT_name(char name[], int size)
{
    char ip;
    int len = 0;

    while (1)
    {
        scanf("%c", &ip);

        if ((ip >= 'a' && ip <= 'z') || (ip >= 'A' && ip <= 'Z'))   // 아스키 범위면 알파벳으로 인식
        {
            if (len < size - 1)   // '\0' 자리를 남기고 저장
            {
                name[len] = ip;
                len++;
            }
        }
        else if (ip == ' ' || ip == '\n')   // 줄바꿈, 띄어쓰기면 입력 종료
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

/* ---------- [2] 학생 정보 입력 함수 ------------ */
/*

 > 1. "입력: " 안내를 출력한 뒤
 > 2. size 명만큼 반복하며 학년, 성적, 이름을 각각의 검사 함수로 입력받는다.
 > 3. 하나라도 잘못된 입력이면 즉시 0을 반환하고,
 > 4. 모두 정상이면 배열에 저장된 채 1을 반환한다.

*/
int INPUT_Student_data(Student arr[], int size)
{
    printf("입력: ");
    for (int i = 0; i < size; i++)
    {
        if (!INPUT_int(&arr[i].grade))      // 학년: 정수 검사
            return 0;
        if (!INPUT_double(&arr[i].gpa))     // 성적: 실수 검사
            return 0;
        if (!INPUT_name(arr[i].name, 10))   // 이름: 알파벳 검사
            return 0;
    }
    return 1;
    // ㄴ모든 학생 정보를 정상적으로 입력받음
}

/* ---------- [3] 정렬 함수 ------------ */
/*

 > 1. 이웃한 두 학생을 비교해 순서가 어긋나면 교환하는 버블 정렬을 사용한다.
 > 2. 학년이 크면 뒤로 보내고,
 > 3. 학년이 같으면 성적을, 성적도 같으면 이름의 첫 글자를 비교해 교환한다.
 > 4. 한 바퀴를 도는 동안 교환이 없으면(swapped == 0) 정렬이 끝난 것으로 보고 종료한다.

*/
void SORTING_data(Student arr[], int size)
{
    Student temp;
    int swapped;   // 이번 바퀴에서 교환이 있었는지 여부

    while (1)
    {
        swapped = 0;
        for (int i = 0; i < size - 1; i++)
        {
            if (arr[i].grade > arr[i + 1].grade)   // 1순위: 학년 오름차순
            {
                temp = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = temp;
                swapped = 1;
            }

            else if (arr[i].grade == arr[i + 1].grade)
            {
                if (arr[i].gpa > arr[i + 1].gpa)   // 2순위: 성적 오름차순
                {
                    temp = arr[i];
                    arr[i] = arr[i + 1];
                    arr[i + 1] = temp;
                    swapped = 1;
                }

                else if (arr[i].gpa == arr[i + 1].gpa)
                {
                    if (arr[i].name[0] > arr[i + 1].name[0])   // 3순위: 이름 첫 글자 오름차순
                    {
                        temp = arr[i];
                        arr[i] = arr[i + 1];
                        arr[i + 1] = temp;
                        swapped = 1;
                    }
                }
            }
        }
        if (swapped == 0)   // 교환이 없으면 정렬 완료
            break;
    }
}

/* ---------- [4] 결과 출력 함수 ------------ */
/*

 > 1. 정렬이 끝난 배열을 앞에서부터 순서대로
 > 2. 학년, 성적(소수 첫째 자리), 이름 형식으로 한 줄씩 출력한다.

*/
void OUTPUT_Student_data(Student arr[], int size)
{
    printf("출력: ");
    for (int i = 0; i < size; i++)
    {
        printf("%d %.1lf %s\n", arr[i].grade, arr[i].gpa, arr[i].name);
    }
}
