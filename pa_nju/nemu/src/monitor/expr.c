#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

#define test_mode false

#define double_operand_helper \
    val1 = eval(s, oper - 1, success); \
    val2 = eval(oper + 1, e, success); \
    if(test_mode) printf("val1 = %d, val2 = %d \n", val1, val2);
    
#define single_operand_helper \
    val2 = eval(oper + 1, e, success); \
    if(test_mode) printf("val2 = %d \n", val2);

#define is_oper(x) \
    (x) == DOR || (x) == DAND || \
    (x) == '|' || (x) == '^' || (x) == '&' || \
    (x) == EQ || (x) == NEQ || \
    (x) == '<' || (x) == '>' || (x) == LE || (x) == GE || \
    (x) == SHL || (x) == SHR || \
    (x) == '+' || (x) == '-' || \
    (x) == '*' || (x) == '/' || (x) == '%' || \
    (x) == MINUS || (x) == '!' || (x) == DEREF
    
#define is_parentheses(x) \
    (x) == '(' || (x) == ')' || \
    (x) == '[' || (x) == ']' || \
    (x) == '{' || (x) == '}'
    
#define is_left_parentheses(x) \
    (x) == '(' || \
    (x) == '[' || \
    (x) == '{'
    
#define is_right_parentheses(x) \
    (x) == ')' || \
    (x) == ']' || \
    (x) == '}'

#define set_precedence(x, pre) \
    switch(x){ \
        case DOR: pre = 0; break; \
        case DAND: pre = 1; break; \
        case '|': pre = 2; break; \
        case '^': pre = 3; break; \
        case '&': pre = 4; break; \
        case EQ: case NEQ: pre = 5; break; \
        case '<': case '>': case LE: case GE: pre = 6; break; \
        case SHL: case SHR: pre = 7; break; \
        case '+': case '-': pre = 8; break; break; \
        case '*': case '/': case '%': pre = 9; break; \
        case MINUS: case '!': case DEREF: pre = 10; break; \
        default: \
            printf("Error: Bad expr. \n"); \
            *success = false; \
    }

enum
{
	NOTYPE = 256,
	EQ,
	NEQ,
	DOR, DAND,
	LE, GE,
	SHL, SHR,
	MINUS, DEREF,
	NUM,
	HEXNUM,
	REG,
	SYMB
};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{"0[xX][0-9a-fA-F]{1,8}", HEXNUM}, // 0x12345678
	{"[0-9]{1,10}", NUM}, // 0123456789
	{" +", NOTYPE}, // white space
	{"\\|\\|", DOR},
	{"\\&\\&", DAND},
	{"\\=\\=", EQ},
	{"\\!\\=", NEQ},
	{"<\\=", LE},
	{">\\=", GE},
	{"<<", SHL},
	{">>", SHR},
	{"\\+", '+'},
	{"\\-", '-'},
	{"\\*", '*'},
	{"\\/", '/'},
	{"\\%", '%'},
	{"\\&", '&'},
	{"\\|", '|'},
	{"\\^", '^'},
	{"\\(", '('},
	{"\\[", '['},
	{"\\{", '{'},
	{"\\)", ')'},
	{"\\]", ']'},
	{"\\}", '}'},
	{">", '>'},
	{"<", '<'},
	{"\\!", '!'},
	{"[a-zA-Z\\_]+", SYMB}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

                if(test_mode)
				    printf("match regex[%d] at position %d with len %d: %.*s \n", i, position, substr_len, substr_len, substr_start);
				position += substr_len; 

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */

				switch (rules[i].token_type)
				{
				case NUM:
				    memset(tokens[nr_token].str, 0, 32 * sizeof(char));
				    strncpy(tokens[nr_token].str, substr_start, (substr_len > 31) ? 31 : substr_len);
				    tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				    break;
				case HEXNUM:
				    memset(tokens[nr_token].str, 0, 32 * sizeof(char));
				    strncpy(tokens[nr_token].str, substr_start + 2, (substr_len > 31) ? 31 : (substr_len - 2));
				    tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				    break;
				case SYMB:
				    memset(tokens[nr_token].str, 0, 32 * sizeof(char));
				    strncpy(tokens[nr_token].str, substr_start, (substr_len > 31) ? 31 : (substr_len));
				    tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				    break;
				case NOTYPE:
				    break;
				default:
					tokens[nr_token].type = rules[i].token_type;
					nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

inline bool check_parentheses(int s, int e){
    return (tokens[s].type == '(' && tokens[e].type == ')') ||
           (tokens[s].type == '[' && tokens[e].type == ']') ||
           (tokens[s].type == '{' && tokens[e].type == '}');
}

int search_oper(int s, int e, bool *success){
    int i;
    int res = -1;
    int pre_pre = 10, pre_now = 10;
    int flag_parentheses = 0;
    for(i = s; i <= e; i ++){
        if((is_oper(tokens[i].type)) && (flag_parentheses == 0)){
            if(res == -1){
                res = i;
                set_precedence(tokens[i].type, pre_pre)
            }   
            else{
                set_precedence(tokens[i].type, pre_now)
                if(pre_now <= pre_pre){
                    res = i;
                    pre_pre = pre_now;
                }
            }
        }
        else if(is_left_parentheses(tokens[i].type)){
            ++flag_parentheses;
        }
        else if(is_right_parentheses(tokens[i].type)){
            if(flag_parentheses > 0){
                --flag_parentheses;
            }
            else{
                // printf("Error: Wrong parentheses matching. \n");
                *success = false;
                return 0;
            }
        }
    }
    if(res == -1 || flag_parentheses != 0){
        // printf("Error: Bad search. \n");
        *success = false;
        return 0;
    }
    else return res;
}

uint32_t look_up_symtab(char *sym, bool *success);

uint32_t eval(int s, int e, bool *success){
    int len;
    uint32_t res;
    int i;
    int oper;
    uint32_t val1, val2;
    if(!*success){
        return 0;
    }
    else if(s > e){
        // printf("Error: Bad eval. \n");
        *success = false;
        // fflush(stdout);
        return 0;
    }
    else if(s == e){
        switch(tokens[s].type){
            case NUM:
                res = 0;
                len = strlen(tokens[s].str);
                for(i = 0; i < len; i ++){
                    res *= 10;
                    res += tokens[s].str[i] - '0';
                }
                return res;
            case HEXNUM:
                res = 0;
                len = strlen(tokens[s].str);
                for(i = 0; i < len; i ++){
                    res <<= 4;
                    if(tokens[s].str[i] >= '0' && tokens[s].str[i] <= '9') res += tokens[s].str[i] - '0';
                    else if(tokens[s].str[i] >= 'A' && tokens[s].str[i] <= 'F') res += tokens[s].str[i] - 'A' + 10;
                    else res += tokens[s].str[i] - 'a' + 10;
                }
                return res;
            case SYMB:
                return look_up_symtab(tokens[s].str, success);
            default:
                // printf("Error: Bad eval. \n");
                *success = false;
                return 0;
        }
    }
    else if(check_parentheses(s, e) == true){
        return eval(s + 1, e - 1, success);
    }
    else{
        oper = search_oper(s, e, success);
        switch(tokens[oper].type){
            case '+':
                double_operand_helper
                return val1 + val2;
            case '-':
                double_operand_helper
                return val1 - val2;
            case '*':
                double_operand_helper
                return val1 * val2;
            case '/':
                double_operand_helper
                return val1 / val2;
            case '%':
                double_operand_helper
                return val1 % val2;
            case '&':
                double_operand_helper
                return val1 & val2;
            case '|':
                double_operand_helper
                return val1 | val2;
            case '^':
                double_operand_helper
                return val1 ^ val2;
            case DOR:
                double_operand_helper
                return val1 || val2;
            case DAND:
                double_operand_helper
                return val1 && val2;
            case EQ:
                double_operand_helper
                return val1 == val2;
            case NEQ:
                double_operand_helper
                return val1 != val2;
            case '<':
                double_operand_helper
                return val1 < val2;
            case '>':
                double_operand_helper
                return val1 > val2;
            case LE:
                double_operand_helper
                return val1 <= val2;
            case GE:
                double_operand_helper
                return val1 >= val2;
            case SHL:
                double_operand_helper
                return val1 << val2;
            case SHR:
                double_operand_helper
                return val1 >> val2;
            case '!':
                single_operand_helper
                return !val2;
            case DEREF:
                single_operand_helper
                return vaddr_read(val2, SREG_DS, 4);
            case MINUS:
                single_operand_helper
                return - val2;
            default:
                // printf("Error: Bad expr. \n");
                *success = false;
                return 0;
        }
    }
}

uint32_t expr(char *e, bool *success)
{
    int i;
    *success = true;
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}
/* TODO: Implement code to evaluate the expression. */
    for(i = 0; i < nr_token; i ++) {
        if(tokens[i].type == '*' && (i == 0 || is_oper(tokens[i - 1].type) || is_parentheses(tokens[i - 1].type))) {
            tokens[i].type = DEREF;
        }
        if(tokens[i].type == '-' && (i == 0 || is_oper(tokens[i - 1].type) || is_parentheses(tokens[i - 1].type))) {
            tokens[i].type = MINUS;
        }
    }
    // printf("nr_token = %d \n", nr_token);
	return eval(0, nr_token - 1, success);
}
