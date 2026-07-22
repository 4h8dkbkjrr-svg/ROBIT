//  ROBIT C PROJECT DAY 4
//  ㄴ hw4
//  ㄴㄴ hw4.c
//
//  Created by Lee DY on 7/22/26.
//

#include <stdio.h>
#include <stdlib.h>

typedef struct _Date
{
    int year;    // 연
    int month;   // 월
    int day;     // 일
} Date;

typedef struct _Time
{
    int hour;   // 시
    int min;    // 분
    int sec;    // 초
} Time;

typedef struct _Timestamp
{
    Date date;   // 날짜 구조체를 멤버로 갖는 중첩 구조체
    Time time;   // 시각 구조체를 멤버로 갖는 중첩 구조체
} Timestamp;

/* ------------ [1] 함수 원형 선언 ------------ */

int INPUT_natural(int* out);
// ㄴ자연수 입력 검사 함수
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고
//  > 2. 실수(.), 음수(-), 문자, 빈 입력, 0을 각각 걸러낸다.
//  > 3. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int INPUT_time(int* out);
// ㄴ시각 입력 검사 함수
//  > 1. 자연수 검사와 같은 방식으로 한 글자씩 읽어 숫자를 판별하되
//  > 2. 시, 분, 초는 0도 정상값이므로 0을 허용한다.
//  > 3. 성공 시 값을 out 포인터에 담고 1을, 실패 시 0을 반환한다.

int INPUT_timestamp(Timestamp* pTimestamp);
// ㄴ날짜, 시각 입력 함수
//  > 1. 연 월 일 시 분 초 여섯 값을 순서대로 입력받고
//  > 2. 연, 월, 일은 자연수로, 시, 분, 초는 0을 허용하는 검사기로 검사한다.
//  > 3. 월(1~12), 일(1~31), 시(0~23), 분(0~59), 초(0~59) 범위를 벗어나면 걸러낸다.
//  > 4. 성공 시 값을 pTimestamp 포인터에 담고 1을, 실패 시 0을 반환한다.

int IS_leapYear(int year);
// ㄴ윤년 판별 함수
//  > 1. 4의 배수이면서 100의 배수가 아니거나, 400의 배수이면 윤년으로 보고
//  > 2. 윤년이면 1을, 아니면 0을 반환한다.

int CALC_days(Date date);
// ㄴ날짜 -> 일수 변환 함수
//  > 1. 1년 1월 1일부터 해당 날짜까지 지난 일수를 계산해 반환한다.
//  > 2. 윤년인 해는 366일, 윤년의 2월은 29일로 계산한다.

int CALC_seconds(Time time);
// ㄴ시각 -> 초 변환 함수
//  > 1. 하루 안에서 0시 0분 0초부터 지난 초를 계산해 반환한다.

void CALC_diff(Timestamp first, Timestamp second, int* pHour, int* pMin, int* pSec);
// ㄴ차이 계산 함수
//  > 1. 두 시점을 일수와 초로 바꿔 차이를 구하고
//  > 2. 그 차이를 시, 분, 초로 환산해 포인터 인자로 돌려준다.

void PRINT_result(int hour, int min, int sec);
// ㄴ결과 출력 함수
//  > 1. 두 시점의 차이를 시, 분, 초 형식으로 출력한다.

/* ---------- [2] main문 ------------ */
/*

 > 1. 두 개의 날짜, 시각 입력 -> 차이 계산 -> 출력 순으로 진행하며
 > 2. 입력 단계에서 예외가 발생하면(반환값 0) 그 즉시 종료한다.

*/
int main(void)
{
    Timestamp first, second;

    printf("입력: ");
    if (INPUT_timestamp(&first) == 0)
        return 0;
    // ㄴ첫 번째 날짜, 시각 입력이 잘못되면 종료한다.

    if (INPUT_timestamp(&second) == 0)
        return 0;
    // ㄴ두 번째 날짜, 시각 입력이 잘못되면 종료한다.

    int hour, min, sec;
    CALC_diff(first, second, &hour, &min, &sec);

    PRINT_result(hour, min, sec);

    return 0;
}

/* ---------- [1-1] 자연수 입력 검사 함수 ------------ */
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

/* ---------- [1-2] 시각 입력 검사 함수 ------------ */
/*

 > 1. 자연수 검사와 같은 방식으로 scanf("%c") 로 글자를 하나씩 받아 자릿수를 누적한다.
 > 2. 시, 분, 초는 0시 0분 0초처럼 0도 정상값이므로 0을 거부하지 않는다.
 > 3. 실수(.), 음수(-), 그 외 문자, 빈 입력은 안내 후 0을 반환한다.
 > 4. 검사를 모두 통과하면 값을 out 에 담고 1을 반환한다.

*/
int INPUT_time(int* out)
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
                printf("음수는 입력할 수 없습니다.\n");
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

    *out = value;   // 시, 분, 초는 0도 그대로 저장
    return 1;
}

/* ---------- [2] 날짜, 시각 입력 함수 ------------ */
/*

 > 1. 연, 월, 일은 0이 될 수 없으므로 INPUT_natural 로 검사하고
 > 2. 시, 분, 초는 0을 허용하는 INPUT_time 으로 검사한다.
 > 3. 하나라도 잘못된 입력이면 즉시 0을 반환한다.
 > 4. 월(1~12), 일(1~31), 시(0~23), 분(0~59), 초(0~59) 범위를 벗어나면 안내 후 0을 반환한다.
 > 5. 모두 통과하면 값이 pTimestamp 에 담긴 채 1을 반환한다.

*/
int INPUT_timestamp(Timestamp* pTimestamp)
{
    if (INPUT_natural(&pTimestamp->date.year) == 0)    // 연: 자연수
        return 0;
    if (INPUT_natural(&pTimestamp->date.month) == 0)   // 월: 자연수
        return 0;
    if (INPUT_natural(&pTimestamp->date.day) == 0)     // 일: 자연수
        return 0;
    if (INPUT_time(&pTimestamp->time.hour) == 0)       // 시: 0 허용
        return 0;
    if (INPUT_time(&pTimestamp->time.min) == 0)        // 분: 0 허용
        return 0;
    if (INPUT_time(&pTimestamp->time.sec) == 0)        // 초: 0 허용
        return 0;

    if (pTimestamp->date.month > 12)   // 월 범위 검사
    {
        printf("월은 1~12 사이여야 합니다.\n");
        return 0;
    }
    if (pTimestamp->date.day > 31)     // 일 범위 검사
    {
        printf("일은 1~31 사이여야 합니다.\n");
        return 0;
    }
    if (pTimestamp->time.hour > 23)    // 시 범위 검사
    {
        printf("시는 0~23 사이여야 합니다.\n");
        return 0;
    }
    if (pTimestamp->time.min > 59)     // 분 범위 검사
    {
        printf("분은 0~59 사이여야 합니다.\n");
        return 0;
    }
    if (pTimestamp->time.sec > 59)     // 초 범위 검사
    {
        printf("초는 0~59 사이여야 합니다.\n");
        return 0;
    }

    return 1;
    // ㄴ여섯 값 모두 검사를 통과함
}

/* ---------- [3] 윤년 판별 함수 ------------ */
/*

 > 1. 4의 배수이면서 100의 배수가 아니면 윤년이고
 > 2. 400의 배수이면 예외적으로 윤년이다.
 > 3. 윤년이면 1을, 아니면 0을 반환한다.

*/
int IS_leapYear(int year)
{
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
        return 1;

    return 0;
}

/* ---------- [4] 날짜 -> 일수 변환 함수 ------------ */
/*

 > math.h 없이 정수 연산만으로 날짜 차이를 구하기 위해,
 > 날짜를 "1년 1월 1일부터 지난 일수" 하나로 바꾼다.
 > 1. 1년부터 (해당 연도 - 1)년까지 윤년이면 366일, 아니면 365일을 더한다.
 > 2. 1월부터 (해당 월 - 1)월까지 각 달의 일수를 더한다.
 > 3. 윤년이면서 3월 이후라면 2월이 29일이므로 하루를 더한다.
 > 4. 마지막으로 (일 - 1)을 더해 총 일수를 반환한다.

*/
int CALC_days(Date date)
{
    int daysInMonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int days = 0;

    for (int y = 1; y < date.year; y++)
    {
        if (IS_leapYear(y))
            days = days + 366;
        else
            days = days + 365;
    }

    for (int m = 1; m < date.month; m++)
    {
        days = days + daysInMonth[m];
    }

    if (date.month > 2 && IS_leapYear(date.year))
        days = days + 1;   // 윤년이면 2월이 29일

    days = days + (date.day - 1);

    return days;
    // ㄴ1년 1월 1일부터 지난 일수 반환
}

/* ---------- [5] 시각 -> 초 변환 함수 ------------ */
/*

 > 1. 하루 안에서의 위치를 초 단위 하나로 바꾼다.
 > 2. 시 * 3600 + 분 * 60 + 초 를 계산해 반환한다.

*/
int CALC_seconds(Time time)
{
    return time.hour * 3600 + time.min * 60 + time.sec;
    // ㄴ0시 0분 0초부터 지난 초 반환
}

/* ---------- [6] 차이 계산 함수 ------------ */
/*

 > 1. 두 시점을 각각 (일수, 하루 안의 초)로 바꿔 차이를 구한다.
 > 2. 첫 시점이 더 늦으면 부호를 뒤집어 항상 양수 차이로 만든다.
 > 3. 초 차이가 음수이면 하루(86400초)를 빌려오고 일수 차이에서 하루를 뺀다.
 > 4. 일수 차이 * 24 + (초 차이 / 3600) 으로 시를,
 >    나머지 초를 60으로 나눠 분과 초를 구해 포인터 인자로 돌려준다.

*/
void CALC_diff(Timestamp first, Timestamp second, int* pHour, int* pMin, int* pSec)
{
    int dayDiff = CALC_days(second.date) - CALC_days(first.date);
    int secDiff = CALC_seconds(second.time) - CALC_seconds(first.time);

    // 첫 시점이 더 늦은 경우: 부호를 뒤집어 양수 차이로 만든다
    if (dayDiff < 0 || (dayDiff == 0 && secDiff < 0))
    {
        dayDiff = -dayDiff;
        secDiff = -secDiff;
    }

    if (secDiff < 0)   // 시각이 앞서면 하루를 빌려온다
    {
        dayDiff = dayDiff - 1;
        secDiff = secDiff + 86400;
    }

    *pHour = dayDiff * 24 + secDiff / 3600;
    *pMin = (secDiff % 3600) / 60;
    *pSec = secDiff % 60;
    // ㄴ계산한 시, 분, 초 전달
}

/* ---------- [7] 결과 출력 함수 ------------ */
/*

 > 1. 두 시점의 차이를 시, 분, 초 형식으로 출력한다.

*/
void PRINT_result(int hour, int min, int sec)
{
    printf("출력: %d시 %d분 %d초\n", hour, min, sec);
}
