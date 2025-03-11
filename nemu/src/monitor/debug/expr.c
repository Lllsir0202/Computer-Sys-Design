#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_HEX, TK_REG, TK_NEQ, TK_AND, TK_OR, 
  // 这两个token其实就是*和-，但是作出区分主要为了作为解引用和负号
  TK_DEREF, TK_NEG

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
  {"(0x|X)[0-9a-fA-F]+", TK_HEX},     // HEX_NUM
  {"[0-9]+", TK_NUM},    // NUM
  {"\\$[a-z]+", TK_REG},  // reg
  {"!", '!'},           // !
  {"&&", TK_AND},       // and
  {"||", TK_OR}         // or
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

static bool check_parentheses(int p, int q){
  // 用来判断括号是否匹配，其实就是首尾是否都是括号、内部括号是否对应
  int judge = 0;
  for(int i = p; i <= q; i++){
    if(tokens[i].type == '('){
      judge++;
    }
    if(tokens[i].type == ')'){
      judge--;
    }
    if(judge < 0){
      panic("Meet unmatched () in pairing");
      return false;
    }
  }
  if(judge != 0){
    Log("p is %d, q is %d\n", p, q);
    
    panic("Meet unmatched () in the end");
  }
  return (judge == 0 && tokens[p].type == '(' && tokens[q].type == ')');
}

// 在寻找dominate op之前，我们需要确定好op的优先级
static int get_op_prior(int type){
  // 这里我们先确定下优先级
  // 首先最高的是：括号
  // 其次是：单目运算符，右结合
  // 然后是：乘除，左结合
  // 再后是：加减，左结合

  // 在这里是越小优先级越高
  switch(type){
    case '(':
    case ')':{
      return 0;
    }
    case TK_DEREF:
    case TK_NEG:{
      return 1;
    }
    case '*':
    case '/':{
      return 2;
    }
    case '+':
    case '-':{
      return 3;
    }
    case TK_EQ:
    case TK_NEQ:{
      return 4;
    }
    case TK_AND:{
      return 5;
    }
    case TK_OR:{
      return 6;
    }
    default:{
      Log("Get op prior REACHES some strange position");
      panic("Op is %d", type);
    }
  }
}

// 寻找dominate op
static int find_dominate_op(int p, int q){
  // 可以在进入eval之前，就进行一次计算，这样就不需要处理别的情况了
  // for(int i = p ; i <= q; i++){
  //   if(tokens[i].type == '-' && (i == 0 || tokens[i-1].type == ))
  // }
  int parentheses = 0;
  int op = -1; // 这里的op表示索引，需要使用tokens[op]来访问
  int min_prior = 10; // 表示优先级，由于我们上面的实现中，是数字越小，优先级越高，所以开始选择一个较大的数字
  int cur_prior;
  for(int i = p ; i <= q; i++){
    if(tokens[i].type == '('){
      parentheses++;
    }
    if(tokens[i].type == ')'){
      parentheses--;
    }
    if(parentheses == 0){
      // 进入这个分支表示不在一个括号中
      // 下一个分支表示是非运算符，那么直接continue
      if(tokens[i].type == TK_NUM || tokens[i].type == TK_HEX || tokens[i].type == TK_REG){
        continue;
      }
      // 接下来就是基于优先级去处理了
      cur_prior = get_op_prior(tokens[i].type);
      if(min_prior >= cur_prior){
        min_prior = cur_prior;
        op = i;
      }
    }
  }
  assert(op != -1);
  return op;
}

// 通过char*得到寄存器的值
static uint32_t get_reg_value(char* reg){
  for(int i = 0; i < 8 ; i++){
    if(strcmp(reg, regsl[i]) == 0){
      return reg_l(i);
    }
    if(strcmp(reg, regsw[i]) == 0){
      return reg_w(i);
    }
    if(strcmp(reg, regsb[i]) == 0){
      return reg_b(i);
    }
  }
  if(strcmp(reg, "eip") == 0){
    return cpu.eip;
  }
  panic("Invalid register %s", reg);
}

static uint32_t eval(int p, int q){
  if (p > q) {
    /* Bad expression */
    // 这里表示出现了交错，可能情况应该是()这种
    panic("Invalid expression!");
  }
  else if (p == q) {
    // panic("here");
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
     // 表示一个token，因为我们没有别的情况，所以应该是一个num
    assert(p == q);
    int index = p;
    switch (tokens[index].type){
      case TK_NUM:{
        // 表示十进制数字
        uint32_t num =  strtoul(tokens[index].str, NULL, 10);
        Log("dec num is %d", num);
        return num;
      }
      case TK_HEX:{
        char* _noperfix = tokens[index].str + 2;
        uint32_t num =  strtoul(_noperfix, NULL, 16);
        Log("hex num is 0x%x",num);
        return num;
      }
      case TK_REG:{
        char* _noperfix = tokens[index].str + 1;
        return get_reg_value(_noperfix);
      }
      default:{
        panic("Invalid case");
      }
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    Log("p is %s, q is %s\n", tokens[p].str, tokens[q].str);
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
    // 这里其实就是作出计算的操作，一方面这里不是完整的一个括号，另一方面这里先需要知道哪个是dominate operator
    // 首先找到dominate op，然后定位前后位置
    int op = find_dominate_op(p,q);
    // 这里如果op对应的type是单目运算符，那么需要重新处理
    if(tokens[op].type == TK_DEREF || tokens[op].type == TK_NEG){
      return 0;
    }else{
      // 这个分支，表示这里是正常的计算了
      uint32_t left_val = eval(p, op-1);
      if(tokens[op].type == TK_AND){
        if(left_val == 0){
          return 0;
        }else{
          return eval(p, op);
        }
      }
      if(tokens[op].type == TK_OR){
        if(left_val == 0){
          return eval(op+1, q);
        }else{
          return 1;
        }
      }
      uint32_t right_val = eval(op+1, q);
      uint32_t result = 0;
      switch (tokens[op].type){
        case '+':{
          result = left_val + right_val;
        }break;
        case '-':{
          result = left_val - right_val;
        }break;
        case '*':{
          result = left_val * right_val;
        }break;
        case '/':{
          if(right_val != 0){
            result = left_val / right_val;
          }else{
            panic("Invalid result: the divisor is 0");
          }
        }break;
        case TK_EQ:{
          result = (left_val == right_val);
        }break;
        case TK_NEQ:{
          result = (left_val != right_val);
        }break;
      }
      return result;
    }
  }
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
    printf("str is %s\n", tokens[i].str);
    // 接下来就是需要去计算求值了
  }

  uint32_t result = eval(0,nr_token-1);
  Log("Test: Num is 0x%x",result);
  

  return result;
}
