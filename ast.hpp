#pragma once

#include <memory>
#include <vector>

namespace zlang {
    namespace ast {
        struct statement;
        struct switch_statement;
        struct expression;

        struct statement {
            statement() = default;
            statement(statement&&) = default;
            virtual ~statement() = 0;
        };
        inline statement::~statement() {}

        struct if_statement : statement {
            struct branch {
                branch() = default;
                branch(branch&&) noexcept = default;
                std::unique_ptr<expression> condition;
                std::unique_ptr<statement> body;
            };

            std::vector<branch> branch;
        };

        struct expression {
            expression() = default;
            expression(expression&&) = default;
            virtual ~expression() = 0;
        };
        inline expression::~expression() {}
    }
}

