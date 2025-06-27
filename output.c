#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// Testing variable declarations
int x = 42;
bool y = true;
char z = 'Z';
char s[] = "Line1\nLine2\tTabbed";
int arr[3] = { 10, 20, 30 };
// Testing function with parameters and return
int sum(int a, int b) {
    int result = a + b;
    return result;
}

/* test for multi
line comment */
int power(int a, int b) {
    int result = (int)pow(a, 2) + (int)pow(b, 3);
    return result;
}

// Function using boolean and character logic
bool match_char(char* s, char c) {
    int i = 0;
    while (s[i] != '\0') {
    if (s[i] == c) {
    return true;
    }
    i = i + 1;
    }
    return false;
}

int main() {
    printf("Testing basic types:\n\n");
    printf("x =%d\n\n", x);
    printf("y =%d\n\n", y);
    printf("z =%c\n\n", z);
    printf("s =%s\n\n", s);
    printf("Array contents:\n\n");
    int i = 0;
    while (i < 3) {
    printf("%d \n", arr[i]);
    i = i + 1;
}

printf("\n\n");
int a = 5;
int b = 2;
int c  = 1;
printf("Sum of %d and %d is %d%d\n\n", a, b, sum(a, b));
printf("%dto the power of%dis %d%d\n\n", a, b, power(a, b));
char txt[] = "banana";
char ch = 'a';
printf("Does \"%s\" contain '%c'? %d%d\n\n", txt, ch, match_char(txt, ch));
return 0;
}
