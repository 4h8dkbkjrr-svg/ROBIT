//  ROBIT C DAY 6
//  ㄴ hw5
//  ㄴㄴ hw5.c
//
//  <AI 사용 안내>
//  > 1. Claude 사용 원칙 및 사용 과정은 폴더에 들어있는 README.md 파일을 통해 투명하게 공개됩니다.
//  > 2. 과제 5의 경우, 코드 제작 대부분의 과정에서 Claude가 개입했습니다.
//  > 3. 특히 정렬 기능을 구현하는데 어려움이 매우 커서, Claude의 도움이 필수적이라고 판단, Claude로 초안을 작성하고 최대한 로직을 이해하려 했습니다.
//
//  Created by Lee DY on 7/23/26. Xcode

#include <stdio.h>
#include <stdlib.h>

#define MAX_STUDENT 100   // 출석부에 담을 수 있는 최대 학생 수
#define STR_LEN     30    // 이름, 주소 각 항목의 최대 길이
#define FILE_NAME   "attendance.txt"

// 주소는 나라, 도, 시, 구 네 항목으로 나누어 저장한다.
typedef struct _STUDENT
{
    int  number;              // 번호
    char name[STR_LEN];       // 이름
    char country[STR_LEN];    // 주소 - 나라
    char province[STR_LEN];   // 주소 - 도
    char city[STR_LEN];       // 주소 - 시
    char district[STR_LEN];   // 주소 - 구
    int  grade;               // 성적 (0 ~ 100)

}STUDENT;

/* ------------ [1] 함수 원형 선언 ------------ */

int  INPUT_command(void);
// ㄴ명령어 문자열을 입력받아 대응되는 번호를 반환하는 함수

int  INPUT_int(int* out);
// ㄴ정수 입력 검사 함수
//  > 1. 한 글자씩 읽어 '0'~'9' 아스키 범위로 숫자를 판별하고 맨 앞 '-'는 음수로 인정한다.
//  > 2. 성공 시 out 에 담아 1을, 잘못된 입력이면 안내 후 0을 반환한다.

int  str_equal(const char* a, const char* b);
// ㄴ두 문자열이 같은지 비교하는 함수

int  str_compare(const char* a, const char* b);
// ㄴ두 문자열의 사전순 앞뒤를 비교하는 함수
//  > 1. 같으면 0, a 가 앞서면 음수, b 가 앞서면 양수를 반환한다.

int  ADD_student(void);
// ㄴ학생 1명의 정보를 입력받아 출석부에 추가하는 함수

int  DELETE_student(void);
// ㄴ번호로 학생을 찾아 삭제하는 함수 (중복 시 선택 삭제)

int  FIND_student(void);
// ㄴ번호, 주소(나라/도/시/구), 성적 중 하나로 학생을 찾아 이름을 출력하는 함수

int  SORT_students(void);
// ㄴ기준(번호/이름/주소 각 항목/성적)을 골라 정렬해 출력하는 함수

int  COMPARE_student(const STUDENT* a, const STUDENT* b, int key);
// ㄴ정렬 기준(key)에 따라 두 학생의 앞뒤를 비교하는 함수

void PRINT_one(const STUDENT* s);
// ㄴ학생 1명의 정보를 한 줄로 출력하는 함수

void PRINT_all(void);
// ㄴ출석부의 모든 학생을 출력하는 함수

void SAVE_file(void);
// ㄴ출석부를 파일에 저장하는 함수 (파일 입출력)

void LOAD_file(void);
// ㄴ파일에서 출석부를 불러오는 함수 (파일 입출력)

// 구조체 배열로 출석부를 관리한다. studentCount 는 현재 학생 수.
STUDENT students[MAX_STUDENT];
int     studentCount = 0;

/* ---------- [2] main문 ------------ */
/*

 > 1. 명령어를 입력받아 알맞은 기능 함수를 반복 호출한다.
 > 2. 입력 항목에서 잘못된 값이 들어오면(반환 0) 안내 후 프로그램을 종료한다.
 > 3. stop 이면 반복을 벗어나 종료한다.

*/
int main(void)
{
    printf("=== 출석부 프로그램 ===\n");

    while (1)
    {
        int cmd = INPUT_command();

        if (cmd == 1)
        {
            if (ADD_student() == 0)
                break;
        }
        else if (cmd == 2)
        {
            if (DELETE_student() == 0)
                break;
        }
        else if (cmd == 3)
        {
            if (FIND_student() == 0)
                break;
        }
        else if (cmd == 4)
        {
            if (SORT_students() == 0)
                break;
        }
        else if (cmd == 5)
            PRINT_all();
        
        else if (cmd == 6)
            SAVE_file();
        
        else if (cmd == 7)
            LOAD_file();
        
        else if (cmd == 8)
            break;   // stop
        
        else
            printf("명령어가 잘못되었습니다.\n");
    }

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
    char cmd_add[]   = "add";     // 1
    char cmd_del[]   = "delete";  // 2
    char cmd_find[]  = "find";    // 3
    char cmd_sort[]  = "sort";    // 4
    char cmd_print[] = "print";   // 5
    char cmd_save[]  = "save";    // 6
    char cmd_load[]  = "load";    // 7
    char cmd_stop[]  = "stop";    // 8

    char input[20] = "";

    printf("\n명령어 (add, delete, find, sort, print, save, load, stop): ");
    scanf("%19s", input);

    if (input[0] == '\0')
        return 8;

    if      (str_equal(input, cmd_add)   == 1)
        return 1;
    
    else if (str_equal(input, cmd_del)   == 1)
        return 2;
    
    else if (str_equal(input, cmd_find)  == 1)
        return 3;
    
    else if (str_equal(input, cmd_sort)  == 1)
        return 4;
    
    else if (str_equal(input, cmd_print) == 1)
        return 5;
    
    else if (str_equal(input, cmd_save)  == 1)
        return 6;
    
    else if (str_equal(input, cmd_load)  == 1)
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

/* ---------- [6] 문자열 사전순 비교 함수 ------------ */
/*

 > 1. 같은 자리 글자를 앞에서부터 비교한다.
 > 2. 다른 글자를 만나면 두 글자의 아스키 값 차이를 반환한다.
 > 3. 끝까지 같으면 0을 반환한다. (정렬에서 오름차순 기준으로 사용)

*/
int str_compare(const char* a, const char* b)
{
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
            return (int)a[i] - (int)b[i];

        i++;
    }

    return (int)a[i] - (int)b[i];
}

/* ---------- [7] 학생 추가 함수 ------------ */
/*

 > 1. 출석부가 가득 찼으면 안내한다.
 > 2. 번호, 이름, 주소(나라/도/시/구), 성적을 입력받는다.
 > 3. 정수 항목(번호, 성적)은 INPUT_int 로 검사하고 성적은 0~100 범위를 확인한다.
 > 4. 모두 통과하면 배열에 추가하고 학생 수를 늘린다.

*/
int ADD_student(void)
{
    if (studentCount >= MAX_STUDENT)
    {
        printf("출석부가 가득 찼습니다.\n");
        return 1;   // 추가만 못 할 뿐 프로그램은 계속 진행
    }

    STUDENT* s = &students[studentCount];   // 새로 채울 자리

    printf("번호를 입력하세요: ");
    if (INPUT_int(&(s -> number)) == 0)
        return 0;

    printf("이름을 입력하세요: ");
    scanf("%29s", s -> name);

    printf("나라를 입력하세요: ");
    scanf("%29s", s -> country);

    printf("도를 입력하세요: ");
    scanf("%29s", s -> province);

    printf("시를 입력하세요: ");
    scanf("%29s", s -> city);

    printf("구를 입력하세요: ");
    scanf("%29s", s -> district);

    printf("성적을 입력하세요 (0~100): ");
    if (INPUT_int(&(s -> grade)) == 0)
        return 0;

    if (s -> grade < 0 || s -> grade > 100)
    {
        printf("성적은 0~100 사이여야 합니다.\n");
        return 0;
    }

    studentCount = studentCount + 1;
    printf("학생을 추가했습니다.\n");
    return 1;
}

/* ---------- [8] 학생 삭제 함수 ------------ */
/*

 > 1. 삭제할 번호를 입력받아 일치하는 학생들의 위치를 모은다.
 > 2. 없으면 안내하고, 1명이면 바로 삭제한다.
 > 3. 여러 명이면 목록을 보여주고 선택한 1명을 삭제한다.
 > 4. 삭제는 뒤 원소들을 한 칸씩 당겨 덮어쓰는 방식으로 한다.

*/
int DELETE_student(void)
{
    int target;
    int matchIndex[MAX_STUDENT];   // 일치하는 학생들의 배열 위치
    int matchCount = 0;
    int i;
    int delPos;

    printf("삭제할 학생의 번호를 입력하세요: ");
    if (INPUT_int(&target) == 0)
        return 0;

    for (i = 0; i < studentCount; i++)
    {
        if (students[i].number == target)
        {
            matchIndex[matchCount] = i;
            matchCount = matchCount + 1;
        }
    }

    if (matchCount == 0)
    {
        printf("번호 %d 인 학생이 없습니다.\n", target);
        return 1;
    }

    if (matchCount == 1)
    {
        delPos = matchIndex[0];
    }
    else   // 중복: 목록을 보여주고 선택받는다
    {
        int choice;
        printf("번호 %d 인 학생이 %d 명 있습니다.\n", target, matchCount);
        for (i = 0; i < matchCount; i++)
        {
            printf("  [%d] ", i + 1);
            PRINT_one(&students[matchIndex[i]]);
        }

        printf("삭제할 학생의 순번을 입력하세요 (1~%d): ", matchCount);
        if (INPUT_int(&choice) == 0)
            return 0;

        if (choice < 1 || choice > matchCount)
        {
            printf("순번이 범위를 벗어났습니다.\n");
            return 1;
        }
        delPos = matchIndex[choice - 1];
    }

    printf("삭제: ");
    PRINT_one(&students[delPos]);

    for (i = delPos; i < studentCount - 1; i++)
        students[i] = students[i + 1];   // 뒤 원소를 한 칸씩 당긴다

    studentCount = studentCount - 1;
    return 1;
}

/* ---------- [9] 학생 찾기 함수 ------------ */
/*

 > 1. 찾기 기준(번호/나라/도/시/구/성적)을 고른다.
 > 2. 기준에 맞는 모든 학생의 이름을 출력한다.
 > 3. 하나도 없으면 안내한다.

*/
int FIND_student(void)
{
    int  key;
    int  i;
    int  found = 0;
    int  targetNum = 0;
    char targetStr[STR_LEN] = "";

    printf("찾기 기준 (1:번호 2:나라 3:도 4:시 5:구 6:성적): ");
    if (INPUT_int(&key) == 0)
        return 0;

    if (key == 1 || key == 6)   // 정수 기준
    {
        printf("찾을 값을 입력하세요: ");
        if (INPUT_int(&targetNum) == 0)
            return 0;
    }
    else if (key >= 2 && key <= 5)   // 문자열 기준
    {
        printf("찾을 값을 입력하세요: ");
        scanf("%29s", targetStr);
    }
    else
    {
        printf("기준이 잘못되었습니다.\n");
        return 1;
    }

    printf("--- 찾기 결과 ---\n");
    for (i = 0; i < studentCount; i++)
    {
        int hit = 0;

        if      (key == 1 && students[i].number == targetNum)
            hit = 1;
        
        else if (key == 6 && students[i].grade == targetNum)
            hit = 1;
        
        else if (key == 2 && str_equal(students[i].country,  targetStr) == 1)
            hit = 1;
        
        else if (key == 3 && str_equal(students[i].province, targetStr) == 1)
            hit = 1;
        
        else if (key == 4 && str_equal(students[i].city,     targetStr) == 1)
            hit = 1;
        
        else if (key == 5 && str_equal(students[i].district, targetStr) == 1)
            hit = 1;

        if (hit == 1)
        {
            printf("%s\n", students[i].name);
            found = found + 1;
        }
    }

    if (found == 0)
        printf("해당하는 학생이 없습니다.\n");

    return 1;
}

/* ---------- [10] 정렬 기준 비교 함수 ------------ */
/*

 > 1. key 에 따라 번호, 성적은 숫자 차이로 비교한다.
 > 2. 이름, 나라, 도, 시, 구는 str_compare 로 사전순 비교한다.
 > 3. a 가 앞서면 음수, b 가 앞서면 양수, 같으면 0을 반환한다.

*/
int COMPARE_student(const STUDENT* a, const STUDENT* b, int key)
{
    if (key == 1)
        return a -> number - b -> number;
    
    if (key == 7)
        return a -> grade - b -> grade;

    if (key == 2)
        return str_compare(a -> name, b -> name);
    
    if (key == 3)
        return str_compare(a -> country, b -> country);
    
    if (key == 4)
        return str_compare(a -> province, b -> province);
    if (key == 5)
        
        return str_compare(a -> city, b -> city);

    return str_compare(a -> district, b -> district);   // key == 6
}

/* ---------- [11] 학생 정렬 함수 ------------ */
/*

 > 1. 정렬 기준을 고른다. (번호/이름/나라/도/시/구/성적)
 > 2. 선택 정렬로 오름차순 정렬한 뒤 전체를 출력한다.

*/
int SORT_students(void)
{
    int key;
    int i, j, min;

    printf("정렬 기준 (1:번호 2:이름 3:나라 4:도 5:시 6:구 7:성적): ");
    if (INPUT_int(&key) == 0)
        return 0;

    if (key < 1 || key > 7)
    {
        printf("기준이 잘못되었습니다.\n");
        return 1;
    }

    // 선택 정렬: 매 회 가장 앞서는 원소를 골라 앞쪽으로 보낸다.
    for (i = 0; i < studentCount - 1; i++)
    {
        min = i;
        for (j = i + 1; j < studentCount; j++)
        {
            if (COMPARE_student(&students[j], &students[min], key) < 0)
                min = j;
        }
        if (min != i)
        {
            STUDENT tmp = students[i];
            students[i] = students[min];
            students[min] = tmp;
        }
    }

    printf("--- 정렬 결과 ---\n");
    PRINT_all();
    return 1;
}

/* ---------- [12] 학생 1명 출력 함수 ------------ */
/*

 > 1. 번호, 이름, 주소(나라,도,시,구), 성적을 한 줄로 출력한다.

*/
void PRINT_one(const STUDENT* s)
{
    printf("번호:%d 이름:%s 주소:%s,%s,%s,%s 성적:%d\n",
           s -> number, s -> name,
           s -> country, s -> province, s -> city, s -> district,
           s -> grade);
}

/* ---------- [13] 전체 출력 함수 ------------ */
/*

 > 1. 학생이 없으면 안내한다.
 > 2. 모든 학생을 한 명씩 출력한다.

*/
void PRINT_all(void)
{
    int i;

    if (studentCount == 0)
    {
        printf("출석부가 비어 있습니다.\n");
        return;
    }

    for (i = 0; i < studentCount; i++)
    {
        printf("[%d] ", i + 1);
        PRINT_one(&students[i]);
    }
}

/* ---------- [14] 파일 저장 함수 ------------ */
/*

 > 1. 파일을 쓰기 모드로 연다. 실패하면 안내한다.
 > 2. 학생 수를 먼저 쓰고, 학생마다 한 줄씩 정보를 기록한다.
 > 3. 스트림을 닫는다.

*/
void SAVE_file(void)
{
    int   i;
    FILE* fp = fopen(FILE_NAME, "w");

    if (fp == NULL)
    {
        printf("파일을 열 수 없습니다.\n");
        return;
    }

    fprintf(fp, "%d\n", studentCount);
    for (i = 0; i < studentCount; i++)
    {
        fprintf(fp, "%d %s %s %s %s %s %d\n",
                students[i].number, students[i].name,
                students[i].country, students[i].province,
                students[i].city, students[i].district,
                students[i].grade);
    }

    fclose(fp);
    printf("출석부를 %s 에 저장했습니다. (%d명)\n", FILE_NAME, studentCount);
}

/* ---------- [15] 파일 불러오기 함수 ------------ */
/*

 > 1. 파일을 읽기 모드로 연다. 실패하면 안내한다.
 > 2. 저장된 학생 수를 읽고, 그 수만큼 학생 정보를 배열에 채운다.
 > 3. 스트림을 닫는다.

*/
void LOAD_file(void)
{
    int   i;
    int   count;
    FILE* fp = fopen(FILE_NAME, "r");

    if (fp == NULL)
    {
        printf("파일을 열 수 없습니다. (저장된 출석부가 없을 수 있습니다.)\n");
        return;
    }

    // 맨 앞의 학생 수를 읽는다. 형식이 맞지 않으면 중단한다.
    if (fscanf(fp, "%d", &count) != 1)
    {
        printf("파일 형식이 올바르지 않습니다.\n");
        fclose(fp);
        return;
    }

    if (count > MAX_STUDENT)
        count = MAX_STUDENT;   // 배열 크기를 넘지 않도록 제한한다

    studentCount = 0;
    for (i = 0; i < count; i++)
    {
        // 한 명분 7개 항목을 읽는다. 개수가 맞지 않으면 중단한다.
        if (fscanf(fp, "%d %29s %29s %29s %29s %29s %d",
                   &students[i].number, students[i].name,
                   students[i].country, students[i].province,
                   students[i].city, students[i].district,
                   &students[i].grade) != 7)
            break;

        studentCount = studentCount + 1;
    }

    fclose(fp);
    printf("출석부를 불러왔습니다. (%d명)\n", studentCount);
}
