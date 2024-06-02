#ifndef __SIMPLE_AST_FOR_NODE_H__
#define __SIMPLE_AST_FOR_NODE_H__

#include <cdk/ast/expression_node.h>

namespace til {

  /**
   * Class for describing for-cycle nodes.
   */
  class for_node : public cdk::basic_node {
    cdk::expression_node *_assignment;
    cdk::expression_node *_condition;
    cdk::expression_node *_increment;
    cdk::basic_node *_block;

  public:
    for_node(int lineno, cdk::expression_node *assignment, cdk::expression_node *condition, cdk::expression_node *increment, cdk::basic_node *block) :
        basic_node(lineno), _assignment(assignment), _condition(condition), _increment(increment), _block(block) {
    }

    cdk::expression_node *assignment() { return _assignment; }
    
    cdk::expression_node *condition() { return _condition; }

    cdk::expression_node *increment() { return _increment; }

    cdk::basic_node *block() { return _block; }

    void accept(basic_ast_visitor *sp, int level) { sp->do_for_node(this, level); }

  };

} // til

#endif
