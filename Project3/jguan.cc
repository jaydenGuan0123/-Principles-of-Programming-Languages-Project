//By Jiandong Guan
#include <iostream>
#include <vector>
#include "lexer.h"
#include "compiler.h"
#include <map>
using namespace std;
string variable[100];
LexicalAnalyzer lexer;
struct InstructionNode* parse_body();
struct InstructionNode* parse_statement_list();
struct InstructionNode* parse_statement();
struct InstructionNode* parse_assign();
struct InstructionNode* parse_output();
struct InstructionNode* parse_input();
struct InstructionNode* parse_while();
struct InstructionNode* parse_if();
struct InstructionNode* parse_switch();
struct InstructionNode* parse_for();
struct InstructionNode* parse_case_list(Token var, InstructionNode* skip);
struct InstructionNode* ope(InstructionNode* temp,Token t);
std::map<std::string,int> variable_index;
void addNUM(Token t);
void addNUM(Token t){
    if(variable_index.find(t.lexeme)==variable_index.end()) {
        variable_index[t.lexeme] = next_available;
        mem[next_available] = std::stoi(t.lexeme);
        next_available++;
    }
}
struct InstructionNode* ope(InstructionNode* temp,Token t)
{
    if (t.token_type ==  PLUS)
        temp->assign_inst.op = OPERATOR_PLUS;
    else if(t.token_type == MINUS)
        temp->assign_inst.op =  OPERATOR_MINUS;
    else if(t.token_type == MULT)
        temp->assign_inst.op =  OPERATOR_MULT;
    else if(t.token_type == DIV)
        temp->assign_inst.op =  OPERATOR_DIV;
    else if(t.token_type == LESS )
        temp->cjmp_inst.condition_op = CONDITION_LESS;
    else if(t.token_type == GREATER)
        temp->cjmp_inst.condition_op = CONDITION_GREATER;
    else if(t.token_type == NOTEQUAL)
        temp->cjmp_inst.condition_op =  CONDITION_NOTEQUAL;
    return temp;
}

struct InstructionNode * parse_generate_intermediate_representation()
{
    Token t=lexer.GetToken();
    while(t.token_type!=SEMICOLON) {
        if (t.token_type == ID) {
            variable_index[t.lexeme] = next_available;
            mem[next_available] = 0;
            next_available++;
        }
        t = lexer.GetToken();
    }
    struct InstructionNode* temp = parse_body();
    t=lexer.GetToken();
    while(t.token_type==NUM){
        inputs.push_back(std::stoi(t.lexeme));
        t=lexer.GetToken();
    }
    lexer.GetToken();//End of file
    return temp;
}

struct InstructionNode* parse_body()
{
    lexer.GetToken();//{
    struct InstructionNode* temp = parse_statement_list();
    lexer.GetToken();//}
    return temp;
}

struct InstructionNode* parse_statement_list()
{
    struct InstructionNode* inst1=parse_statement();
    struct InstructionNode* inst2;
    Token temp = lexer.peek(1);
    if (temp.token_type == INPUT || temp.token_type == OUTPUT ||temp.token_type == ID || temp.token_type == IF ||
        temp.token_type == FOR|| temp.token_type == SWITCH ||temp.token_type == WHILE){
        inst2 = parse_statement_list();
        struct InstructionNode* temp1 = inst1;
        while(temp1->next != NULL)
            temp1 = temp1->next;
        temp1->next = inst2;
    }
    return inst1;
}

struct InstructionNode* parse_statement()
{
    struct InstructionNode* start = new InstructionNode;
    Token temp = lexer.peek(1);
    if(temp.token_type == ID)
        start = parse_assign();

    else if(temp.token_type == OUTPUT)
        start= parse_output();

    else if(temp.token_type == INPUT)
        start= parse_input();

    else if(temp.token_type == WHILE)
    {//fix order cause problem here
        start = parse_while();
        start->next = parse_body();

        struct InstructionNode* jump = new InstructionNode;
        jump->type = JMP;
        jump->jmp_inst.target = start;

        struct InstructionNode* temp1 = start->next;
        while(temp1->next != NULL)
            temp1 = temp1->next;
        temp1->next = jump;
        start->cjmp_inst.target = jump;

        struct InstructionNode* skip = new InstructionNode;
        skip->type = NOOP;
        skip->next = NULL;
        jump->next = skip;
        start->cjmp_inst.target = skip;
    }

    else if(temp.token_type == IF)
        start = parse_if();

    else if(temp.token_type == SWITCH)
        start= parse_switch();

    else if(temp.token_type == FOR)
        start= parse_for();

    return start;
}

struct InstructionNode* parse_input()
{
    struct InstructionNode* input = new InstructionNode;
    lexer.GetToken();//input
    Token temp = lexer.GetToken();
    input->type = IN;
    input->input_inst.var_index = variable_index[temp.lexeme];
    lexer.GetToken();//;
    input->next = NULL;
    return input;
}

struct InstructionNode* parse_output()
{
    struct InstructionNode* output = new InstructionNode;
    lexer.GetToken();//output
    Token temp = lexer.GetToken();
    output->type = OUT;
    output->output_inst.var_index = variable_index[temp.lexeme];
    lexer.GetToken();//;
    output->next = NULL;
    return output;
}

struct InstructionNode* parse_assign()
{
    Token lhs = lexer.GetToken();
    lexer.GetToken();//=

    struct InstructionNode* temp = new InstructionNode;
    temp->type = ASSIGN;
    temp->assign_inst.left_hand_side_index = variable_index[lhs.lexeme];
    Token t=lexer.GetToken();
    Token t1=lexer.GetToken();
    if(t.token_type==NUM){
        addNUM(t);
    }
    temp->assign_inst.operand1_index=variable_index[t.lexeme];
    if(t1.token_type!=SEMICOLON){
        temp=ope(temp,t1);
        t=lexer.GetToken();//NUM OR Variable
        lexer.GetToken();//;
        if(t.token_type==NUM) {
            addNUM(t);
        }
        temp->assign_inst.operand2_index=variable_index[t.lexeme];
    }
    else{
        temp->assign_inst.op=OPERATOR_NONE;
    }
    temp->next = NULL;
    return temp;
}

struct InstructionNode* parse_if()
{
    struct InstructionNode* temp = new InstructionNode;
    struct InstructionNode* skip = new InstructionNode;
    lexer.GetToken();//if
    temp->type = CJMP;
    Token t=lexer.GetToken();
    temp->cjmp_inst.operand1_index = variable_index[t.lexeme];
    t=lexer.GetToken();
    temp=ope(temp,t);
    t=lexer.GetToken();
    if(t.token_type==NUM) {
        addNUM(t);
    }
    temp->cjmp_inst.operand2_index = variable_index[t.lexeme];
    temp->next = parse_body();
    skip->type = NOOP;
    skip->next = NULL;
    struct InstructionNode* temp1 = temp->next;
    while(temp1->next != NULL)
        temp1 = temp1->next;
    temp1->next = skip;
    temp->cjmp_inst.target = skip;
    return temp;
}

struct InstructionNode* parse_while()
{
    struct InstructionNode* temp = new InstructionNode;
    lexer.GetToken();//while
    temp->type = CJMP;
    Token t=lexer.GetToken();
    temp->cjmp_inst.operand1_index = variable_index[t.lexeme];
    t=lexer.GetToken();
    temp=ope(temp,t);
    t=lexer.GetToken();
    if(t.token_type==NUM) {
        addNUM(t);
    }
    temp->cjmp_inst.operand2_index = variable_index[t.lexeme];
    return temp;
}

struct InstructionNode* parse_for()
{
    struct InstructionNode* jump = new InstructionNode;
    struct InstructionNode* skip = new InstructionNode;
    struct InstructionNode* start;
    struct InstructionNode* temp2;
    struct InstructionNode* temp = new InstructionNode;
    jump->type = JMP;
    jump->next = skip;
    skip->type = NOOP;
    skip->next = NULL;

    lexer.GetToken();//for
    lexer.GetToken();//(
    start  = parse_assign();
    start ->next = temp;

    temp->type = CJMP;
    Token t=lexer.GetToken();
    temp->cjmp_inst.operand1_index = variable_index[t.lexeme];
    t=lexer.GetToken();
    temp=ope(temp,t);
    t=lexer.GetToken();
    if(t.token_type==NUM) {
        addNUM(t);
    }
    temp->cjmp_inst.operand2_index = variable_index[t.lexeme];
    temp->cjmp_inst.target = skip;
    lexer.GetToken();//;
    temp2 = parse_assign();
    temp2->next = jump;
    lexer.GetToken();//)

    temp->next = parse_body();
    struct InstructionNode* temp1 = temp->next;
    while(temp1->next != NULL)
        temp1 = temp1->next;
    temp1->next = temp2;
    jump->jmp_inst.target = temp;

    return start;
}

struct InstructionNode* parse_switch()
{
    struct InstructionNode* skip = new InstructionNode;
    skip->type = NOOP;
    skip->next = NULL;
    struct InstructionNode* start;
    lexer.GetToken();//switch
    Token var = lexer.GetToken();
    lexer.GetToken();//{
    start = parse_case_list(var, skip);

    Token temp = lexer.peek(1);
    if(temp.token_type == DEFAULT){
        struct InstructionNode* def = new InstructionNode;
        lexer.GetToken();//default
        lexer.GetToken();//:
        def = parse_body();//connect switch body to default
        struct InstructionNode* temp1 = start;
        while(temp1->next != NULL)
            temp1 = temp1->next;
        temp1->next = def;
        temp1 = def;
        while(temp1->next != NULL)
            temp1 = temp1->next;
        temp1->next = skip;
    }
    else if(temp.token_type == RBRACE){
        struct InstructionNode* temp1 = start;
        while(temp1->next != NULL)
            temp1 = temp1->next;
        temp1->next = skip;
    }

    lexer.GetToken();//}
    return start;
}

struct InstructionNode* parse_case_list(Token var, InstructionNode* skip)
{
    struct InstructionNode* inst1 = new InstructionNode;
    struct InstructionNode* inst2 = new InstructionNode;
    lexer.GetToken();//case
    Token num = lexer.GetToken();
    addNUM(num);
    inst1->type = CJMP;
    inst1->cjmp_inst.operand1_index = variable_index[var.lexeme];
    inst1->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    inst1->cjmp_inst.operand2_index = variable_index[num.lexeme];
    inst1->next = NULL;
    lexer.GetToken();//:

    inst1->cjmp_inst.target = parse_body();
    struct InstructionNode* temp1 = inst1->cjmp_inst.target;
    while(temp1->next != NULL)
        temp1 = temp1->next;
    temp1->next = skip;

    Token temp = lexer.peek(1);
    if(temp.token_type == CASE)
    {
        inst2 = parse_case_list(var, skip);
        temp1 = inst1;
        while(temp1->next != NULL)
            temp1 = temp1->next;
        temp1->next = inst2;
    }

    return inst1;
}
