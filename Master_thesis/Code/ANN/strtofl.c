#include<stdio.h>
float a2f(char *);

int main() {
  char ch[20];
  float i;
  printf("\n enter the number as a string\n");
  scanf("%[^\n]", ch);
  i = my_a2f(ch);
  printf(" string =%s , float =%g \n", ch, i);
  return 0;
}

float a2f(char *p) {
  // here i took another two   variables for counting the number of digits in mantissa
  int i, num = 0, num2 = 0, pnt_seen = 0, x = 0, y = 1; 
  float f1, f2, f3;
  for (i = 0; p[i]; i++)
    if (p[i] == '.') {
      pnt_seen = i;
      break;
    }
  for (i = 0; p[i]; i++) {
    if (i < pnt_seen) num = num * 10 + (p[i] - 48);
    else if (i == pnt_seen) continue;
    else {
      num2 = num2 * 10 + (p[i] - 48);
      ++x;
    }
  }
  // it takes 10 if it has 1 digit ,100 if it has 2 digits in mantissa
  for (i = 1; i <= x; i++) 
    y = y * 10;
  f2 = num2 / (float) y;
  f3 = num + f2;
  return f3;
}