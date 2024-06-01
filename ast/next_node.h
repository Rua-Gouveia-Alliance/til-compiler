#ifndef __TIL_AST_NEXT_NODE__
#define __TIL_AST_NEXT_NODE__

#include <cdk/ast/basic_node.h>

namespace til {

/**
 * Class for describing next nodes.
 */
class next_node : public cdk::basic_node {
    int _n;

  public:
    next_node(int lineno, int n) : cdk::basic_node(lineno), _n(n) {}

    int n() { return _n; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_next_node(this, level);
    }
};
} // namespace til

#endif
