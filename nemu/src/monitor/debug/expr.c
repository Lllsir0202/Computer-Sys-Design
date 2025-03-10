#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_HEX, TK_REG, TK_NEQ

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"-", '-'},           // sub
  {"\\*", '*'},         // mul
  {"/", '/'},           // div
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // not equal
  {"\\(", '('},         // (
  {"\\)", ')'},         // )
  {"[0-9]|[1-9][0-9]+", TK_NUM},    // NUM
  {"[0x|X][0-9a-fA-F]+", TK_HEX},     // HEX_NUM
  {"$[a-z]+", TK_REG},  // reg
  {"!", '!'}           // !
  // 对于解引用来说，我们的处理其实和乘法识别没有区别
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:{
          } break;
          case TK_NUM:
          case TK_HEX:
          case TK_REG:{
            if(substr_len > 31){
              Log("Too long expr token of \"%.*s\", len is %d", substr_len, substr_start, substr_len);
              assert(0);
            }
            int j;
            for(j = 0 ; j < substr_len; j++)
            {
              tokens[nr_token].str[j] = substr_start[j];
            }
            tokens[nr_token].str[j] = '\0';
          }
          default:{
            assert(nr_token < 32);
            tokens[nr_token].type = rules[i].token_type;
            ++nr_token;
          } break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  // 到这里说明token都成功识别了，并且在tokens数组中
  *success = true;
  int i;
  for(i = 0 ; i < nr_token; i++){
    printf("%d\n", tokens[i].type);
    // 接下来就是需要去计算求值了
    
  }


  return 0;
}
