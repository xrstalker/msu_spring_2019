#include <iostream>
#include <vector>
#include <string>
#include <exception>

#include <cstdint>
#include <cctype>

typedef enum {
    TOKEN_INVALID,
    TOKEN_NUMBER,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_PLUS,
    TOKEN_MIN,
    TOKEN_END,
} token_type_t;

struct Token 
{
    token_type_t type;
    int64_t num;

    Token(token_type_t t, int64_t n=0): type(t), num(n) {
    }

    std::string repr() const
    {
        switch (type) {
            case TOKEN_INVALID: 
                return "Invalid";
            case TOKEN_NUMBER: 
                return std::to_string(num);
            case TOKEN_MUL:
                return "*";
            case TOKEN_DIV:
                return "/";
            case TOKEN_PLUS:
                return "+";
            case TOKEN_MIN:
                return "-";
            case TOKEN_END:
                return "End";
        }
        return "Broken";
    }
};

class Error: public std::exception
{
protected:
    const std::string cause;

public:
    Error(const char *c): cause(c) { }
    virtual std::string repr() const
    {
        return "Error: " + cause;
    }
};

class BadToken: public Error 
{
    const Token token;

public:
    BadToken(const Token &t, const char *c): Error(c), token(t) { }
    
    std::string repr() const
    {
        if (cause.size()) {
            return "BadToken(" + token.repr() + "): " + cause;
        } else {
            return "BadToken(" + token.repr() + ")";
        }
    }
};


class Tokenizer 
{
    const std::string s;
    std::string::const_iterator c;

    const static int64_t NEGAIVE_OVERFLOW = INT64_MIN / 10 - 1;
    const static int64_t POSITIVE_OVERFLOW = INT64_MAX / 10 + 1;

    token_type_t operation_to_type(char c) 
    {
        if (c == '*') 
            return TOKEN_MUL;
        if (c == '/')
            return TOKEN_DIV;
        if (c == '+')
            return TOKEN_PLUS;
        if (c == '-')
            return TOKEN_MIN;
        throw Error("invalid operation");
    }

    int64_t get_positive()
    {
        int64_t n = *c++ - '0';
        if (c < s.end() && std::isdigit(*c) && n == 0) 
            throw Error("leading zeros");
        while (c < s.end() && std::isdigit(*c)) {
            if (n > POSITIVE_OVERFLOW || INT64_MAX - n*10 < *c - '0')
                throw Error("positive overflow");
            n = n*10 + *c++ - '0';
        }
        return n;
    }
    int64_t get_negative() 
    {
        int64_t n = -(*c++ - '0');
        if (c < s.end() && std::isdigit(*c) && n == 0) 
            throw Error("leading zeros");
        while (c < s.end() && std::isdigit(*c)) {
            if (n < NEGAIVE_OVERFLOW || n*10 - INT64_MIN < *c - '0')
                throw Error("negative overflow");
            n = n*10 - (*c++ - '0');
        }
        return n;
    }

    Tokenizer(const Tokenizer &tok); 
public:
    Tokenizer(const char *str): s(str), c(s.begin()) { }
    Token get(bool negative=false) 
    {
        while (c < s.end() && std::isspace(*c))
            ++c;
        
        if (c == s.end())
            return Token(TOKEN_END);
        
        if (std::isdigit(*c)) {
            if (negative) {
                return Token(TOKEN_NUMBER, get_negative());
            } else {
                return Token(TOKEN_NUMBER, get_positive());
            }
        }
        return Token(operation_to_type(*c++));
    }
};

class Calc 
{
    Tokenizer tokenizer;
    Token token;

    int64_t get_number() 
    {
        Token t = tokenizer.get();
        if (t.type == TOKEN_MIN)
            t = tokenizer.get(true);
        if (t.type == TOKEN_NUMBER)
            return t.num;
        throw BadToken(t, "expect number");
    }

    int64_t l1_eval() 
    {
        int64_t val = l2_eval();
        while (true) {
            switch (token.type) {
                case TOKEN_PLUS:
                    val += l2_eval();
                    break;
                case TOKEN_MIN:
                    val -= l2_eval();
                    break;
                default:
                    return val;
            }
        }
    }

    int64_t l2_eval()
    {
        int64_t val = get_number();
        while (true) {
            token = tokenizer.get();
            switch (token.type) {
                case TOKEN_MUL:
                    val *= get_number();
                    break;
                case TOKEN_DIV:
                    if (int64_t d = get_number())
                        val /= d;
                    else
                        throw Error("division by zero");
                    break;
                default:
                    return val;
            }
        }
    }

public:
    Calc(const char *s): tokenizer(s), token(TOKEN_INVALID) { }

    int64_t evaluate()
    {
        int64_t val = l1_eval();
        if (token.type != TOKEN_END)
            throw BadToken(token, "evaluation was stopped on token");
        return val;
    }
};

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "error" << std::endl;
        return 1;
    }
    try {
        Calc calc(argv[1]);
        std::cout << calc.evaluate() << std::endl;
    }
    catch (Error &e) {
      //std::cerr << e.repr() << std::endl;
        std::cout << "error" << std::endl;
        return 1;
    }
    return 0;
}
