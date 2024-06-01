#ifndef __TIL_AST_DECLARATION_NODE__
#define __TIL_AST_DECLARATION_NODE__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/typed_node.h>

namespace til {

/**
 * Class for describing declaration nodes.
 */
class declaration_node : public cdk::typed_node {
    typedef std::shared_ptr<cdk::basic_type> basic_type;

    int _qualifier;
    std::string _identifier;
    cdk::expression_node *_expression;

  public:
    declaration_node(int lineno, int qualifier, const std::string &identifier,
                     cdk::expression_node *expression, basic_type type)
        : cdk::typed_node(lineno), _qualifier(qualifier),
          _identifier(identifier), _expression(expression) {
        this->type(type);
    }

    int qualifier() { return _qualifier; }
    std::string identifier() { return _identifier; }
    cdk::expression_node *expression() { return _expression; };

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_declaration_node(this, level);
    };
};
} // namespace til

#endif
