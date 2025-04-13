#include <stdio.h>

int main(){
  FILE *fp = fopen("/dev/events", "r");
  volatile int j = 0;
  while(1){
    j ++;
    if (j == 1000000) {
      char buf[256];
      char *p = buf, ch;
      printf("hello");
      while((ch = fgetc(fp)) != -1){
        printf("hello2");
        *p ++ = ch;
        if(ch == '\n') {
          *p = '\0';
          break;
        }
      }

      printf("receive event: %s", buf);
      j = 0;
    }
  }

  fclose(fp);
  return 0;
}

