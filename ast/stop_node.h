#ifndef __TIL_AST_STOP_NODE__
#define __TIL_AST_STOP_NODE__

#include <cdk/ast/basic_node.h>

namespace til {

/**
 * Class for describing stop nodes.
 */
class stop_node : public cdk::basic_node {
    int _n;

  public:
    stop_node(int lineno, int n) : cdk::basic_node(lineno), _n(n) {}

    int n() { return _n; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_stop_node(this, level);
    }
};
} // namespace til

#endif
