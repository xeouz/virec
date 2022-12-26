#include "parser.hpp"

#define ERR_OUT stdout

namespace vire
{
    std::unique_ptr<ExprAST> VParser::LogError(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(ERR_OUT,"Parse Error: ");
        std::vfprintf(ERR_OUT,str,args);
        va_end(args);
        return nullptr;
    }
    std::unique_ptr<PrototypeAST> VParser::LogErrorP(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(ERR_OUT,"Parse Error: ");
        std::vfprintf(ERR_OUT,str,args);
        va_end(args);
        return nullptr;
    }
    std::unique_ptr<FunctionAST> VParser::LogErrorF(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(ERR_OUT,"Parse Error: ");
        std::vfprintf(ERR_OUT,str,args);
        va_end(args);
        return nullptr;
    }
    std::unique_ptr<ClassAST> VParser::LogErrorC(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(ERR_OUT,"Parse Error: ");
        std::vfprintf(ERR_OUT,str,args);
        va_end(args);
        return nullptr;
    }
    std::vector<std::unique_ptr<ExprAST>> VParser::LogErrorVP(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(ERR_OUT,"Parse Error: ");
        std::vfprintf(ERR_OUT,str,args);
        va_end(args);
        return std::vector<std::unique_ptr<ExprAST>>();
    }
    std::pair<std::unordered_map<proto::IName, std::unique_ptr<ExprAST>>, std::unique_ptr<FunctionAST>> VParser::LogErrorPB(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(ERR_OUT,"Parse Error: ");
        std::vfprintf(ERR_OUT,str,args);
        va_end(args);
        return std::pair<std::unordered_map<proto::IName, std::unique_ptr<ExprAST>>, std::unique_ptr<FunctionAST>>();
    }

    void VParser::getNextToken(bool first_token)
    {
        if(current_token && current_token->type==tok_eof && !first_token)
        {
            LogError("End of file\n");
            return;
        }

        current_token.reset();
        current_token=lexer->getToken();

        if(!current_token)
        {
            LogError("Invalid Token Detected\n");
            getNextToken();
        }
    }
    void VParser::getNextToken(int toktype)
    {  
        if(current_token->type!=toktype)
        {
            LogError("Current token type %s does not match type %s, Current token: `%s`\n",
            tokToStr(current_token->type), tokToStr(toktype), current_token->value.c_str());
            parse_success=false;
        }
        
        getNextToken();
    }
    std::unique_ptr<VToken> VParser::copyCurrentToken()
    {
        return std::make_unique<VToken>(current_token->value, current_token->type, current_token->line, current_token->charpos);
    }

    std::unique_ptr<types::Base> VParser::ParseTypeIdentifier()
    {
        // int[3][3]
        auto main_type_tok=copyCurrentToken();
        auto main_type=types::construct(main_type_tok->value);
        getNextToken(tok_id);

        while(current_token->type==tok_lbrack)
        {
            getNextToken();
            auto arr_num=std::stoi(current_token->value);
            getNextToken(tok_int);

            auto main_type_child=std::move(main_type);
            main_type=std::make_unique<types::Array>(std::move(main_type_child), arr_num);
            getNextToken(tok_rbrack);
        }

        if(main_type->getType() == types::TypeNames::Void)
        {
            ((types::Void*)main_type.get())->setName(proto::IName(main_type_tok->value).get());
        }

        return std::move(main_type);
    }
    std::vector<std::unique_ptr<ExprAST>> VParser::ParseBlock()
    {
        getNextToken(tok_lbrace); // consume '{'

        std::vector<std::unique_ptr<ExprAST>> stms;
        while(current_token->type!=tok_rbrace)
        {
            if(current_token->type==tok_eof)
            {
                parse_success=false;
                return LogErrorVP("Expected '}', found end of file");
            }
            
            auto stm=ParsePrimary();
            while(!stm)
            {
                if(current_token->type==tok_eof)
                {
                    parse_success=false;
                    return LogErrorVP("Expected statement, found end of file");
                }

                getNextToken();
                stm=ParsePrimary();
            }

            if(stm->asttype==ast_return) 
            {
                ((std::unique_ptr<ReturnExprAST> const&)stm)->setName(current_func_name->name); 
            }

            if(stm->asttype!=ast_for 
            && stm->asttype!=ast_while 
            && stm->asttype!=ast_unsafe
            && stm->asttype!=ast_if
            && stm->asttype!=ast_ifelse)
                getNextToken(tok_semicol);

            stms.push_back(std::move(stm));
        }

        getNextToken(tok_rbrace);

        return std::move(stms);
    }

    std::unique_ptr<ExprAST> VParser::ParsePrimary()
    {
        switch(current_token->type)
        {
            default:
            {
                parse_success=false;
                return LogError("Unknown token `%s` found when expecting statement, value: %s\n",tokToStr(current_token->type),current_token->value.c_str());
            }
            
            case tok_id: return ParseIdExpr();
            case tok_incr: return ParseIncrementDecrementExpr();
            case tok_decr: return ParseIncrementDecrementExpr();

            case tok_int: return ParseNumberExpr();
            case tok_float: return ParseNumberExpr();
            case tok_double: return ParseNumberExpr();
            case tok_char: return ParseStrExpr();
            case tok_str: return ParseStrExpr();
            case tok_true: return ParseBoolExpr();
            case tok_false: return ParseBoolExpr();
            case tok_lbrack: return ParseArrayExpr();

            case tok_lparen: return ParseParenExpr();

            case tok_vardef: return ParseVariableDef();
            case tok_const: return ParseVariableDef();
            case tok_let: return ParseVariableDef();

            case tok_for: return ParseForExpr();
            case tok_while: return ParseWhileExpr();

            case tok_if: return ParseIfExpr();

            case tok_return: return ParseReturn();
            case tok_break: return ParseBreakContinue();
            case tok_continue: return ParseBreakContinue();

            case tok_union: return ParseUnion();
            case tok_struct: return ParseStruct();

            case tok_new: return ParseNewExpr();
            case tok_delete: return ParseDeleteExpr();

            case tok_unsafe: return ParseUnsafe();
            case tok_reference: return ParseReference();
        }
    }

    std::unique_ptr<ExprAST> VParser::ParseExpression()
    {
        auto LHS=ParsePrimary();
        if(!LHS)
        {
            parse_success=false;
            return nullptr;
        }

        return ParseBinopExpr(0,std::move(LHS));
    }

    std::unique_ptr<ExprAST> VParser::ParseIdExpr(bool include_assign)
    {
        auto id_name=copyCurrentToken();

        getNextToken(tok_id);
        
        std::unique_ptr<ExprAST> expr;
        if(current_token->type!=tok_lparen) // if it is not a function call
        {
            expr=std::make_unique<VariableExprAST>(std::move(id_name));
            
            while(current_token->type==tok_dot || current_token->type==tok_lbrack)
            {
                if(current_token->type==tok_dot)
                {
                    expr=ParseClassAccess(std::move(expr));
                }
                
                if(current_token->type==tok_lbrack)
                {
                    std::vector<std::unique_ptr<ExprAST>> indices;
                    while(current_token->type == tok_lbrack)
                    {
                        getNextToken();
                        auto index=ParseExpression();
                        if(!index)
                        {
                            return nullptr;
                        }
                        getNextToken(tok_rbrack);

                        indices.push_back(std::move(index));
                    }

                    expr=std::make_unique<VariableArrayAccessAST>(std::move(expr),std::move(indices));
                }
            }

            if(include_assign)
            {
                if(current_token->type==tok_equal)
                {
                    return ParseVariableAssign(std::move(expr));
                }
                else if(current_token->type==tok_pluseq
                    || current_token->type==tok_minuseq
                    || current_token->type==tok_muleq
                    || current_token->type==tok_diveq
                    || current_token->type==tok_modeq
                )
                {
                    return ParseShorthandVariableAssign(std::move(expr));
                }
            }
            
            if(current_token->type==tok_incr || current_token->type==tok_decr)
            {
                bool is_increment=(current_token->type=tok_incr);
                getNextToken();
                expr=std::make_unique<IncrementDecrementAST>(std::move(expr), false, is_increment);
            }
            
            return std::move(expr);
        }
        
        getNextToken(tok_lparen); // consume '('

        std::vector<std::unique_ptr<ExprAST>> args;
        if(current_token->type != tok_rparen)
        {
            while(1)
            {
                if(auto arg=ParseExpression())
                {
                    args.push_back(std::move(arg));
                }
                else
                    return nullptr;
                
                if(current_token->type==tok_rparen)
                    break;
                
                if(current_token->type!=tok_comma)
                    return LogError("Expected ')' or ',' in function call arg list");
                
                getNextToken(tok_comma);
            }
        }

        getNextToken(tok_rparen); // consume ')'

        expr=std::make_unique<CallExprAST>(std::move(id_name),std::move(args));
        
        return std::move(expr);
    }
    std::unique_ptr<ExprAST> VParser::ParseIncrementDecrementExpr()
    {
        bool is_increment=(current_token->type==tok_incr);
        getNextToken();

        auto expr=ParsePrimary();

        return std::make_unique<IncrementDecrementAST>(std::move(expr), true, is_increment);
    }

    std::unique_ptr<ExprAST> VParser::ParseNumberExpr()
    {
        auto token=copyCurrentToken();

        if(token->type==tok_int)
        {
            int num=std::stoi(token->value);
            auto result=std::make_unique<IntExprAST>(num, std::move(token));
            getNextToken(tok_int);
            return std::move(result);
        }
        else if(token->type==tok_float)
        {
            auto result=std::make_unique<FloatExprAST>(std::stof(current_token->value),std::move(token));
            getNextToken(tok_float);
            return std::move(result);
        }
        else if(token->type==tok_double)
        {
            auto result=std::make_unique<DoubleExprAST>(std::stod(current_token->value),std::move(token));
            getNextToken(tok_double);
            return std::move(result);
        }

        return nullptr;
    }
    std::unique_ptr<ExprAST> VParser::ParseStrExpr()
    {
        auto token=copyCurrentToken();
        if(current_token->type==tok_char)
        {
            auto result=std::make_unique<CharExprAST>(current_token->value.at(0),std::move(token));
            getNextToken(tok_char);
            return std::move(result);
        }
        else // assume the tok is tok_str
        {
            auto result=std::make_unique<StrExprAST>(std::string(current_token->value),std::move(token));
            getNextToken(tok_str);
            return result;
        }
    }
    std::unique_ptr<ExprAST> VParser::ParseBoolExpr()
    {
        auto token=copyCurrentToken();
        if(current_token->type==tok_true)
        {
            getNextToken();
            return std::make_unique<BoolExprAST>(true, std::move(token));
        }
        else
        {
            getNextToken(tok_false);
            return std::make_unique<BoolExprAST>(false, std::move(token));
        }
    }
    std::unique_ptr<ExprAST> VParser::ParseArrayExpr()
    {
        getNextToken(tok_lbrack);

        std::vector<std::unique_ptr<ExprAST>> Elements;
        
        if(current_token->type!=tok_rbrack)
        {
            while(1)
            {
                auto elm=ParseExpression();
                Elements.push_back(std::move(elm));

                if(current_token->type==tok_rbrack)
                    break;
                
                if(current_token->type!=tok_comma)
                    return LogError("Expected ']' or ',' in array def");
                
                getNextToken(tok_comma);
            }
        }
        getNextToken(tok_rbrack);

        return std::make_unique<ArrayExprAST>(std::move(Elements));
    }

    std::unique_ptr<ExprAST> VParser::ParseParenExpr()
    {
        getNextToken(tok_lparen); // consume '('
        auto stm=ParseExpression();

        if(!stm)
            return nullptr;
        if(current_token->type!=tok_rparen)
            return LogError("Expected ')' left-parenthesis");
        
        getNextToken(tok_rparen); // consume ')'
        return stm;
    }

    std::unique_ptr<ExprAST> VParser::ParseBinopExpr(int ExprPrec, std::unique_ptr<ExprAST> LHS)
    {
        while(1)
        {  
            int prec=config->getBinopPrecedence(current_token->value);

            //Debug line, uncomment to use:
            //std::cout << *current_token << std::to_string(prec) << std::endl;
 
            if(prec<ExprPrec)
                return LHS;
            
            auto Binop=copyCurrentToken();
            getNextToken(); // consume the binop
            
            auto RHS=ParsePrimary();
            if(!RHS)
                return nullptr;
            
            int nextprec=config->getBinopPrecedence(current_token->value.c_str());
            if(prec<nextprec)
            {
                RHS=ParseBinopExpr(prec+1,std::move(RHS));
                if(!RHS)
                    return nullptr;
            }

            LHS=std::make_unique<BinaryExprAST>(std::move(Binop),std::move(LHS),std::move(RHS));
            if(current_token->type==tok_eof)
            {
                return std::move(LHS);
            }
        }
    }
 
    std::unique_ptr<ExprAST> VParser::ParseVariableDef()
    { 
        bool isconst=false;
        bool islet=true;
        if(current_token->type==tok_const)
            isconst=true;
        if(current_token->type==tok_let)
            islet=true;
        
        if(!isconst && !islet) 
            getNextToken(tok_vardef); // consume `var`
        else
            getNextToken(); // consume `let` / `const`

        auto var_name=copyCurrentToken();
        getNextToken(tok_id);

        bool is_array=false;
        std::unique_ptr<types::Base> type;
        types::Array* type_ref;

        if(current_token->type==tok_lbrack)
        {
            is_array=true;
            getNextToken(tok_lbrack);
            type=std::make_unique<types::Array>(std::make_unique<types::Void>(), std::stoi(current_token->value));
            type_ref=(types::Array*)type.get();
            getNextToken(tok_int);
            getNextToken(tok_rbrack);
            while(current_token->type==tok_lbrack)
            {
                getNextToken(tok_lbrack);
                type=std::make_unique<types::Array>(std::move(type), std::stoi(current_token->value));
                getNextToken(tok_int);
                getNextToken(tok_rbrack);
            }
        }

        if(is_array)
        {
            if(current_token->type==tok_colon)
            {
                getNextToken(tok_colon);
                type_ref->setChild(ParseTypeIdentifier());
            }
            else
            {
                // Automatic type inference
                type_ref->setChild(types::construct("void"));
            }
        }
        else
        {
            if(current_token->type==tok_colon)
            {
                getNextToken(tok_colon);
                type=ParseTypeIdentifier();
            }
            else
            {
                // Automatic type inference
                type=types::construct("void");
            }
        }

        std::unique_ptr<ExprAST> value=nullptr;
        if(current_token->type==tok_equal)
        {
            getNextToken(tok_equal);
            if(is_array)
                value=ParseArrayExpr();
            else
                value=ParseExpression();
        }

        if(is_array && value!=nullptr)
        {
            auto* vtype=(types::Array*)value->getType();
            auto* stype=(types::Array*)type.get();
            
            if(vtype->getLength() <= stype->getLength())
            {
                vtype->setLength(stype->getLength());
            }

        }

        return std::make_unique<VariableDefAST>(std::move(var_name), std::move(type), std::move(value), isconst, islet);
    }
    std::unique_ptr<ExprAST> VParser::ParseVariableAssign(std::unique_ptr<ExprAST> expr)
    {
        getNextToken(tok_equal);
        auto value=ParseExpression();

        return std::make_unique<VariableAssignAST>(std::move(expr), std::move(value));
    }
    std::unique_ptr<ExprAST> VParser::ParseShorthandVariableAssign(std::unique_ptr<ExprAST> var)
    {
        auto token=copyCurrentToken();
        getNextToken();

        auto expr=ParseExpression();

        std::unique_ptr<VToken> sym;

        switch(token->type)
        {
            case tok_pluseq: 
            {
                sym=VToken::construct("+", tok_plus, token->line, token->charpos);
                break;
            }
            case tok_minuseq:
            {
                sym=VToken::construct("-", tok_minus, token->line, token->charpos);
                break;
            }
            case tok_muleq:
            {
                sym=VToken::construct("*", tok_mul, token->line, token->charpos);
                break;
            }
            case tok_diveq:
            {
                sym=VToken::construct("/", tok_div, token->line, token->charpos);
                break;
            }
            case tok_modeq:
            {
                sym=VToken::construct("%", tok_mod, token->line, token->charpos);
                break;
            }

            default:
            {
                sym=nullptr;
            }
        }

        return std::make_unique<VariableAssignAST>(std::move(var), std::move(expr), std::move(sym));
    }

    std::unique_ptr<ExprAST> VParser::ParseForExpr()
    {
        getNextToken(tok_for);
        getNextToken(tok_lparen);

        auto initStm=ParsePrimary();
        getNextToken(tok_semicol);
        auto condStm=ParseExpression();
        getNextToken(tok_semicol);
        auto incrStm=ParsePrimary();
        getNextToken(tok_rparen);

        auto Stms=ParseBlock();

        return std::make_unique<ForExprAST>
        (std::move(initStm), std::move(condStm), std::move(incrStm), std::move(Stms));
    }
    std::unique_ptr<ExprAST> VParser::ParseWhileExpr()
    {
        getNextToken(tok_while);
        getNextToken(tok_lparen);

        auto cond=ParseExpression();
        getNextToken(tok_rparen);
        auto Stms=ParseBlock();

        return std::make_unique<WhileExprAST>(std::move(cond), std::move(Stms));
    }
    std::unique_ptr<ExprAST> VParser::ParseBreakContinue()
    {
        bool is_break=1;
        if(current_token->type==tok_continue) is_break=0;

        getNextToken();

        bool has_stm=0;
        std::unique_ptr<ExprAST> stm;
        if(current_token->type!=tok_semicol)
        {
            has_stm=1;
            stm=ParsePrimary();
        }

        if(is_break)
        {
            if(has_stm) return std::make_unique<BreakExprAST>(std::move(stm));
            else        return std::make_unique<BreakExprAST>();
        }
        else
        {
            if(has_stm) return std::make_unique<ContinueExprAST>(std::move(stm));
            else        return std::make_unique<ContinueExprAST>();
        }
    }

    std::unique_ptr<PrototypeAST> VParser::ParsePrototype()
    {
        if(current_token->type!=tok_id)
            return LogErrorP("Expected function name in prototype");
        
        std::unique_ptr<VToken> fn_name=copyCurrentToken();
        getNextToken(tok_id);

        if(current_token->type!=tok_lparen)
            return LogErrorP("Expected '(' in prototype after name");
        getNextToken(); // consume '('

        std::vector<std::unique_ptr<VariableDefAST>> args;
        while(current_token->type==tok_id)
        {
            std::unique_ptr<VToken> var_name=copyCurrentToken();
            getNextToken(tok_id); // consume id
            if(current_token->type!=tok_colon) 
                return LogErrorP("Expected ':' for type specifier after arg name");
            getNextToken(tok_colon); // consume colon

            auto type=ParseTypeIdentifier();
            auto var=std::make_unique<VariableDefAST>(std::move(var_name) , std::move(type), nullptr, true, false);
            
            args.push_back(std::move(var));

            if(current_token->type!=tok_comma)
            {
                break;
            }
            getNextToken(tok_comma); // consume comma
        }

        getNextToken(tok_rparen);

        std::unique_ptr<types::Base> return_type;
        if(current_token->type==tok_colon || current_token->type==tok_returns)
        {
            getNextToken();
            return_type=ParseTypeIdentifier();
        }
        else
        {
            return_type=types::construct(types::TypeNames::Void);
        }

        return std::make_unique<PrototypeAST>(std::move(fn_name), std::move(args), std::move(return_type));
    }
    std::unique_ptr<PrototypeAST> VParser::ParseProto()
    {
        getNextToken(tok_proto); // consume `proto`
        return ParsePrototype();
    }
    std::unique_ptr<ExternAST> VParser::ParseExtern()
    {
        getNextToken(tok_extern); // consume `extern`
        auto proto=ParsePrototype();
        return std::make_unique<ExternAST>(std::move(proto));
    }
    std::unique_ptr<FunctionAST> VParser::ParseFunction()
    {
        getNextToken(tok_func); // eat `func`

        auto proto=ParsePrototype();
        if(!proto)  return nullptr;

        current_func_name=&proto->getIName();
    
        auto stms=ParseBlock();
        
        return std::make_unique<FunctionAST>(std::move(proto), std::move(stms));
    }
    std::unique_ptr<ExprAST> VParser::ParseReturn()
    {
        getNextToken(tok_return);

        return std::make_unique<ReturnExprAST>(ParseExpression());
    }
 
    std::unique_ptr<ExprAST> VParser::ParseIfExpr()
    {
        getNextToken(tok_if);
        getNextToken(tok_lparen);
        auto cond=ParseExpression();
        getNextToken(tok_rparen);
        auto mthenStm=ParseBlock();

        std::vector<std::unique_ptr<IfThenExpr>> elseStms;
        while(current_token->type==tok_else)
        {
            getNextToken(tok_else);
            if(current_token->type==tok_if)
            {
                getNextToken(tok_if);
                getNextToken(tok_lparen);
                auto cond=ParseExpression();
                getNextToken(tok_rparen);
                auto thenStm=ParseBlock();
                elseStms.push_back(std::make_unique<IfThenExpr>(std::move(cond),std::move(thenStm)));
            }
            else
            {
                auto thenStm=ParseBlock();
                elseStms.push_back(std::make_unique<IfThenExpr>(nullptr,std::move(thenStm)));
                break;
            }
        }

        auto ifthen=std::make_unique<IfThenExpr>(std::move(cond),std::move(mthenStm));
        return std::make_unique<IfExprAST>(std::move(ifthen),std::move(elseStms));
    }
    std::unique_ptr<ClassAST> VParser::ParseClass()
    {
        getNextToken(tok_class);
        auto className=copyCurrentToken();
        getNextToken(tok_id);

        std::unique_ptr<VToken> Parent;
        if(current_token->type==tok_lparen)
        {
            getNextToken(tok_lparen);
            Parent=copyCurrentToken();
            getNextToken(tok_id);
            getNextToken(tok_rparen);
        }
        else if(current_token->type==tok_colon)
        {
            getNextToken(tok_colon);
            Parent=copyCurrentToken();
            getNextToken(tok_id);
        }
        else if(current_token->type==tok_extends)
        {
            getNextToken(tok_extends);
            Parent=copyCurrentToken();
            getNextToken(tok_id);
        }

        getNextToken(tok_lbrace);

        std::vector<std::unique_ptr<FunctionBaseAST>> funcs;
        std::vector<std::unique_ptr<VariableDefAST>> vars;
        while(current_token->type!=tok_rbrace)
        {
            if(current_token->type==tok_eof) return LogErrorC("Expected '}' found end of file");

            if(current_token->type==tok_func)
            {
                auto func=ParseFunction();
                funcs.push_back(std::move(func));
            }
            else if(current_token->type==tok_proto)
            {
                auto proto=ParseFunction();
                funcs.push_back(std::move(proto));
                getNextToken(tok_semicol);
            }
            else if(current_token->type==tok_vardef || current_token->type==tok_const || current_token->type==tok_let)
            {
                auto var=ParseVariableDef();
                auto varCast=cast_static<VariableDefAST>(std::move(var));
                vars.push_back(std::move(varCast));
                getNextToken(tok_semicol);
            }
        
        }
        getNextToken(tok_rbrace);

        return std::make_unique<ClassAST>(std::move(className),std::move(funcs),std::move(vars),std::move(Parent));
    }
    std::unique_ptr<ExprAST> VParser::ParseNewExpr()
    {
        getNextToken(tok_new); // consume `new`
        
        auto id_expr=ParseIdExpr();

        std::vector<std::unique_ptr<ExprAST>> args;
        std::unique_ptr<VToken> id_name;
        if(id_expr->asttype==ast_var)
        {
            std::unique_ptr<VariableExprAST> var(static_cast<VariableExprAST*>(id_expr.release()));
            id_name=var->moveToken();
        }
        else if(id_expr->asttype==ast_call)
        {
            std::unique_ptr<CallExprAST> call(static_cast<CallExprAST*>(id_expr.release()));
            id_name=call->moveToken();
            args=call->moveArgs();
        }

        return std::make_unique<NewExprAST>(std::move(id_name),std::move(args));
    }
    std::unique_ptr<ExprAST> VParser::ParseDeleteExpr()
    {
        getNextToken(tok_delete);

        auto id_name=copyCurrentToken();
        getNextToken(tok_id);

        return std::make_unique<DeleteExprAST>(std::move(id_name));
    }
    std::unique_ptr<ExprAST> VParser::ParseClassAccess(std::unique_ptr<ExprAST> parent)
    {
        getNextToken(tok_dot);
        auto child=ParseIdExpr(false);
 
        if(!(child->asttype==ast_var || child->asttype==ast_call || child->asttype==ast_type_access))
        {
            std::cout << "Expected a class member during parsing" << std::endl;
            return nullptr;
        }

        auto cast_child=cast_static<IdentifierExprAST>(std::move(child));

        auto x=std::make_unique<TypeAccessAST>(std::move(parent), std::move(cast_child));

        return std::move(x);
    }

    std::unique_ptr<FunctionAST> VParser::ParseConstructor()
    {
        getNextToken(tok_constructor);
        getNextToken(tok_lparen);

        std::vector<std::unique_ptr<VariableDefAST>> args;
        while(current_token->type==tok_id)
        {
            auto var_name=copyCurrentToken();
            getNextToken(tok_id);
            getNextToken(tok_colon);
            
            auto type=ParseTypeIdentifier();
            auto var=std::make_unique<VariableDefAST>(std::move(var_name), std::move(type), nullptr, true, false);
            args.push_back(std::move(var));
        }
        getNextToken(tok_rparen);

        auto block=ParseBlock();

        auto proto=std::make_unique<PrototypeAST>(VToken::construct("", tok_id), std::move(args), types::construct("void"), true, true);
        return std::make_unique<FunctionAST>(std::move(proto), std::move(block), true, true);
    }
    std::pair<std::unordered_map<proto::IName, std::unique_ptr<ExprAST>>, std::unique_ptr<FunctionAST>> VParser::ParsePrimitiveBody()
    {
        getNextToken(tok_lbrace);
        std::unique_ptr<FunctionAST> constructor;
        std::unordered_map<proto::IName, std::unique_ptr<ExprAST>> members;

        bool found_constructor=false;
        while(current_token->type!=tok_rbrace)
        {
            if(current_token->type==tok_eof) return LogErrorPB("Expected '}' found end of file");

            std::unique_ptr<ExprAST> member;
            std::string member_name;
            if(current_token->type==tok_union)
            {
                member=ParseUnion();
                member_name=((UnionExprAST*)member.get())->getIName().name;
            }
            else if(current_token->type==tok_struct)
            {
                member=ParseStruct();
                member_name=((StructExprAST*)member.get())->getIName().name;
            }
            else if(current_token->type==tok_id)
            {
                auto type=copyCurrentToken();
                getNextToken(tok_id);

                auto name=copyCurrentToken();
                getNextToken(tok_id);
                getNextToken(tok_semicol);

                member_name=name->value;
                member=std::make_unique<VariableDefAST>(std::move(name), types::construct(type->value), nullptr);
            }
            else if(current_token->type==tok_constructor)
            {
                auto cons=ParseConstructor();
                if(found_constructor) std::cout << "Already encountered constructor, multiple constructors yet to be added" << std::endl;
                else constructor=std::move(cons);
                continue;
            }
            else
            {
                std::cout << "Invalid Token: " << current_token->value << std::endl;
                break;
            }
            
            members.insert(std::make_pair(proto::IName(member_name), std::move(member)));
        }
        
        getNextToken(tok_rbrace);

        return std::make_pair(std::move(members), std::move(constructor));
    }
    std::unique_ptr<ExprAST> VParser::ParseUnion()
    {
        getNextToken(tok_union);

        char is_anonymous=1;
        std::unique_ptr<VToken> name;
        if(current_token->type==tok_id)
        {
            is_anonymous=0;
            name=copyCurrentToken();
            getNextToken();
        }

        auto body=ParsePrimitiveBody();
        return std::make_unique<UnionExprAST>(std::move(body.first), std::move(name));
    }
    std::unique_ptr<ExprAST> VParser::ParseStruct()
    {
        getNextToken(tok_struct);

        char is_anonymous=1;
        std::unique_ptr<VToken> name;
        if(current_token->type==tok_id)
        {
            is_anonymous=0;
            name=copyCurrentToken();
            getNextToken();
        }

        auto body=ParsePrimitiveBody();

        auto cons=std::move(body.second);
        auto members=std::move(body.first);
        return std::make_unique<StructExprAST>(std::move(members), std::move(cons), std::move(name));
    }

    std::unique_ptr<ExprAST> VParser::ParseUnsafe()
    {
        getNextToken(tok_unsafe);
        
        auto block=ParseBlock();
        return std::make_unique<UnsafeExprAST>(std::move(block));
    }
    std::unique_ptr<ExprAST> VParser::ParseReference()
    {
        getNextToken(tok_reference);

        auto var=ParseIdExpr();
        return std::make_unique<ReferenceExprAST>(std::move(var));
    }

    std::unique_ptr<ModuleAST> VParser::ParseSourceModule()
    {
        lexer->reset();

        getNextToken(true); // load the first token
        parse_success=true;

        std::vector<std::unique_ptr<ExprAST>> PreExecutionStatements;
        std::vector<std::unique_ptr<FunctionBaseAST>> Functions;
        std::vector<std::unique_ptr<ClassAST>> Classes;
        std::vector<std::unique_ptr<ExprAST>> StructUnionDefs;
        while(current_token->type!=tok_eof)
        {
            if(current_token->type==tok_class)
            {
                auto class_ast=ParseClass();
                Classes.push_back(std::move(class_ast));
            }
            else if(current_token->type==tok_func)
            {
                auto func_ast=ParseFunction();
                Functions.push_back(std::move(func_ast));
            }
            else if(current_token->type==tok_proto)
            {
                auto proto_ast=ParseProto();
                getNextToken(tok_semicol);
                Functions.push_back(std::move(proto_ast));
            }
            else if(current_token->type==tok_extern)
            {
                auto extern_ast=ParseExtern();
                getNextToken(tok_semicol);
                Functions.push_back(std::move(extern_ast));
            }
            else if(current_token->type==tok_struct)
            {
                auto struct_ast=ParseStruct();
                StructUnionDefs.push_back(std::move(struct_ast));
            }
            else if(current_token->type==tok_union)
            {
                auto union_ast=ParseUnion();
                StructUnionDefs.push_back(std::move(union_ast));
            }
            else
            {
                auto stm=ParsePrimary();

                if(stm->asttype!=ast_for 
                && stm->asttype!=ast_while 
                && stm->asttype!=ast_unsafe
                && stm->asttype!=ast_if
                && stm->asttype!=ast_ifelse)
                    getNextToken(tok_semicol);
                
                PreExecutionStatements.push_back(std::move(stm));
            }
        }
        
        if(!parse_success)
        {
            return nullptr;
        }

        return std::make_unique<ModuleAST>(std::move(PreExecutionStatements),std::move(Functions),std::move(Classes),std::move(StructUnionDefs));
    }
}