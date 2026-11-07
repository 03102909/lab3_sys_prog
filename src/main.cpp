#include <iostream>
#include <sstream>
#include <string>
#include "ast.h"

using namespace std;

map<string, Value> variables;
extern ASTNode* root;
extern int yyparse();
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char * str);

static string escapeJSON(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

int main(int argc, char** argv) {
    if (argc > 1 && string(argv[1]) == "--stdin") {
        ostringstream buf;
        buf << cin.rdbuf();
        string input = buf.str();

        yy_scan_string(input.c_str());
        if (yyparse() != 0 || !root) {
            return 1;
        }

        root->eval();

        cout << "{\"python\": \"" << escapeJSON(root->codegen(0))
             << "\", \"vars\": {";
        bool first = true;
        for (auto const& kv : variables) {
            if (!first) cout << ", ";
            cout << "\"" << escapeJSON(kv.first) << "\": " << kv.second.toString();
            first = false;
        }
        cout << "}, \"ast\": " << root->toJSON() << "}\n";
        return 0;
    }

    const char* code =
        "int threshold = 10;\n"
        "int val = 5 + 2 * 2;\n"
        "if (val < threshold) {\n"
        "    val = 99;\n"
        "}\n";

    cout << "INPUT (C++)\n" << code << "\n";

    yy_scan_string(code);
    yyparse();

    if (root) {
        cout << "SEMANTICS (Evaluation)\n";
        root->eval();
        for (auto const& kv : variables) {
            cout << kv.first << " = " << kv.second.toString() << "\n";
        }

        cout << "\nCODE GENERATION (Python)\n";
        cout << root->codegen(0) << "\n";

        cout << "\nAST VISUALIZATION (JSON for D3)\n";
        cout << root->toJSON() << "\n";
    }
    return 0;
}
