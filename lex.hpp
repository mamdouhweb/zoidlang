#pragma once
#include <boost/optional.hpp>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace zlang {
    class end_of_text : public std::out_of_range {
    public:
        end_of_text() : std::out_of_range{"EOT"} {}
    };

    class bad_token : public std::runtime_error {
    public:
        bad_token(std::size_t idx)
        : std::runtime_error{"bad token"}
        , index{idx} {}

        std::size_t index;
    };

    class bad_escape_sequence : public bad_token {
    public:
        bad_escape_sequence(std::size_t idx) : bad_token{idx} {}
    };

    struct token {
        enum tok_kind {
            EOL,
            INDENT,

            STRING,
            NUMBER,
            IDENTIFIER,

            IMPORT,

            IF,
            ELIF,
            ELSE,

            UNREACHABLE,
        } kind;
        boost::optional<std::string> value;
        std::size_t index;
    };

    class lexer {
    public:
        lexer(std::string code);

        token peek();
        token next();

    private:
        std::string text;
        std::size_t index;

        token mktok(token::tok_kind kind, std::size_t idx);
        token mktok(token::tok_kind kind, std::string value, std::size_t idx);

        char curc();
        char peekc();
        char nextc();
    };
}

