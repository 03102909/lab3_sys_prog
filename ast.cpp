#ifndef AST_H
#define AST_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

extern map<string, int> variables;

class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual int eval() = 0;
    virtual string codegen(int indent = 0) = 0;
    virtual string toJSON() = 0;
};

class NumNode : public ASTNode {
    int val;
public:
    NumNode(int v) : val(v) {}
    int eval() override { return val; }
    string codegen(int ind) override { return to_string(val); }
    string toJSON() override { return "{\"name\": \"" + to_string(val) + "\"}"; }
};

class IdNode : public ASTNode {
    string id;
public:
    IdNode(string i) : id(i) {}
    int eval() override {
        return variables.count(id) ? variables[id] : 0;
    }
    string codegen(int ind) override { return id; }
    string toJSON() override { return "{\"name\": \"" + id + "\"}"; }
};

class BinOpNode : public ASTNode {
    char op;
    ASTNode *left, *right;
public:
    BinOpNode(char o, ASTNode* l, ASTNode* r) : op(o), left(l), right(r) {}
    int eval() override {
        if (op == '+') return left->eval() + right->eval();
        if (op == '*') return left->eval() * right->eval();
        if (op == '<') return left->eval() < right->eval();
        return 0;
    }
    string codegen(int ind) override {
        return left->codegen(0) + " " + op + " " + right->codegen(0);
    }
    string toJSON() override {
        return "{\"name\": \"" + string(1, op) + "\", \"children\": [" + left->toJSON() + ", " + right->toJSON() + "]}";
    }
};

class AssignNode : public ASTNode {
    string id;
    ASTNode* expr;
    bool is_decl;
public:
    AssignNode(string i, ASTNode* e, bool decl) : id(i), expr(e), is_decl(decl) {}
    int eval() override {
        int val = expr->eval();
        variables[id] = val;
        return val;
    }
    string codegen(int ind) override {
        return string(ind, '\t') + id + " = " + expr->codegen(0);
    }
    string toJSON() override {
        string prefix = is_decl ? "int " : "";
        return "{\"name\": \"=\", \"children\": [{\"name\": \"" + prefix + id + "\"}, " + expr->toJSON() + "]}";
    }
};

class SeqNode : public ASTNode {
public:
    vector<ASTNode*> stmts;
    void add(ASTNode* node) { stmts.push_back(node); }
    int eval() override {
        int last_val = 0;
        for (auto stmt : stmts) last_val = stmt->eval();
        return last_val;
    }
    string codegen(int ind) override {
        string res = "";
        for (auto stmt : stmts) res += stmt->codegen(ind) + "\n";
        return res;
    }
    string toJSON() override {
        string res = "{\"name\": \"Block\", \"children\": [";
        for (size_t i = 0; i < stmts.size(); ++i) {
            res += stmts[i]->toJSON() + (i < stmts.size() - 1 ? ", " : "");
        }
        return res + "]}";
    }
};

class IfNode : public ASTNode {
    ASTNode *cond, *body;
public:
    IfNode(ASTNode* c, ASTNode* b) : cond(c), body(b) {}
    int eval() override {
        if (cond->eval()) return body->eval();
        return 0;
    }
    string codegen(int ind) override {
        string res = string(ind, '\t') + "if " + cond->codegen(0) + ":\n";
        res += body->codegen(ind + 1);
        return res;
    }
    string toJSON() override {
        return "{\"name\": \"if\", \"children\": [{\"name\": \"cond\", \"children\": [" + cond->toJSON() + "]}, " + body->toJSON() + "]}";
    }
};
#endif