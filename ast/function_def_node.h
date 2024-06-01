#ifndef __SIMPLE_AST_FUNCTION_DEF_NODE_H__
#define __SIMPLE_AST_FUNCTION_DEF_NODE_H__

#include "block_node.h"
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/types/basic_type.h>

namespace til {

/**
 * Class for describing function definition nodes.
 */
class function_def_node : public cdk::expression_node {
    typedef std::shared_ptr<cdk::basic_type> basic_type;
    cdk::sequence_node *_arg_definitions;
    til::block_node *_body;

  public:
    function_def_node(int lineno, cdk::sequence_node *arg_definitions,
                      basic_type return_type, til::block_node *body)
        : cdk::expression_node(lineno), _arg_definitions(arg_definitions),
          _body(body) {

          std::vector<std::shared_ptr<cdk::basic_type>> arg_types;
          for (size_t i = 0; i < arg_definitions->size(); i++) {
            arg_types.push_back(dynamic_cast<cdk::typed_node*>(arg_definitions->node(i))->type());
          }
          this->type(cdk::functional_type::create(arg_types, return_type));
    }

    cdk::sequence_node *arg_definitions() { return _arg_definitions; }

    til::block_node *body() { return _body; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_function_def_node(this, level);
    }
};

} // namespace til

#endif
