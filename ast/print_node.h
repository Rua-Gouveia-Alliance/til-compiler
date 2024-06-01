#ifndef __SIMPLE_AST_PRINT_NODE_H__
#define __SIMPLE_AST_PRINT_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace til {

/**
 * Class for describing print nodes.
 */
class print_node : public cdk::basic_node {
    cdk::sequence_node *_expressions;
    bool _ln;

  public:
    print_node(int lineno, cdk::sequence_node *expressions, bool ln = false)
        : cdk::basic_node(lineno), _expressions(expressions), _ln(ln) {}

    cdk::sequence_node *expressions() { return _expressions; }

    bool ln() { return _ln; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_print_node(this, level);
    }
};

} // namespace til

#endif
