#ifndef __TIL_AST_RETURN_NODE__
#define __TIL_AST_RETURN_NODE__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>

namespace til {

/**
 * Class for describing return nodes.
 */
class return_node : public cdk::basic_node {
    cdk::expression_node *_expression;

  public:
    return_node(int lineno, cdk::expression_node *expression)
        : cdk::basic_node(lineno), _expression(expression) {}

    cdk::expression_node *expression() { return _expression; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_return_node(this, level);
    }
};
} // namespace til

#endif
