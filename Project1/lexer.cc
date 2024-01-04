/*
 * Copyright (C) Rida Bazzi
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE",
                      "IF", "WHILE", "DO", "THEN", "PRINT",
                      "PLUS", "MINUS", "DIV", "MULT",
                      "EQUAL", "COLON", "COMMA", "SEMICOLON",
                      "LBRAC", "RBRAC", "LPAREN", "RPAREN",
                      "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
                      "DOT", "NUM", "ID", "ERROR" ,"REALNUM","BASE08NUM","BASE16NUM"
};

#define KEYWORDS_COUNT 5
string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT" };
    std::vector<char> input_store;
    #define CHARWORDS_COUNT 5
    char charword[] = {'A','B','C','D','E' ,'F'};

void Token::Print()
{
    cout << "{" << this->lexeme << " , "
         << reserved[(int) this->token_type] << " , "
         << this->line_no << "}\n";
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}
Token LexicalAnalyzer::ScanBaseSixteen(){
    char c,d,e,f,h;
    bool base16=false;
    bool scale=true;
    input.GetChar(c);
    h=c;
    while(isdigit(c)||scale==true){
        input_store.push_back(h);
        input.GetChar(c);
        h = c;
        for (int i = 0; i < CHARWORDS_COUNT; i++) {
            if (c == charword[i]){
                scale = true;
                break;}
            else
                scale = false;
        }
    }

    if(c=='x') {
        d = c;
        input.GetChar(c);
        e = c;
        if (c == '1') {
            input.GetChar(c);
            f = c;
            if (c == '6') {
                base16 = true;
            } else {
                input.UngetChar(f);
                input.UngetChar(e);
                input.UngetChar(d);
            }
        } else {
            input.UngetChar(e);
            input.UngetChar(d);
        }
    }
            else
                input.UngetChar(c);


    if(base16==true) {
        while(!input_store.empty()) {
            h = input_store[0];
            input_store.erase(input_store.begin());
            tmp.lexeme += h;
        }
        tmp.lexeme+=d;
        tmp.lexeme+=e;
        tmp.lexeme+=f;
        tmp.line_no = line_no;
        tmp.token_type = BASE16NUM;
    }
    else {
        while(!input_store.empty()){
            h = input_store.back();
            input_store.pop_back();
            tmp.lexeme+=h;
        }
        for(int i=0;i<tmp.lexeme.size();i++){
            h=tmp.lexeme[i];
            input.UngetChar(h);
        }
        input.GetChar(c);
        input.UngetChar(c);
        if(isdigit(c))
            tmp=ScanNumber();
        else
            tmp=ScanIdOrKeyword();
    }
    return tmp;
}
Token LexicalAnalyzer::ScanNumber()
{
    char c;
    //
    char d,e,f;
    std::string a="";
    bool base8=false;
    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }

        // TODO: You can check for REALNUM, BASE08NUM and BASE16NUM here!
        input.GetChar(c);
        d=c;
        if(d=='.') {
            input.GetChar(c);
            if(isdigit(c)) {
                tmp.lexeme += d;
                while (!input.EndOfInput() && isdigit(c)) {
                    tmp.lexeme += c;
                    input.GetChar(c);
                }
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = REALNUM;
                tmp.line_no = line_no;
            }
            else{
                input.UngetChar(c);
                input.UngetChar(d);
                tmp.token_type = NUM;
                tmp.line_no = line_no;

            }
        }

        else if(d=='x') {
            input.GetChar(c);
            e = c;
            if (e == '0') {
                input.GetChar(c);
                f = c;
                if (f == '8')
                    base8 = true;
                else {
                    input.UngetChar(f);
                    input.UngetChar(e);
                    input.UngetChar(d);
                }
            } else {
                input.UngetChar(e);
                input.UngetChar(d);
            }

            if (base8 == true) {
                for (int i = 0; i < tmp.lexeme.size(); i++) {
                    int c = stoi(tmp.lexeme.substr(i, 1));
                    if (0 <= c && c <= 7)
                        base8 = true;
                    else {
                        base8 = false;
                        break;
                    }
                }
                if (base8 == false) {
                    input.UngetChar(f);
                    input.UngetChar(e);
                    input.UngetChar(d);
                    tmp.token_type = NUM;
                    tmp.line_no = line_no;
                } else {
                    tmp.lexeme += d;
                    tmp.lexeme += e;
                    tmp.lexeme += f;
                    tmp.token_type = BASE08NUM;
                    tmp.line_no = line_no;
                }
            }
            else{
                    tmp.token_type = NUM;
                    tmp.line_no = line_no;
            }
        }

        else {
            input.UngetChar(d);
            tmp.token_type = NUM;
            tmp.line_no = line_no;
        }
        return tmp;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        //
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            return tmp;
        case '/':
            tmp.token_type = DIV;
            return tmp;
        case '*':
            tmp.token_type = MULT;
            return tmp;
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '[':
            tmp.token_type = LBRAC;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
        case '<':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = LTEQ;
            } else if (c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if (c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if (!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanBaseSixteen();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                return ScanBaseSixteen();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while (token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}