#ifndef __SIMPLE_AST_CALL_NODE_H__
#define __SIMPLE_AST_CALL_NODE_H__

#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>

namespace til {

  /**
   * Class for describing function call nodes.
   */
  class call_node : public cdk::expression_node {
    cdk::expression_node *_function_ptr;
    cdk::sequence_node *_args;

  public:
    call_node(int lineno,
                       cdk::expression_node *function_ptr,
                       cdk::sequence_node *args) :
      cdk::expression_node(lineno), _function_ptr(function_ptr), _args(args) {
    }

    cdk::expression_node *function_ptr() { return _function_ptr; }

    cdk::sequence_node *args() { return _args; }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_call_node(this, level);
    }

  };

} // til

#endif
