/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *               Rida Bazzi 2019
 * Do not share this file with anyone
 *
 * 
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <map>
#include "lexer.h"
using namespace std;

// Define data structures to hold the grammar, token lists, and sets.
vector<pair<string, vector<string>>> grammar;   // Store the grammar rules.
vector<std::string> RHS;                       // Right-hand sides of grammar rules.
vector<std::string> tokenlist;                 // List of tokens in the grammar.
vector<std::string> nonterminals;              // List of non-terminal symbols.
vector<std::string> nonterminal1;             // List of non-terminal symbols excluding start symbol.
vector<std::string> terminals;                // List of terminal symbols.
vector<int> index;                            // Store indices of useful grammar rules.
LexicalAnalyzer lexer;                        // Lexical analyzer for tokenizing input.
Token temp;                                   // Temporary variable for storing tokens.
map<string, vector<string>> firstSet;         // First sets for non-terminals.
map<string, vector<string>> orderFirstset;    // Ordered first sets for non-terminals.
map<string, vector<string>> followSet;        // Follow sets for non-terminals.
map<string, vector<string>> orderFollowset;   // Ordered follow sets for non-terminals.

// Function to read the grammar from standard input.
void ReadGrammar() {
    string left;
    temp = lexer.GetToken();
    while (temp.token_type != HASH) {
        if (temp.token_type == ID) {
            left = temp.lexeme; // left side of the grammar
            nonterminals.push_back(left);
            temp = lexer.GetToken();
            if(find(tokenlist.begin(), tokenlist.end(), left) == tokenlist.end())
                tokenlist.push_back(left);
            if (temp.token_type == ARROW) {
                RHS.clear();

                temp = lexer.GetToken();
                while (temp.token_type != STAR) {
                    RHS.push_back(temp.lexeme); // Right-hand side symbols.
                    if(find(tokenlist.begin(), tokenlist.end(), temp.lexeme) == tokenlist.end())
                        tokenlist.push_back(temp.lexeme);
                    temp = lexer.GetToken();
                }
                grammar.push_back(make_pair(left, RHS));
            }
        }
        temp = lexer.GetToken();
    }
    // Separate terminals and non-terminals.
    for(string temp:tokenlist) {//oredernonterminal
        if (find(nonterminals.begin(), nonterminals.end(), temp) == nonterminals.end())
            terminals.push_back(temp);
        else
            nonterminal1.push_back(temp);
    }
    }

// Task 1: Print terminals and non-terminals.
void printTerminalsAndNoneTerminals()
{
  for(string temp:terminals)
      cout<<temp<<" ";
  for(string temp:nonterminal1)
      cout<<temp<<" ";

}

// Task 2: Remove useless symbols from the grammar.
void RemoveUselessSymbols() {
    // Identify useful and reachable symbols.
    bool use, reach;
    bool change = true;
    std::vector<std::string> useful = terminals;
    std::vector<std::string> reachable;
    while (change == true) {
        change = false;
        for (int i = 0; i < grammar.size(); i++) {
            use = true;
            for (int j = 0; j < grammar[i].second.size(); j++) {
                if (find(useful.begin(), useful.end(), grammar[i].second[j]) == useful.end()) {
                    use = false;
                    break;
                }
            }
            if (use == true && find(useful.begin(), useful.end(), grammar[i].first) == useful.end()) {
                useful.push_back(grammar[i].first);
                change = true;
            }
        }
    }
    change = true;
    reachable.push_back(grammar[0].first); // The start symbol is always reachable.
    while (change == true) {
        change = false;
        for (int t = 0; t < grammar.size(); t++) {
            if (find(reachable.begin(), reachable.end(), grammar[t].first) != reachable.end()) {
                reach = true;
                for (int j = 0; j < grammar[t].second.size(); j++) {
                    if (find(useful.begin(), useful.end(), grammar[t].second[j]) == useful.end()) {
                        reach = false;
                    }
                }

                if (reach == true && find(index.begin(), index.end(), t) == index.end()) {
                    for (int j = 0; j < grammar[t].second.size(); j++) {
                        reachable.push_back(grammar[t].second[j]);
                    }
                    if (grammar[t].second.empty())
                        grammar[t].second.push_back("#");
                    index.push_back(t);
                    change = true;
                }
            }
        }
    }
}

// Print grammar rules after removing useless symbols.
void PrintUseless(){
    for (int i = 0; i < grammar.size(); i++) {
        if(find(index.begin(),index.end(),i)!=index.end()){
            cout << grammar[i].first << " -> ";
            for (int j = 0; j < grammar[i].second.size(); j++) {
                cout << grammar[i].second[j] << " ";
            }
            cout << "\n";
        }
    }
}

// Task 3: Calculate and print First Sets for non-terminals.
void CalculateFirstSets() {
    string temp;
    bool epsilon;
    bool change = true;
    // Initialize First Sets for terminals.
    for (string a: terminals)
        firstSet[a].push_back(a);
    // Initialize First Sets for non-terminals.
    for (string b: nonterminal1)
        firstSet[b];
    while (change == true) {
        change = false;
        for (int i = 0; i < grammar.size(); i++) {
            string j = grammar[i].first;
            if (grammar[i].second.empty()) {
                if (find(firstSet[j].begin(), firstSet[j].end(), "#") == firstSet[j].end()) {
                    firstSet[j].push_back("#");
                    change = true;
                }
            } else {
                //rule 3 and 4
                for (int l = 0; l < grammar[i].second.size(); l++) {
                    epsilon = false;
                    temp = grammar[i].second[l];
                    for (string a: firstSet[temp]) {
                        if (a == "#")
                            epsilon = true;
                        else {
                            if (find(firstSet[j].begin(), firstSet[j].end(), a) == firstSet[j].end()) {
                                firstSet[j].push_back(a);
                                change = true;
                            }
                        }
                    }
                    if (epsilon == false)
                        break;
                }
                //rule 5
                if (epsilon == true) {
                    if (find(firstSet[j].begin(), firstSet[j].end(), "#") == firstSet[j].end()) {
                        firstSet[j].push_back("#");
                        change = true;
                    }
                }

            }
        }
    }
}

// Print First Sets for non-terminals.
void PrintFirstSet(){
    std::string c;
    terminals.insert(terminals.begin(), "#");
    for (int a = 0; a < nonterminal1.size(); a++) {
        c = nonterminal1[a];
        for (string r: terminals) {
            if (find(firstSet[c].begin(), firstSet[c].end(), r) != firstSet[c].end())
                orderFirstset[c].push_back(r);
        }
    }
    terminals.erase(terminals.begin());

    c = nonterminal1[0];
    if(firstSet[c].empty())
        cout << "FIRST(" << c << ")={  }";
    else if (firstSet[c].size() == 1)
        cout << "FIRST(" << c << ")={ " << orderFirstset[c][0] << " }";
    else {
        cout << "FIRST(" << c << ")={ " << orderFirstset[c][0];
        for (int i = 1; i < orderFirstset[c].size(); i++)
            cout << ", " << orderFirstset[c][i];
            cout << " }";

    }

        for (int a = 1; a < nonterminal1.size(); a++) {
            c = nonterminal1[a];
            if(firstSet[c].empty())
                cout << "\nFIRST(" << c << ")={  }";
            else if (orderFirstset[c].size() == 1)
                cout << "\nFIRST(" << c << ")={ " << orderFirstset[c][0] << " }";
            else {
                cout << "\nFIRST(" << c << ")={ " << orderFirstset[c][0];
                for (int i = 1; i < orderFirstset[c].size(); i++)
                    cout << ", " << orderFirstset[c][i];
                cout << " }";
            }
        }
    }


// Task 4: Calculate and print Follow Sets for non-terminals.
void CalculateFollowSets() {
    bool change = true;
    bool epsilon;
    string temp;
    for (string b: nonterminal1)
        followSet[b];
    followSet[nonterminal1[0]].push_back("$"); // Follow set of the start symbol contains the end marker "$".
    while (change == true) {
        change = false;
        for (string a: nonterminal1) {
            for (int i = 0; i < grammar.size(); i++) {
                for (int index = 0; index < grammar[i].second.size(); index++) {
                    if (grammar[i].second[index] == a) {
                        temp = grammar[i].first;
                        if (index == grammar[i].second.size() - 1) {
                            for (string o: followSet[temp]) {
                                if (find(followSet[a].begin(), followSet[a].end(), o) == followSet[a].end()) {
                                    followSet[a].push_back(o);
                                    change = true;
                                }
                            }
                        } else {
                            for (int k = index + 1; k < grammar[i].second.size(); k++) {
                                epsilon = false;
                                for (string b: firstSet[grammar[i].second[k]]) {
                                    if (b == "#")
                                        epsilon = true;
                                    else {
                                        if (find(followSet[a].begin(), followSet[a].end(), b) == followSet[a].end()) {
                                            followSet[a].push_back(b);
                                            change = true;
                                        }
                                    }
                                }
                                if (epsilon == false)
                                    break;
                            }
                            if (epsilon == true) {
                                for (string o: followSet[temp]) {
                                    if (find(followSet[a].begin(), followSet[a].end(), o) == followSet[a].end()) {
                                        followSet[a].push_back(o);
                                        change = true;
                                    }
                                }
                            }
                        }

                    }
                }
            }
        }
    }
}

// Print Follow Sets for non-terminals.
void PrintFollowSets(){
    std::string c;
    terminals.insert(terminals.begin(), "$");
    for (int a = 0; a < nonterminal1.size(); a++) {
        c = nonterminal1[a];
        for (string r: terminals) {
            if (find(followSet[c].begin(), followSet[c].end(), r) != followSet[c].end())
                orderFollowset[c].push_back(r);
        }
    }
    terminals.erase(terminals.begin());

    c = nonterminal1[0];
    if (orderFollowset[c].empty())
        cout << "FOLLOW(" << c << ")={  }";
    else if (orderFollowset[c].size() == 1)
        cout << "FOLLOW(" << c << ")={ " << orderFollowset[c][0] << " }";
    else {
        cout << "FOLLOW(" << c << ")={ " << orderFollowset[c][0];
        for (int i = 1; i < orderFollowset[c].size(); i++)
            cout << ", " << orderFollowset[c][i];
        cout << " }";
    }
    for (int a = 1; a < nonterminal1.size(); a++) {
        c = nonterminal1[a];
        if (orderFollowset[c].empty())
            cout << "\nFOLLOW(" << c << ")={  }";
        else if (orderFollowset[c].size() == 1)
            cout << "\nFOLLOW(" << c << ")={ " << orderFollowset[c][0] << " }";
        else {
            cout << "\nFOLLOW(" << c << ")={ " << orderFollowset[c][0];
            for (int i = 1; i < orderFollowset[c].size(); i++)
                cout << ", " << orderFollowset[c][i];
            cout << " }";
        }
    }
}

// Task 5: Check if the grammar has a predictive parser.
void CheckIfGrammarHasPredictiveParser()
{
    bool ispp=true;
    RemoveUselessSymbols();
    CalculateFirstSets();
    CalculateFollowSets();
    std::vector<string> temp;
    std::vector<string> temp2;
    std::vector<string> temp3;
    if(index.size()==grammar.size()) {
         for(string a:nonterminal1) {
             for (int i = 0; i < grammar.size(); i++) {
                 if (grammar[i].first == a)
                      temp.push_back(grammar[i].second[0]);
                 if (grammar[i].second[0] == "#")
                     if (find(temp3.begin(), temp3.end(), grammar[i].first) == temp3.end())
                     temp3.push_back(grammar[i].first);
             }

             // 1: Check for conflicts in the First Sets.
             for (string b: temp) {
                 for (string c: firstSet[b]) {
                     if (find(temp2.begin(), temp2.end(), c) == temp2.end())
                         temp2.push_back(c);
                     else
                         ispp = false; // If there is a conflict, the grammar is not predictive.
                 }
             }
             temp2.clear();
             temp.clear();
         }
         // 2: Check if Follow Sets of non-terminals and First Sets of # have an intersection.
         for(string d:temp3){
             for(string e:firstSet[d]){
                 if(find(followSet[d].begin(),followSet[d].end(),e)!=followSet[d].end()) {
                     ispp = false; // If there is an intersection, the grammar is not predictive.
                     break;
                 }
             }
         }
         if(ispp==true)
             cout<<"YES"; // The grammar has a predictive parser.
         else
             cout<<"NO"; // The grammar does not have a predictive parser.

    }
    else
        cout<<"NO"; // The grammar does not have a predictive parser.
}

int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);

    ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file

    switch (task) {
        case 1: printTerminalsAndNoneTerminals();
            break;

        case 2: RemoveUselessSymbols();
            PrintUseless();
            break;

        case 3: CalculateFirstSets();
            PrintFirstSet();
            break;

        case 4:  CalculateFirstSets();
            CalculateFollowSets();
            PrintFollowSets();
            break;

        case 5: CheckIfGrammarHasPredictiveParser();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}