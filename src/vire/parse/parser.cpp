#include "../Commons.cpp"

#include "parser.hpp"
#include "../includes.hpp"
#include __VIRE_AST_PATH
#include __VIRE_LEX_PATH

#include <memory>
#include <string>
#include <vector>
#include <cstdarg>

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

    void Vireparse::getNextToken()
    {
        CurTok.reset();
        CurTok=lexer->getToken();

        if(CurTok==nullptr) CurTok=lexer->getToken();
    }
    void Vireparse::getNextToken(int toktype)
    {
        if(CurTok->type!=toktype)
        {
            LogError("Current token type %s does not match type %s, Current token: `%s`",tokToStr(CurTok->type),tokToStr(toktype),CurTok->value.c_str());
        }
        getNextToken();
    }
    std::unique_ptr<Viretoken> Vireparse::copyCurrentToken()
    {
        return std::make_unique<Viretoken>(CurTok->value,CurTok->type,CurTok->line,CurTok->charpos);
    }

    std::vector<std::unique_ptr<ExprAST>> Vireparse::ParseBlock()
    {
        getNextToken(tok_lbrace); // consume '}'

        std::vector<std::unique_ptr<ExprAST>> Stms;
        while(CurTok->type!=tok_rbrace)
        {
            if(CurTok->type==tok_eof)
                return LogErrorVP("Expected '}' found end of file");
            auto stm=ParsePrimary();
            if(!stm) continue;

            if(stm->asttype!=ast_for 
            && stm->asttype!=ast_while 
            && stm->asttype!=ast_unsafe)
                getNextToken(tok_semicol);

            Stms.push_back(std::move(stm));
        }
        getNextToken(tok_rbrace);

        return std::move(Stms);
    }

    std::unique_ptr<ExprAST> Vireparse::ParsePrimary()
    {
        switch(CurTok->type)
        {
            default: 
                return LogError("Unknown token `%s` found when expecting statement,value: %s",tokToStr(CurTok->type),CurTok->value.c_str());
            
            case tok_id: return ParseIdExpr();

            case tok_int: return ParseNumberExpr();
            case tok_float: return ParseNumberExpr();
            case tok_double: return ParseNumberExpr();
            case tok_char: return ParseStrExpr();
            case tok_str: return ParseStrExpr();
            case tok_lbrack: return ParseArrayExpr();

            case tok_lparen: return ParseParenExpr();

            case tok_vardef: return ParseVariableDef();
            case tok_const: return ParseVariableDef();
            case tok_let: return ParseVariableDef();

            case tok_for: return ParseForExpr();
            case tok_while: return ParseWhileExpr();

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
            return nullptr;
        
        return ParseBinopExpr(0,std::move(LHS));
    }

    std::unique_ptr<ExprAST> Vireparse::ParseIdExpr()
    {
        std::unique_ptr<Viretoken> IdName=copyCurrentToken();
        getNextToken(tok_id);

        if(CurTok->type != tok_lparen) // if it is not a function call
        {
            if(CurTok->type == tok_equal)
                return ParseVariableAssign(std::move(IdName));
            else
            {    
                auto var=std::make_unique<VariableExprAST>(std::move(IdName));
                if(CurTok->type==tok_dot)
                    return ParseClassAccess(std::move(var));
                return std::move(var);
            }
        }
        
        getNextToken(tok_lparen); // consume '('

        std::vector<std::unique_ptr<ExprAST>> Args;
        if(CurTok->type!=tok_rparen)
        {
            while(1)
            {
                if(auto Arg=ParseExpression())
                {
                    Args.push_back(std::move(Arg));
                }
                else
                    return nullptr;
                
                if(CurTok->type==tok_rparen)
                    break;
                
                if(CurTok->type!=tok_comma)
                    return LogError("Expected ')' or ',' in function call arg list");
                
                getNextToken(tok_comma);
            }
        }

        getNextToken(tok_rparen); // consume ')'

        auto call=std::make_unique<CallExprAST>(std::move(std::move(IdName)),std::move(Args));

        if(CurTok->type==tok_dot)
            return ParseClassAccess(std::move(call));
        
        return std::move(call);
    }

    std::unique_ptr<ExprAST> Vireparse::ParseNumberExpr()
    {
        auto token=copyCurrentToken();
        if(CurTok->type==tok_int)
        {
            auto Result=std::make_unique<IntExprAST>(std::stoi(CurTok->value),std::move(token));
            getNextToken(tok_int);
            return std::move(Result);
        }
        else if(CurTok->type==tok_float)
        {
            auto Result=std::make_unique<FloatExprAST>(std::stof(CurTok->value),std::move(token));
            getNextToken(tok_float);
            return std::move(Result);
        }
        else if(CurTok->type==tok_double)
        {
            auto Result=std::make_unique<DoubleExprAST>(std::stod(CurTok->value),std::move(token));
            getNextToken(tok_double);
            return std::move(Result);
        }

        return nullptr;
    }
    std::unique_ptr<ExprAST> Vireparse::ParseStrExpr()
    {
        auto token=copyCurrentToken();
        if(CurTok->type==tok_char)
        {
            auto Result=std::make_unique<CharExprAST>(CurTok->value.at(0),std::move(token));
            getNextToken(tok_char);
            return std::move(Result);
        }
        else // assume the tok is tok_str
        {
            auto Result=std::make_unique<StrExprAST>(std::string(CurTok->value),std::move(token));
            getNextToken(tok_str);
            return Result;
        }
    }
    std::unique_ptr<ExprAST> Vireparse::ParseArrayExpr()
    {
        getNextToken(tok_lbrack);

        std::vector<std::unique_ptr<ExprAST>> Elements;
        
        if(CurTok->type!=tok_rbrack)
        {
            while(1)
            {
                auto elm=ParseExpression();
                Elements.push_back(std::move(elm));

                if(CurTok->type==tok_rbrack)
                    break;
                
                if(CurTok->type!=tok_comma)
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
        if(CurTok->type!=tok_rparen)
            return LogError("Expected ')' left-parenthesis");
        
        getNextToken(tok_rparen); // consume ')'
        return stm;
    }

    std::unique_ptr<ExprAST> Vireparse::ParseBinopExpr(int ExprPrec, std::unique_ptr<ExprAST> LHS)
    {
        while(1)
        {  
            int prec=config->getBinopPrecedence(CurTok->value);

            //Debug line, uncomment to use:
            //std::cout << *CurTok << std::to_string(prec) << std::endl;
 
            if(prec<ExprPrec)
                return LHS;
            
            auto Binop=copyCurrentToken();
            getNextToken(); // consume the binop
            
            auto RHS=ParsePrimary();
            if(!RHS)
                return nullptr;
            
            int nextprec=config->getBinopPrecedence(CurTok->value.c_str());
            if(prec<nextprec)
            {
                RHS=ParseBinopExpr(prec+1,std::move(RHS));
                if(!RHS)
                    return nullptr;
            }

            LHS=std::make_unique<BinaryExprAST>(std::move(Binop),std::move(LHS),std::move(RHS));
            if(CurTok->type==tok_eof)
            {
                return std::move(LHS);
            }
        }
    }

    std::unique_ptr<ExprAST> Vireparse::ParseVariableDef()
    {
        unsigned char isconst=0;
        unsigned char islet=0;
        if(CurTok->type==tok_const)
            isconst=1;
        else if(CurTok->type==tok_let)
            islet=1;
        
        if(!isconst && !islet) 
            getNextToken(tok_vardef); // consume `var`
        else
            getNextToken(); // consume `let` / `const`

        auto varName=copyCurrentToken();    
        getNextToken(tok_id);

        unsigned char isarr=0;
        int arr_size=0;
        if(CurTok->type==tok_lbrack)
        {
            isarr=1;
            getNextToken(tok_lbrack);
            if(CurTok->type!=tok_rbrack)
            {
                std::string arr_size=CurTok->value;
                getNextToken(tok_int);
                arr_size=std::stoi(arr_size);
            }
            getNextToken(tok_rbrack);
        }

        std::string typeName;
        if(CurTok->type!=tok_colon) typeName="auto";
        else
        {
            getNextToken(); // consume the colon
            typeName=std::string(CurTok->value);
            getNextToken(tok_id);
        }

        std::unique_ptr<ExprAST> value=nullptr;
        if(CurTok->type==tok_equal)
        {
            getNextToken(tok_equal);
            if(isarr)
                value=ParseArrayExpr();
            else
                value=ParseExpression();
        }

        return std::make_unique<VariableDefAST>(std::move(varName),typeName,std::move(value),isconst,islet,isarr,arr_size);
    }
    std::unique_ptr<ExprAST> Vireparse::ParseVariableAssign(std::unique_ptr<Viretoken> varName)
    {
        getNextToken(tok_equal);
        auto value=ParseExpression();

        return std::make_unique<VariableAssignAST>(std::move(varName), std::move(value));
    }
    std::unique_ptr<TypedVarAST> Vireparse::ParseTypedVar()
    {
        auto typeName=copyCurrentToken();
        getNextToken(tok_id);
        auto varName=copyCurrentToken();
        getNextToken(tok_id);

        return std::make_unique<TypedVarAST>(std::move(varName),std::move(typeName));
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
        char is_break=1;
        if(CurTok->type==tok_continue) is_break=0;

        getNextToken();

        char has_stm=0;
        std::unique_ptr<ExprAST> stm;
        if(CurTok->type!=tok_semicol)
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
        if(CurTok->type!=tok_id)
            return LogErrorP("Expected function name in prototype");
        
        auto fnName=copyCurrentToken();
        getNextToken(tok_id);

        if(CurTok->type!=tok_lparen)
            return LogErrorP("Expected '(' in prototype after name");
        getNextToken(); // consume '('

        std::vector<std::unique_ptr<VariableDefAST>> Args;
        while(CurTok->type==tok_id)
        {
            std::unique_ptr<Viretoken> varName=copyCurrentToken();
            getNextToken(tok_id); // consume id
            if(CurTok->type!=tok_colon) 
                return LogErrorP("Expected ':' for type specifier after arg name");
            getNextToken(tok_colon); // consume colon

            unsigned char isconst=0;
            if(CurTok->type==tok_const)
            {
                isconst=1;
                getNextToken();
            }

            std::string typeName(CurTok->value);
            getNextToken(); // consume typename
            auto var=std::make_unique<VariableDefAST>(std::move(varName),typeName,nullptr,isconst);
            Args.push_back(std::move(var));

            if(CurTok->type!=tok_comma)
            {
                break;
            }
            getNextToken(tok_comma); // consume comma
        }

        getNextToken(tok_rparen);

        std::unique_ptr<Viretoken> returnType;
        if(CurTok->type==tok_rarrow || CurTok->type==tok_returns)
        {
            getNextToken();
            returnType=copyCurrentToken();
            getNextToken(tok_id);
        }
        else
            returnType=nullptr;

        return std::make_unique<PrototypeAST>(std::move(fnName),std::move(Args),std::move(returnType));
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
    
        auto Stms=ParseBlock();

        return std::make_unique<FunctionAST>(std::move(proto),std::move(Stms));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseReturn()
    {
        getNextToken(tok_return);

        std::vector<std::unique_ptr<ExprAST>> vals;
        while(CurTok->type!=tok_semicol)
        {
            if(CurTok->type==tok_eof)   return LogError("Expected ';' found end of file");
            auto value=ParseExpression();
            vals.push_back(std::move(value));
            if(CurTok->type==tok_comma)
                getNextToken(tok_comma);
            else
                break;
        }

        return std::make_unique<ReturnExprAST>(std::move(vals));
    }

    std::unique_ptr<ExprAST> Vireparse::ParseIfExpr()
    {
        getNextToken(tok_if);
        getNextToken(tok_lparen);
        auto cond=ParseExpression();
        getNextToken(tok_rparen);
        auto mthenStm=ParseBlock();

        std::vector<std::unique_ptr<IfThenExpr>> elseStms;
        while(CurTok->type==tok_else)
        {
            getNextToken(tok_else);
            if(CurTok->type==tok_if)
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
        if(CurTok->type==tok_lparen)
        {
            getNextToken(tok_lparen);
            Parent=copyCurrentToken();
            getNextToken(tok_id);
            getNextToken(tok_rparen);
        }
        else if(CurTok->type==tok_colon)
        {
            getNextToken(tok_colon);
            Parent=copyCurrentToken();
            getNextToken(tok_id);
        }
        else if(CurTok->type==tok_extends)
        {
            getNextToken(tok_extends);
            Parent=copyCurrentToken();
            getNextToken(tok_id);
        }

        getNextToken(tok_lbrace);

        std::vector<std::unique_ptr<FunctionBaseAST>> funcs;
        std::vector<std::unique_ptr<VariableDefAST>> vars;
        while(CurTok->type!=tok_rbrace)
        {
            if(CurTok->type==tok_eof) return LogErrorC("Expected '}' found end of file");

            if(CurTok->type==tok_func)
            {
                auto func=ParseFunction();
                funcs.push_back(std::move(func));
            }
            else if(CurTok->type==tok_proto)
            {
                auto proto=ParseFunction();
                funcs.push_back(std::move(proto));
                getNextToken(tok_semicol);
            }
            else if(CurTok->type==tok_vardef || CurTok->type==tok_const || CurTok->type==tok_let)
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

        std::vector<std::unique_ptr<ExprAST>> Args;
        std::unique_ptr<Viretoken> IdName;
        if(id_expr->asttype==ast_var)
        {
            std::unique_ptr<VariableExprAST> var(static_cast<VariableExprAST*>(id_expr.release()));
            IdName=var->moveToken();
        }
        else if(id_expr->asttype==ast_call)
        {
            std::unique_ptr<CallExprAST> call(static_cast<CallExprAST*>(id_expr.release()));
            IdName=call->moveToken();
            Args=call->moveArgs();
        }

        return std::make_unique<NewExprAST>(std::move(IdName),std::move(Args));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseDeleteExpr()
    {
        getNextToken(tok_delete);

        auto IdName=copyCurrentToken();
        getNextToken(tok_id);

        return std::make_unique<DeleteExprAST>(std::move(IdName));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseClassAccess(std::unique_ptr<ExprAST> parent)
    {
        getNextToken(tok_dot);
        auto child=ParseIdExpr();

        return std::make_unique<ClassAccessAST>(std::move(parent),std::move(child));
    }

    std::vector<std::unique_ptr<ExprAST>> Vireparse::ParseNativeBody()
    {
        getNextToken(tok_lbrace);
        std::vector<std::unique_ptr<ExprAST>> members;
        while(CurTok->type!=tok_rbrace)
        {
            if(CurTok->type==tok_eof) return LogErrorVP("Expected '}' found end of file");

            std::unique_ptr<ExprAST> member;
            if(CurTok->type==tok_union)
                member=ParseUnion();
            else if(CurTok->type==tok_struct)
                member=ParseStruct();
            else
            {
                member=ParseTypedVar();
                getNextToken(tok_semicol);
            }
            members.push_back(std::move(member));
        }
        getNextToken(tok_rbrace);

        return std::move(members);
    }
    std::unique_ptr<ExprAST> Vireparse::ParseUnion()
    {
        getNextToken(tok_union);

        char is_anonymous=1;
        std::unique_ptr<Viretoken> Name;
        if(CurTok->type==tok_id)
        {
            is_anonymous=0;
            Name=copyCurrentToken();
            getNextToken();
        }

        auto body=ParseNativeBody();

        if(is_anonymous)
            return std::make_unique<UnionExprAST>(std::move(body));
        else
            return std::make_unique<UnionExprAST>(std::move(body),std::move(Name));
    }
    std::unique_ptr<ExprAST> Vireparse::ParseStruct()
    {
        getNextToken(tok_struct);

        char is_anonymous=1;
        std::unique_ptr<Viretoken> Name;
        if(CurTok->type==tok_id)
        {
            is_anonymous=0;
            Name=copyCurrentToken();
            getNextToken();
        }

        auto body=ParseNativeBody();

        if(is_anonymous)
            return std::make_unique<StructExprAST>(std::move(body));
        else
            return std::make_unique<StructExprAST>(std::move(body),std::move(Name));
    }

    std::unique_ptr<ExprAST> Vireparse::ParseUnsafe()
    {
        getNextToken(tok_unsafe);
        
        auto block=ParseBlock();
        return std::make_unique<UnsafeAST>(std::move(block));
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
        while(CurTok->type!=tok_eof)
        {
            if(CurTok->type==tok_class)
            {
                auto class_ast=ParseClass();
                Classes.push_back(std::move(class_ast));
            }
            else if(CurTok->type==tok_func)
            {
                auto func_ast=ParseFunction();
                Functions.push_back(std::move(func_ast));
            }
            else if(CurTok->type==tok_proto)
            {
                auto proto_ast=ParseProto();
                Functions.push_back(std::move(proto_ast));
            }
            else if(CurTok->type==tok_extern)
            {
                auto extern_ast=ParseExtern();
                Functions.push_back(std::move(extern_ast));
            }
            else if(CurTok->type==tok_struct)
            {
                auto struct_ast=ParseStruct();
                StructUnionDefs.push_back(std::move(struct_ast));
            }
            else if(CurTok->type==tok_union)
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