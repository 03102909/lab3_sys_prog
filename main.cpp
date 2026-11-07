#include <iostream>
#include "ast.cpp"

using namespace std;

map<string, int> variables;
extern ASTNode* root;
extern int yyparse();
typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char * str);

int main() {
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
        for (auto const& [key, val] : variables) {
            cout << key << " = " << val << "\n";
        }

        cout << "\nCODE GENERATION (Python)\n";
        cout << root->codegen(0) << "\n";

        cout << "\nAST VISUALIZATION (JSON for D3)\n";
        cout << root->toJSON() << "\n";
    }
    return 0;
}