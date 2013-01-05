#include "lex.hpp"
#include <unordered_map>
#include <utility>

namespace {
    bool is_identifier_start(char c) {
        return c == '_'
            || (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z');
    }

    bool is_identifier_rest(char c) {
        return is_identifier_start(c)
            || (c >= '0' && c <= '9');
    }

    bool is_number_start(char c) {
        return c >= '0' && c <= '9';
    }

    bool is_number_rest(char c) {
        return is_number_start(c)
            || c == '.'
            || c == '#'
            || (c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z');
    }
}

zlang::lexer::lexer(std::string code)
: text{std::move(code += "\n")}
, index{0} {}

zlang::token zlang::lexer::peek() {
    auto i = index;
    auto tok = next();
    index = i;
    return tok;
}

zlang::token zlang::lexer::next() {
    unsigned indent = 0;
    while (curc() == ' ') {
        ++indent;
        nextc();
        if (indent == 4) {
            return mktok(token::INDENT, index - 4);
        }
    }

    if (curc() == '#') {
        while (nextc() != '\n');
    }

    if (curc() == '\n') {
        nextc();
        return mktok(token::EOL, index - 1);
    }

    if (is_identifier_start(curc())) {
        auto idx = index;
        std::string identifier;
        do {
            identifier.push_back(curc());
        } while (is_identifier_rest(nextc()));

        std::unordered_map<std::string, token::tok_kind> kinds{
            {"import", token::IMPORT},

            {"if", token::IF},
            {"elif", token::ELIF},
            {"else", token::ELSE},

            {"unreachable", token::UNREACHABLE},
        };

        auto it = kinds.find(identifier);
        if (it != kinds.end()) {
            return mktok(it->second, idx);
        }

        return mktok(token::IDENTIFIER, identifier, idx);
    }

    if (curc() == '"') {
        auto idx = index;
        std::string string;
        while (nextc() != '"') {
            if (curc() == '\\') {
                char c;
                switch (nextc()) {
                    case 'a': c = '\a'; break;
                    case 'b': c = '\b'; break;
                    case 'f': c = '\f'; break;
                    case 'n': c = '\n'; break;
                    case 'r': c = '\r'; break;
                    case 't': c = '\t'; break;
                    case 'v': c = '\v'; break;
                    case '\'': c = '\''; break;
                    case '"': c = '"'; break;
                    case '\\': c = '\\'; break;
                    default: throw bad_escape_sequence{index};
                }
                string.push_back(c);
            } else {
                string.push_back(curc());
            }
        }
        nextc();
        return mktok(token::STRING, string, idx);
    }

    if (is_number_start(curc())) {
        auto idx = index;
        std::string number;
        do {
            number.push_back(curc());
        } while (is_number_rest(nextc()));
        return mktok(token::NUMBER, number, idx);
    }

    throw bad_token{index};
}

zlang::token zlang::lexer::mktok(token::tok_kind kind, std::size_t idx) {
    return {kind, boost::none_t{}, idx};
}

zlang::token zlang::lexer::mktok(token::tok_kind kind, std::string value,
                   std::size_t idx) {
    return {kind, value, idx};
}

char zlang::lexer::curc() {
    return text[index];
}

char zlang::lexer::peekc() {
    if (index + 1 >= text.length()) {
        throw end_of_text{};
    }
    return text[index + 1];
}

char zlang::lexer::nextc() {
    auto c = peekc();
    ++index;
    return c;
}

