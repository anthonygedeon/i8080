#ifndef ASM_H
#define ASM_H

enum class TokenType {
    opcode,
    register,
    identifier,
    integer,
    string,
    colon,
    comma,
    eof
};

class Token {
    Token();
};

class Scanner {
    Scanner();
};

#endif
