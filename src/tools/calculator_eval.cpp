// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <stack>

namespace {
    using Kind = CalcToken::Kind;

    bool is_operator(Kind k){       //is valid operator?
        return k == Kind::Plus || k == Kind::Minus || k == Kind::Times || k == Kind::Divide;
    }

    int precedence(Kind k){     //order of operations
        return (k == Kind::Times|| k == Kind::Divide) ? 2 : 1;
    }

     
    bool apply_top(std::stack<double>& values, std::stack<Kind>& ops, std::string* error){
        // do you have both values and an operation to do?
        if (ops.empty() || values.size() < 2){     
            *error = "malformed expression";
            return false;
        }
        Kind op = ops.top();
        ops.pop();
        double right = values.top();
        values.pop();
        double left = values.top();
        values.pop();

        double result = 0.0;
        switch (op){
            case Kind::Plus:  
                result = left + right;
                break;
            case Kind::Minus: 
                result = left - right;
                break;
            case Kind::Times: 
                result = left * right;
                break;
            case Kind::Divide:
                if (right == 0.0){
                    *error = "division by zero";
                    return false;
                }
                result = left / right;
                break;
            default:
                *error = "malformed expression";
                return false;
                break;
        }
        values.push(result);    //save result in stack
        return true;
    }

}


bool calc_evaluate(const std::vector<CalcToken>& /*tokens*/, double* /*out*/,
                   std::string* error) {
    *error = "calc_evaluate not implemented yet";   // TODO
    return false;
}
