#ifndef __SIMPLE_AST_OBJECTS_NODE_H__
#define __SIMPLE_AST_OBJECTS_NODE_H__

#include <cdk/ast/unary_operation_node.h>

namespace til {

  /**
   * Class for describing objects nodes.
   */
  class objects_node : public cdk::unary_operation_node {
  public:
    objects_node(int lineno, cdk::expression_node *expression) :
      cdk::unary_operation_node(lineno, expression) {
    }

    void accept(basic_ast_visitor *sp, int level) { sp->do_objects_node(this, level); }

  };

} // til

#endif
