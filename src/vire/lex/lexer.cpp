#pragma once

#include <iostream> // cout, endl
#include <string> // string
#include <memory> // unique_ptr
#include <cctype> // isspace
#include <cstddef> // size_t

#include "token.hpp"
#include "token.cpp"

#include "vire/errors/include.hpp"

namespace vire
{

class VLexer
{
protected:
    char cur;
    std::size_t indx;
    std::size_t line;
    std::size_t charpos;
    errors::ErrorBuilder* builder; // error builder
public:
    bool jit;
    std::string code;
    std::size_t len;

    VLexer(std::string code, errors::ErrorBuilder* builder)
    : builder(builder), jit(false)
    {
        this->code=code;
        reset();
    }

    void reset()
    {
        this->cur=' ';
        this->indx=-1;
        this->line=0;

        if(!jit)
        {
            this->len=code.length();
        }
        else
        {
            this->code="";
            this->len=0;
        }

        this->charpos=-1;
    }
    char getNext(char move_amt=0)
    {
        if(this->indx+move_amt+1==this->len)
            return EOF;
        
        this->indx+=move_amt+1;
        this->charpos++;
        
        return this->code.at(this->indx);
    }
    void advanceNext(char move_amt=0)
    {
        this->cur=getNext(move_amt);
    }

    char peekNext(char amt)
    {
        if(this->indx+amt>this->len-1)  return EOF;

        return this->code.at(this->indx+amt);
    }

    std::string gatherId()
    {
        std::string id="";

        while(isalnum(this->cur) || this->cur=='_')
        {
            if(this->cur==EOF)
                break;
            id+=this->cur;
            advanceNext();   
        }

        return id;
    }
    std::unique_ptr<VToken> gatherNum()
    {
        std::string numstr;
        int ttype;

        while(isdigit(this->cur))
        {
            numstr+=this->cur;
            advanceNext();
        }

        ttype=tok_int;

        if(this->cur=='.')
        {
            advanceNext();
            numstr+=".";
        }
        else
        {
            return makeTokenInplace(numstr, ttype);
        }

        if(!isdigit(this->cur))
        {
            std::cout << "Expected integer literal after decimal point" << std::endl;
        }

        while(isdigit(this->cur))
        {
            numstr+=this->cur;
            advanceNext();
        }

        if(this->cur=='f' || this->cur=='F')
        {
            ttype=tok_float;
            advanceNext();
        }
        else if(this->cur=='d' || this->cur=='D')
        {
            ttype=tok_double;
            advanceNext();
        }
        else
        {
            ttype=tok_float;
        }

        return makeTokenInplace(numstr,ttype);
    }
    std::unique_ptr<VToken> gatherChar()
    {
        std::string ch;
        advanceNext(); // eat `'`
        
        ch+=this->cur; // set ch to char
        
        advanceNext(); // eat the char
        advanceNext(); // eat the `'`

        return makeTokenInplace(ch, tok_char);
    }
    std::unique_ptr<VToken> gatherStr()
    {
        std::string str="";
        advanceNext(); // consume the start d-quote

        while(this->cur!='\"')
        {
            str+=this->cur;
            advanceNext();

            if(this->cur==EOF)
            {
                builder->addError<errors::lex_unknown_char>(code, ' ', '\0', line, charpos);
                return nullptr;
            }
        }

        advanceNext(); // consume the end d-quote

        return makeTokenInplace(str, tok_str);
    }

    std::unique_ptr<VToken> makeTokenInplace(std::string value, int type)
    {
        auto tok=std::make_unique<VToken>(value, type, this->line, this->charpos);
        return std::move(tok);
    }
    std::unique_ptr<VToken> makeToken(std::string value, int type, char move=0)
    {
        this->cur=this->getNext(move);
        auto tok=this->makeTokenInplace(value,type);

        return std::move(tok);
    }
    std::unique_ptr<VToken> makeToken(const char* value, int type, char move=0)
    {
        this->cur=this->getNext(move);
        auto tok=this->makeTokenInplace(std::string(value),type);

        return std::move(tok);
    }

    std::unique_ptr<VToken> getToken()
    {
        while(isspace(this->cur))
        {
            if(this->cur=='\n' || this->cur=='\r')
            {
                ++this->line;
                this->charpos=0;
            }
            else
            {
                
                ++this->charpos;
            }
            
            advanceNext();
        }
        // Checks
        if(isalpha(this->cur) || this->cur=='_')
        {
            auto id_str=gatherId();
            int toktype=0;
            std::unique_ptr<VToken> tok;
            if(id_str=="func")
            {
                toktype=tok_func;
            }
            else if(id_str=="and")
            {
                toktype=tok_and;
            }
            else if(id_str=="or")
            {
                toktype=tok_or;
            }
            else if(id_str=="if")
            {
                toktype=tok_if;
            }
            else if(id_str=="else")
            {
                toktype=tok_else;
            }
            else if(id_str=="var")
            {
                toktype=tok_vardef;
            }
            else if(id_str=="let")
            {
                toktype=tok_let;
            }
            else if(id_str=="const")
            {
                toktype=tok_const;
            }
            else if(id_str=="true")
            {
                toktype=tok_true;
            }
            else if(id_str=="false")
            {
                toktype=tok_false;
            }
            else if(id_str=="as")
            {
                toktype=tok_as;
            }
            else if(id_str=="new")
            {
                toktype=tok_new;
            }
            else if(id_str=="delete")
            {
                toktype=tok_delete;
            }
            else if(id_str=="class")
            {
                toktype=tok_class;
            }
            else if(id_str=="union")
            {
                toktype=tok_union;
            }
            else if(id_str=="struct")
            {
                toktype=tok_struct;
            }
            else if(id_str=="extern")
            {
                toktype=tok_extern;
            }
            else if(id_str=="for")
            {
                toktype=tok_for;
            }
            else if(id_str=="while")
            {
                toktype=tok_while;
            }
            else if(id_str=="return")
            {
                toktype=tok_return;
            }
            else if(id_str=="break")
            {
                toktype=tok_break;
            }
            else if(id_str=="continue")
            {
                toktype=tok_continue;
            }
            else if(id_str=="returns")
            {
                toktype=tok_returns;
            }
            else if(id_str=="proto")
            {
                toktype=tok_proto;
            }
            else if(id_str=="extends")
            {
                toktype=tok_extends;
            }
            else if(id_str=="try")
            {
                toktype=tok_try;
            }
            else if(id_str=="catch")
            {
                toktype=tok_catch;
            }
            else if(id_str=="except")
            {
                toktype=tok_catch;
            }
            else if(id_str=="unsafe")
            {
                toktype=tok_unsafe;
            }
            else if(id_str=="constructor")
            {
                toktype=tok_constructor;
            }
            else
            {
                toktype=tok_id;
            }

            tok=makeTokenInplace(id_str, toktype);
            return std::move(tok);
        }

        if(isdigit(this->cur))
            return gatherNum();

        char peek=peekNext((char)1);

        switch(this->cur)
        {
            case ';': return makeToken(";",tok_semicol);

            case '{': return makeToken("{",tok_lbrace);
            case '}': return makeToken("}",tok_rbrace);
            case '[': return makeToken("[",tok_lbrack);
            case ']': return makeToken("]",tok_rbrack);
            case '(': return makeToken("(",tok_lparen);
            case ')': return makeToken(")",tok_rparen);
            case ':': return makeToken(":",tok_colon);
            case ',': return makeToken(",",tok_comma);

            case '=': {
                if(peek=='=') return makeToken("==",tok_dequal,1);
                return makeToken("=",tok_equal);
            }

            case '+': {
                if(peek=='+') return makeToken("++",tok_incr,1);
                else if(peek=='=') return makeToken("+=",tok_pluseq,1);
                return makeToken("+",tok_plus);
            }
            case '-': {
                if(peek=='-') return makeToken("--",tok_decr,1);
                else if(peek=='>') return makeToken("->",tok_rarrow,1);
                else if(peek=='=') return makeToken("-=",tok_minuseq,1);
                return makeToken("-",tok_minus);
            }
            case '*': {
                if(peek=='=') return makeToken("*=",tok_muleq,1);
                return makeToken("*",tok_mul);
            }
            case '/': {
                if(peek=='=') return makeToken("/=",tok_diveq,1);
                return makeToken("/",tok_div);
            }
            case '%': {
                if(peek=='=') return makeToken("%=",tok_modeq,1);
                return makeToken("%",tok_mod);
            }

            case '|': {
                if(peek=='|') return makeToken("||",tok_or,1);
                return nullptr;
            }

            case '&': {
                if(peek=='&') return makeToken("&&",tok_and,1);
                return makeToken("&",tok_reference);
            }

            case '<':{
                if(peek=='=')   return makeToken("<=",tok_lesseq,1);
                return makeToken("<",tok_lessthan);
            }
            case '>':{
                if(peek=='=')   return makeToken(">=",tok_moreeq,1);
                return makeToken(">",tok_morethan);
            }
            case '!':{
                if(peek=='=')   return makeToken("!=",tok_nequal,1);
                return makeToken("!",tok_not);
            }

            case '.': return makeToken(".",tok_dot);

            case '\'': return gatherChar();
            case '"':  return gatherStr();

            case EOF: return makeToken("",tok_eof);

            default: {
                builder->addError<errors::lex_unknown_char>(this->code, this->cur,' ', this->line, this->charpos);
                advanceNext();
                return nullptr;
            }
        }
    }
    std::unique_ptr<VToken> getToken(std::string str)
    {
        auto old_str=code;
        code=str;
        auto tkn=getToken();
        old_str=code;
        return std::move(tkn);
    }
};

}