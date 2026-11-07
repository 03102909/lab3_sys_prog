#ifndef AST_H
#define AST_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

enum class ValType { INT, DOUBLE, BOOL };

struct Value {
    ValType type = ValType::INT;
    double num = 0;

    bool asBool() const { return num != 0; }

    string toString() const {
        if (type == ValType::BOOL) return num != 0 ? "true" : "false";
        if (type == ValType::INT) return to_string((long long)num);
        ostringstream ss;
        ss << num;
        return ss.str();
    }
};

extern map<string, Value> variables;

class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual Value eval() = 0;
    virtual string codegen(int indent = 0) = 0;
    virtual string toJSON() = 0;
};

class NumNode : public ASTNode {
    Value val;
public:
    NumNode(int v) : val{ValType::INT, (double)v} {}
    NumNode(double v) : val{ValType::DOUBLE, v} {}
    NumNode(bool v) : val{ValType::BOOL, v ? 1.0 : 0.0} {}
    Value eval() override { return val; }
    string codegen(int ind) override {
        if (val.type == ValType::BOOL) return val.asBool() ? "True" : "False";
        return val.toString();
    }
    string toJSON() override { return "{\"name\": \"" + val.toString() + "\"}"; }
};

class IdNode : public ASTNode {
    string id;
public:
    IdNode(string i) : id(i) {}
    Value eval() override {
        return variables.count(id) ? variables[id] : Value{ValType::INT, 0};
    }
    string codegen(int ind) override { return id; }
    string toJSON() override { return "{\"name\": \"" + id + "\"}"; }
};

class ParenNode : public ASTNode {
    ASTNode* expr;
public:
    ParenNode(ASTNode* e) : expr(e) {}
    Value eval() override { return expr->eval(); }
    string codegen(int ind) override { return "(" + expr->codegen(0) + ")"; }
    string toJSON() override { return expr->toJSON(); }
};

class UnaryOpNode : public ASTNode {
    string op;
    ASTNode* expr;
public:
    UnaryOpNode(string o, ASTNode* e) : op(o), expr(e) {}
    Value eval() override {
        if (op == "!") return Value{ValType::BOOL, expr->eval().asBool() ? 0.0 : 1.0};
        return Value{ValType::INT, 0};
    }
    string codegen(int ind) override {
        if (op == "!") return "not " + expr->codegen(0);
        return op + expr->codegen(0);
    }
    string toJSON() override {
        return "{\"name\": \"" + op + "\", \"children\": [" + expr->toJSON() + "]}";
    }
};

class BinOpNode : public ASTNode {
    string op;
    ASTNode *left, *right;
public:
    BinOpNode(string o, ASTNode* l, ASTNode* r) : op(o), left(l), right(r) {}
    Value eval() override {
        if (op == "&&") return Value{ValType::BOOL, (left->eval().asBool() && right->eval().asBool()) ? 1.0 : 0.0};
        if (op == "||") return Value{ValType::BOOL, (left->eval().asBool() || right->eval().asBool()) ? 1.0 : 0.0};

        Value l = left->eval();
        Value r = right->eval();
        bool isFloat = (l.type == ValType::DOUBLE || r.type == ValType::DOUBLE);
        ValType resType = isFloat ? ValType::DOUBLE : ValType::INT;

        if (op == "+") return Value{resType, isFloat ? (l.num + r.num) : (double)((long long)l.num + (long long)r.num)};
        if (op == "-") return Value{resType, isFloat ? (l.num - r.num) : (double)((long long)l.num - (long long)r.num)};
        if (op == "*") return Value{resType, isFloat ? (l.num * r.num) : (double)((long long)l.num * (long long)r.num)};
        if (op == "/") {
            if (r.num == 0) return Value{resType, 0};
            if (isFloat) return Value{resType, l.num / r.num};
            long long q = (long long)l.num / (long long)r.num;
            return Value{resType, static_cast<double>(q)};
        }
        if (op == "<")  return Value{ValType::BOOL, (l.num < r.num) ? 1.0 : 0.0};
        if (op == ">")  return Value{ValType::BOOL, (l.num > r.num) ? 1.0 : 0.0};
        if (op == "<=") return Value{ValType::BOOL, (l.num <= r.num) ? 1.0 : 0.0};
        if (op == ">=") return Value{ValType::BOOL, (l.num >= r.num) ? 1.0 : 0.0};
        if (op == "==") return Value{ValType::BOOL, (l.num == r.num) ? 1.0 : 0.0};
        if (op == "!=") return Value{ValType::BOOL, (l.num != r.num) ? 1.0 : 0.0};
        return Value{ValType::INT, 0};
    }
    string codegen(int ind) override {
        string pyOp = op;
        if (op == "&&") pyOp = "and";
        else if (op == "||") pyOp = "or";
        return left->codegen(0) + " " + pyOp + " " + right->codegen(0);
    }
    string toJSON() override {
        return "{\"name\": \"" + op + "\", \"children\": [" + left->toJSON() + ", " + right->toJSON() + "]}";
    }
};

class AssignNode : public ASTNode {
    string id;
    ASTNode* expr;
    string decl_type;
public:
    AssignNode(string i, ASTNode* e, string dtype) : id(i), expr(e), decl_type(dtype) {}
    Value eval() override {
        Value val = expr->eval();
        ValType targetType = val.type;
        if (decl_type == "int") targetType = ValType::INT;
        else if (decl_type == "double") targetType = ValType::DOUBLE;
        else if (decl_type == "bool") targetType = ValType::BOOL;
        else if (variables.count(id)) targetType = variables[id].type;

        if (targetType == ValType::INT) val = Value{ValType::INT, (double)((long long)val.num)};
        else if (targetType == ValType::DOUBLE) val = Value{ValType::DOUBLE, val.num};
        else if (targetType == ValType::BOOL) val = Value{ValType::BOOL, val.asBool() ? 1.0 : 0.0};

        variables[id] = val;
        return val;
    }
    string codegen(int ind) override {
        return string(ind, '\t') + id + " = " + expr->codegen(0);
    }
    string toJSON() override {
        string prefix = decl_type.empty() ? "" : (decl_type + " ");
        return "{\"name\": \"=\", \"children\": [{\"name\": \"" + prefix + id + "\"}, " + expr->toJSON() + "]}";
    }
};

class SeqNode : public ASTNode {
public:
    vector<ASTNode*> stmts;
    void add(ASTNode* node) { stmts.push_back(node); }
    Value eval() override {
        Value last_val;
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
    ASTNode *cond, *body, *else_body;
public:
    IfNode(ASTNode* c, ASTNode* b, ASTNode* e = nullptr)
        : cond(c), body(b), else_body(e) {}
    Value eval() override {
        if (cond->eval().asBool()) return body->eval();
        if (else_body) return else_body->eval();
        return Value{ValType::INT, 0};
    }
    string codegen(int ind) override {
        string res = string(ind, '\t') + "if " + cond->codegen(0) + ":\n";
        res += body->codegen(ind + 1);
        if (else_body) {
            res += string(ind, '\t') + "else:\n";
            res += else_body->codegen(ind + 1);
        }
        return res;
    }
    string toJSON() override {
        string res = "{\"name\": \"if\", \"children\": [{\"name\": \"cond\", \"children\": [" + cond->toJSON() + "]}, " + body->toJSON();
        if (else_body) {
            res += ", {\"name\": \"else\", \"children\": [" + else_body->toJSON() + "]}";
        }
        return res + "]}";
    }
};

class WhileNode : public ASTNode {
    ASTNode *cond, *body;
public:
    WhileNode(ASTNode* c, ASTNode* b) : cond(c), body(b) {}
    Value eval() override {
        Value res;
        int guard = 0;
        while (cond->eval().asBool() && guard++ < 100000) {
            res = body->eval();
        }
        return res;
    }
    string codegen(int ind) override {
        return string(ind, '\t') + "while " + cond->codegen(0) + ":\n" + body->codegen(ind + 1);
    }
    string toJSON() override {
        return "{\"name\": \"while\", \"children\": [{\"name\": \"cond\", \"children\": [" + cond->toJSON() + "]}, " + body->toJSON() + "]}";
    }
};

class ForNode : public ASTNode {
    ASTNode *init, *cond, *step, *body;
public:
    ForNode(ASTNode* i, ASTNode* c, ASTNode* s, ASTNode* b)
        : init(i), cond(c), step(s), body(b) {}
    Value eval() override {
        init->eval();
        Value res;
        int guard = 0;
        while (cond->eval().asBool() && guard++ < 100000) {
            res = body->eval();
            step->eval();
        }
        return res;
    }
    string codegen(int ind) override {
        string pad(ind, '\t');
        string res = init->codegen(ind) + "\n";
        res += pad + "while " + cond->codegen(0) + ":\n";
        res += body->codegen(ind + 1);
        res += step->codegen(ind + 1) + "\n";
        return res;
    }
    string toJSON() override {
        return "{\"name\": \"for\", \"children\": [" + init->toJSON() +
               ", {\"name\": \"cond\", \"children\": [" + cond->toJSON() + "]}, " +
               step->toJSON() + ", " + body->toJSON() + "]}";
    }
};
#endif
