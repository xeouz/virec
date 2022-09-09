#include "parser.hpp"

namespace vire
{
    std::unique_ptr<ExprAST> Vireparse::LogError(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(stderr,"Parse Error: ");
        std::vfprintf(stderr,str,args);
        va_end(args);
        printf("\n");
        return nullptr;
    }
    std::unique_ptr<PrototypeAST> Vireparse::LogErrorP(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(stderr,"Parse Error: ");
        std::vfprintf(stderr,str,args);
        va_end(args);
        printf("\n");
        return nullptr;
    }
    std::unique_ptr<FunctionAST> Vireparse::LogErrorF(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(stderr,"Parse Error: ");
        std::vfprintf(stderr,str,args);
        va_end(args);
        printf("\n");
        return nullptr;
    }
    std::unique_ptr<ClassAST> Vireparse::LogErrorC(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(stderr,"Parse Error: ");
        std::vfprintf(stderr,str,args);
        va_end(args);
        printf("\n");
        return nullptr;
    }
    std::vector<std::unique_ptr<ExprAST>> Vireparse::LogErrorVP(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(stderr,"Parse Error: ");
        std::vfprintf(stderr,str,args);
        va_end(args);
        printf("\n");
        return std::vector<std::unique_ptr<ExprAST>>();
    }
    std::unordered_map<std::string, std::unique_ptr<ExprAST>> Vireparse::LogErrorPB(const char* str,...)
    {
        std::va_list args;
        va_start(args,str);
        fprintf(stderr,"Parse Error: ");
        std::vfprintf(stderr,str,args);
        va_end(args);
        printf("\n");
        return std::unordered_map<std::string, std::unique_ptr<ExprAST>>();
    }

    void Vireparse::getNextToken()
    {
        current_token.reset();
        current_token=lexer->getToken();

        if(current_token==nullptr) current_token=lexer->getToken();
    }
    void Vireparse::getNextToken(int toktype)
    {
        if(current_token->type!=toktype)
        {
            LogError("Current token type %s does not match type %s, Current token: `%s`",tokToStr(current_token->type),tokToStr(toktype),current_token->value.c_str());
        }
        getNextToken();
    }
    std::unique_ptr<Viretoken> Vireparse::copyCurrentToken()
    {
        return std::make_unique<Viretoken>(current_token->value,current_token->type,current_token->line,current_token->charpos);
    }

    std::vector<std::unique_ptr<ExprAST>> Vireparse::ParseBlock()
    {
        getNextToken(tok_lbrace); // consume '}'

        std::vector<std::unique_ptr<ExprAST>> Stms;
        while(current_token->type!=tok_rbrace)
        {
            if(current_token->type==tok_eof)
                return LogErrorVP("Expected '}' found end of file");
            auto stm=ParsePrimary();
            if(!stm) continue;

            if(stm->asttype==ast_return) 
            { ((std::unique_ptr<ReturnExprAST> const&)stm)->setName(current_func_name); }

            if(stm->asttype!=ast_for 
            && stm->asttype!=ast_while 
            && stm->asttype!=ast_unsafe
            && stm->asttype!=ast_if
            && stm->asttype!=ast_ifelse)
                getNextToken(tok_semicol);

            Stms.push_back(std::move(stm));
        }

        getNextToken(tok_rbrace);

        return std::move(Stms);
    }

    std::unique_ptr<ExprAST> Vireparse::ParsePrimary()
    {
        switch(current_token->type)
        {
            default: 
                return LogError("Unknown token `%s` found when expecting statement,value: %s",tokToStr(current_token->type),current_token->value.c_str());
            
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

    std::unique_ptr<ExprAST> Vireparse::ParseExpression()
    {
        auto LHS=ParsePrimary();
        if(!LHS)
        {
            return nullptr;
        }

        return ParseBinopExpr(0,std::move(LHS));
    }

    std::unique_ptr<ExprAST> Vireparse::ParseIdExpr()
    {
        std::unique_ptr<Viretoken> id_name=copyCurrentToken();
        id_name->value="_"+id_name->value;
        getNextToken(tok_id);
        
        std::unique_ptr<ExprAST> expr;
        if(current_token->type != tok_lparen) // if it is not a function call
        {
            if(current_token->type == tok_equal)
            {
                expr=ParseVariableAssign(std::move(id_name));
            }
            else
            {    
                expr=std::make_unique<VariableExprAST>(std::move(id_name));

                if(current_token->type == tok_dot)
                {
                    expr=ParseClassAccess(std::move(expr));
                }
                
                if(current_token->type == tok_lbrack)
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

            if(current_token->type==tok_incr || current_token->type==tok_decr)
            {
                bool is_increment=(current_token->type=tok_incr);
                getNextToken();
                expr=std::make_unique<IncrementDecrementAST>(std::move(expr), false, is_increment);
            }
            
            return std::move(expr);
        }
        
        // Remove `_` from the function name
        id_name->value=id_name->value.substr(1);

        getNextToken(tok_lparen); // consume '('

        std::vector<std::unique_ptr<ExprAST>> args;
        if(current_token->type != tok_rparen)
        {
            while(1)
            {
                if(auto Arg=ParseExpression())
                {
                    args.push_back(std::move(Arg));
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
    std::unique_ptr<ExprAST> Vireparse::ParseIncrementDecrementExpr()
    {
        bool is_increment=(current_token->type==tok_incr);
        getNextToken();

        auto expr=ParsePrimary();

        return std::make_unique<IncrementDecrementAST>(std::move(expr), true, is_increment);
    }

    std::unique_ptr<ExprAST> Vireparse::ParseNumberExpr()
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
    std::unique_ptr<ExprAST> Vireparse::ParseStrExpr()
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
    std::unique_ptr<ExprAST> Vireparse::ParseBoolExpr()
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
    std::unique_ptr<ExprAST> Vireparse::ParseArrayExpr()
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

    std::unique_ptr<ExprAST> Vireparse::ParseParenExpr()
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

    std::unique_ptr<ExprAST> Vireparse::ParseBinopExpr(int ExprPrec, std::unique_ptr<ExprAST> LHS)
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
 
    std::unique_ptr<ExprAST> Vireparse::ParseVariableDef()
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
        var_name->value="_"+var_name->value;
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
                type_ref->setChild(types::construct(current_token->value));
                getNextToken(tok_id);
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
                type=types::construct(current_token->value);
                getNextToken(tok_id);
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

        return std::make_unique<VariableDefAST>(std::move(var_name),std::move(type),std::move(value),isconst,islet);
    }
    std::unique_ptr<ExprAST> Vireparse::ParseVariableAssign(std::unique_ptr<Viretoken> varName)
    {
        getNextToken(tok_equal);
        auto value=ParseExpression();

        return std::make_unique<VariableAssignAST>(std::move(varName), std::move(value));
    }

    std::unique_ptr<ExprAST> Vireparse::ParseForExpr()
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
    std::unique_ptr<ExprAST> Vireparse::ParseWhileExpr()
    {
        getNextToken(tok_while);
        getNextToken(tok_lparen);

        auto cond=ParseExpression();
        getNextToken(tok_rparen);
        auto Stms=ParseBlock();

        return std::make_unique<WhileExprAST>(std::move(cond), std::move(Stms));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseBreakContinue()
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

    std::unique_ptr<PrototypeAST> Vireparse::ParsePrototype()
    {
        if(current_token->type!=tok_id)
            return LogErrorP("Expected function name in prototype");
        
        std::unique_ptr<Viretoken> fn_name=copyCurrentToken();
        getNextToken(tok_id);

        if(current_token->type!=tok_lparen)
            return LogErrorP("Expected '(' in prototype after name");
        getNextToken(); // consume '('

        std::vector<std::unique_ptr<VariableDefAST>> args;
        while(current_token->type==tok_id)
        {
            std::unique_ptr<Viretoken> var_name=copyCurrentToken();
            var_name->value="_"+var_name->value;
            getNextToken(tok_id); // consume id
            if(current_token->type!=tok_colon) 
                return LogErrorP("Expected ':' for type specifier after arg name");
            getNextToken(tok_colon); // consume colon

            unsigned char isconst=0;
            if(current_token->type==tok_const)
            {
                isconst=1;
                getNextToken();
            }

            auto type=types::construct(current_token->value);
            getNextToken(); // consume typename
            auto var=std::make_unique<VariableDefAST>(std::move(var_name),std::move(type),nullptr,isconst,!isconst);
            args.push_back(std::move(var));

            if(current_token->type!=tok_comma)
            {
                break;
            }
            getNextToken(tok_comma); // consume comma
        }

        getNextToken(tok_rparen);

        std::unique_ptr<Viretoken> return_type;
        if(current_token->type==tok_rarrow || current_token->type==tok_returns)
        {
            getNextToken();
            return_type=copyCurrentToken();
            getNextToken(tok_id);
        }
        else
        {
            return_type=copyCurrentToken();
        }
        
        return std::make_unique<PrototypeAST>(std::move(fn_name),std::move(args),std::move(return_type));
    }

    std::unique_ptr<PrototypeAST> Vireparse::ParseProto()
    {
        getNextToken(tok_proto); // consume `proto`
        return ParsePrototype();
    }
    std::unique_ptr<ExternAST> Vireparse::ParseExtern()
    {
        getNextToken(tok_extern); // consume `extern`
        auto proto=ParsePrototype();
        return std::make_unique<ExternAST>(std::move(proto));
    }
    std::unique_ptr<FunctionAST> Vireparse::ParseFunction()
    {
        getNextToken(tok_func); // eat `func`

        auto proto=ParsePrototype();
        if(!proto)  return nullptr;

        current_func_name=proto->getName();
    
        auto Stms=ParseBlock();

        return std::make_unique<FunctionAST>(std::move(proto),std::move(Stms));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseReturn()
    {
        getNextToken(tok_return);

        return std::make_unique<ReturnExprAST>(ParseExpression());
    }
 
    std::unique_ptr<ExprAST> Vireparse::ParseIfExpr()
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
    std::unique_ptr<ClassAST> Vireparse::ParseClass()
    {
        getNextToken(tok_class);
        auto className=copyCurrentToken();
        getNextToken(tok_id);

        std::unique_ptr<Viretoken> Parent;
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
    std::unique_ptr<ExprAST> Vireparse::ParseNewExpr()
    {
        getNextToken(tok_new); // consume `new`
        
        auto id_expr=ParseIdExpr();

        std::vector<std::unique_ptr<ExprAST>> args;
        std::unique_ptr<Viretoken> id_name;
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
    std::unique_ptr<ExprAST> Vireparse::ParseDeleteExpr()
    {
        getNextToken(tok_delete);

        auto id_name=copyCurrentToken();
        getNextToken(tok_id);

        return std::make_unique<DeleteExprAST>(std::move(id_name));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseClassAccess(std::unique_ptr<ExprAST> parent)
    {
        getNextToken(tok_dot);
        auto child=ParseIdExpr();

        if(!(child->asttype==ast_var || child->asttype==ast_call || child->asttype==ast_class_access))
        {
            std::cout << "Expected a class member during parsing" << std::endl;
            return nullptr;
        }

        auto cast_child=cast_static<IdentifierExprAST>(std::move(child));

        return std::make_unique<ClassAccessAST>(std::move(parent),std::move(cast_child));
    }

    std::unordered_map<std::string, std::unique_ptr<ExprAST>> Vireparse::ParsePrimitiveBody()
    {
        getNextToken(tok_lbrace);
        std::unordered_map<std::string, std::unique_ptr<ExprAST>> members;

        while(current_token->type!=tok_rbrace)
        {
            if(current_token->type==tok_eof) return LogErrorPB("Expected '}' found end of file");

            std::unique_ptr<ExprAST> member;
            std::string member_name;
            if(current_token->type==tok_union)
            {
                member=ParseUnion();
                member_name=((UnionExprAST*)member.get())->getName();
            }
            else if(current_token->type==tok_struct)
            {
                member=ParseStruct();
                member_name=((StructExprAST*)member.get())->getName();
            }
            else if(current_token->type==tok_id)
            {
                auto type=copyCurrentToken();
                getNextToken(tok_id);

                auto name=copyCurrentToken();
                getNextToken(tok_id);
                getNextToken(tok_semicol);

                member_name=name->value;
                name->value="_"+name->value;
                member=std::make_unique<VariableDefAST>(std::move(name), types::construct(type->value), nullptr);
            }
            else
            {
                std::cout << "Invalid Token: " << current_token->value << std::endl;
                break;
            }
            
            member_name="_"+member_name;
            members.insert(std::make_pair(member_name, std::move(member)));
        }
        
        getNextToken(tok_rbrace);

        return std::move(members);
    }
    std::unique_ptr<ExprAST> Vireparse::ParseUnion()
    {
        getNextToken(tok_union);

        char is_anonymous=1;
        std::unique_ptr<Viretoken> name;
        if(current_token->type==tok_id)
        {
            is_anonymous=0;
            name=copyCurrentToken();
            getNextToken();
        }

        auto body=ParsePrimitiveBody();
        return std::make_unique<UnionExprAST>(std::move(body),std::move(name));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseStruct()
    {
        getNextToken(tok_struct);

        char is_anonymous=1;
        std::unique_ptr<Viretoken> name;
        if(current_token->type==tok_id)
        {
            is_anonymous=0;
            name=copyCurrentToken();
            getNextToken();
        }

        auto body=ParsePrimitiveBody();

        return std::make_unique<StructExprAST>(std::move(body),std::move(name));
    }

    std::unique_ptr<ExprAST> Vireparse::ParseUnsafe()
    {
        getNextToken(tok_unsafe);
        
        auto block=ParseBlock();
        return std::make_unique<UnsafeExprAST>(std::move(block));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseReference()
    {
        getNextToken(tok_reference);

        auto var=ParseIdExpr();
        return std::make_unique<ReferenceExprAST>(std::move(var));
    }

    std::unique_ptr<CodeAST> Vireparse::ParseCode()
    {
        getNextToken();

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
                auto statement=ParsePrimary();
                PreExecutionStatements.push_back(std::move(statement));
                getNextToken(tok_semicol);
            }
        }
        
        return std::make_unique<CodeAST>(std::move(PreExecutionStatements),std::move(Functions),std::move(Classes),std::move(StructUnionDefs));
    }
}