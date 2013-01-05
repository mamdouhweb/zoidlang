#pragma once

#include "ast.hpp"
#include "lex.hpp"

namespace zlang {
    ast::if_statement parse_if_stmt(lexer& lex);
}

