#ifndef __SIMPLE_AST_WITH_NODE_H__
#define __SIMPLE_AST_WITH_NODE_H__

#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>

namespace til {

  /**
   * Class for describing the with intruction nodes.
   */
  class with_node : public cdk::expression_node {
    cdk::expression_node *_function;
    cdk::expression_node *_low;
    cdk::expression_node *_high;
    cdk::sequence_node *_vec;

  public:
    call_node(int lineno, cdk::expression_node *function, cdk::expression_node *low, cdk::expression_node *high, cdk::sequence_node *vec) :
      cdk::expression_node(lineno), _function(function), _low(low), _high(high), _vec(vec) {}

    cdk::expression_node *function() { return _function; }

    cdk::expression_node *low() { return _low; }

    cdk::expression_node *high() { return _high; }

    cdk::sequence_node *vec() { return _vec; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_with_node(this, level);
    }

  };

} // til

#endif
