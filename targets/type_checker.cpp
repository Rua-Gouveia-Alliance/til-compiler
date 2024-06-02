#include "targets/type_checker.h"
#include ".auto/all_nodes.h" // automatically generated
#include "til_parser.tab.h"
#include <cdk/ast/assignment_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include <cdk/types/functional_type.h>
#include <cdk/types/primitive_type.h>
#include <cdk/types/reference_type.h>
#include <cdk/types/typename_type.h>
#include <string>

#define ASSERT_UNSPEC                                                                              \
  {                                                                                                \
    if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC))                              \
      return;                                                                                      \
  }

//---------------------------------------------------------------------------

void til::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void til::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_block_node(til::block_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_stop_node(til::stop_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_next_node(til::next_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void til::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//-------------------------- Unary Expressions ------------------------------

void til::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl) {
  cdk::expression_node *arg = node->argument();

  arg->accept(this, lvl + 2);
  if (arg->is_typed(cdk::TYPE_UNSPEC))
    arg->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (!arg->is_typed(cdk::TYPE_INT) && !arg->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("wrong type in argument of unary expression");

  node->type(arg->type());
}

void til::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  ASSERT_UNSPEC;
  processUnaryExpression(node, lvl);
}

void til::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  ASSERT_UNSPEC;
  processUnaryExpression(node, lvl);
}

void til::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC;
  cdk::expression_node *arg = node->argument();

  arg->accept(this, lvl + 2);
  if (arg->is_typed(cdk::TYPE_UNSPEC))
    arg->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (!arg->is_typed(cdk::TYPE_INT))
    throw std::string("wrong type in argument of unary expression");

  node->type(arg->type());
}

void til::type_checker::do_objects_node(til::objects_node *const node, int lvl) {
  ASSERT_UNSPEC;
  cdk::expression_node *arg = node->argument();

  arg->accept(this, lvl + 2);
  if (arg->is_typed(cdk::TYPE_UNSPEC))
    arg->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (!arg->is_typed(cdk::TYPE_INT))
    throw std::string("wrong type in argument of unary expression");

  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------- Binary Expressions ---------------------------

void til::type_checker::processArithmeticBinaryExpression(cdk::binary_operation_node *const node,
                                                          int lvl, bool doubles, bool pointer) {
  ASSERT_UNSPEC;
  cdk::expression_node *left = node->left();
  cdk::expression_node *right = node->right();

  left->accept(this, lvl + 2);

  if (left->is_typed(cdk::TYPE_INT) || left->is_typed(cdk::TYPE_UNSPEC)) {
    right->accept(this, lvl + 2);
    if (right->is_typed(cdk::TYPE_UNSPEC)) {
      right->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));

      if (left->is_typed(cdk::TYPE_UNSPEC))
        left->type(node->type());
      return;
    }

    if (right->is_typed(cdk::TYPE_INT) || (doubles && right->is_typed(cdk::TYPE_DOUBLE))) {
      node->type(right->type());

      if (left->is_typed(cdk::TYPE_UNSPEC))
        left->type(node->type());
      return;
    }

    if (pointer && right->is_typed(cdk::TYPE_POINTER)) {
      node->type(right->type());

      if (left->is_typed(cdk::TYPE_UNSPEC))
        left->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      return;
    }

    throw std::string("bad type in binary expression (right)");
  }

  if (doubles && left->is_typed(cdk::TYPE_DOUBLE)) {
    right->accept(this, lvl + 2);
    if (right->is_typed(cdk::TYPE_UNSPEC)) {
      right->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      return;
    }

    if (right->is_typed(cdk::TYPE_INT) || right->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      return;
    }

    throw std::string("bad type in binary expression (right)");
  }

  if (pointer && left->is_typed(cdk::TYPE_POINTER)) {
    right->accept(this, lvl + 2);
    if (right->is_typed(cdk::TYPE_UNSPEC)) {
      right->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      node->type(left->type());
      return;
    }

    if (right->is_typed(cdk::TYPE_INT)) {
      node->type(left->type());
      return;
    }
    throw std::string("bad type in binary expression (right)");
  }
  throw std::string("bad type in binary expression (left)");
}

void til::type_checker::processPredicateBinaryExpression(cdk::binary_operation_node *const node,
                                                         int lvl, bool doubles, bool pointer) {
  ASSERT_UNSPEC;
  cdk::expression_node *left = node->left();
  cdk::expression_node *right = node->right();

  left->accept(this, lvl + 2);

  if (left->is_typed(cdk::TYPE_UNSPEC)) {
    right->accept(this, lvl + 2);
    if (right->is_typed(cdk::TYPE_UNSPEC)) {
      left->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      right->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    }

    if (right->is_typed(cdk::TYPE_INT) || (doubles && right->is_typed(cdk::TYPE_DOUBLE)))
      left->type(right->type());
    else if (right->is_typed(cdk::TYPE_POINTER))
      left->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else
      throw std::string("bad type in binary expression (right)");
  } else if (left->is_typed(cdk::TYPE_INT)) {
    right->accept(this, lvl + 2);
    if (right->is_typed(cdk::TYPE_UNSPEC))
      right->type(left->type());

    if (!(right->is_typed(cdk::TYPE_INT) || (doubles && right->is_typed(cdk::TYPE_DOUBLE)) ||
          (pointer && right->is_typed(cdk::TYPE_POINTER))))
      throw std::string("bad type in binary expression (right)");
  } else if (doubles && left->is_typed(cdk::TYPE_DOUBLE)) {
    right->accept(this, lvl + 2);
    if (right->is_typed(cdk::TYPE_UNSPEC))
      right->type(left->type());

    if (!(right->is_typed(cdk::TYPE_INT) || (doubles && right->is_typed(cdk::TYPE_DOUBLE))))
      throw std::string("bad type in binary expression (right)");
  } else if (pointer && left->is_typed(cdk::TYPE_POINTER)) {
    right->accept(this, lvl + 2);
    if (right->is_typed(cdk::TYPE_UNSPEC))
      right->type(cdk::primitive_type::create(4, cdk::TYPE_INT));

    if (!(right->is_typed(cdk::TYPE_INT) || (pointer && right->is_typed(cdk::TYPE_POINTER))))
      throw std::string("bad type in binary expression (right)");
  } else {
    throw std::string("bad type in binary expression (left)");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processArithmeticBinaryExpression(node, lvl, true, true);
}

void til::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processArithmeticBinaryExpression(node, lvl, true, true);
}

void til::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processArithmeticBinaryExpression(node, lvl, true, false);
}

void til::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processArithmeticBinaryExpression(node, lvl, true, false);
}

void til::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processArithmeticBinaryExpression(node, lvl, false, false);
}

void til::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processPredicateBinaryExpression(node, lvl, true, false);
}

void til::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processPredicateBinaryExpression(node, lvl, true, false);
}

void til::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processPredicateBinaryExpression(node, lvl, true, false);
}

void til::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processPredicateBinaryExpression(node, lvl, true, false);
}

void til::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processPredicateBinaryExpression(node, lvl, true, false);
}

void til::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processPredicateBinaryExpression(node, lvl, true, false);
}

void til::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processPredicateBinaryExpression(node, lvl, false, false);
}

void til::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processPredicateBinaryExpression(node, lvl, false, false);
}

//---------------------------------------------------------------------------

void til::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<til::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void til::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void til::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  cdk::lvalue_node *lvalue = node->lvalue();
  cdk::expression_node *rvalue = node->rvalue();

  lvalue->accept(this, lvl);
  rvalue->accept(this, lvl);

  if (rvalue->is_typed(cdk::TYPE_UNSPEC))
    rvalue->type(lvalue->type());

  if (rvalue->is_typed(cdk::TYPE_POINTER) && lvalue->is_typed(cdk::TYPE_POINTER)) {
    auto lref_type = cdk::reference_type::cast(lvalue->type())->referenced()->name();
    auto rref_type = cdk::reference_type::cast(rvalue->type())->referenced()->name();

    if (lref_type == cdk::TYPE_VOID || rref_type == cdk::TYPE_UNSPEC || rref_type == cdk::TYPE_VOID)
      rvalue->type(lvalue->type());
  }

  if (!equal_types(lvalue->type(), rvalue->type()))
    throw std::string("bad rvalue type in assigment node");

  node->type(lvalue->type());
}

//---------------------------------------------------------------------------

void til::type_checker::do_program_node(til::program_node *const node, int lvl) {
  auto function = std::make_shared<symbol>(
      cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)), "@", 0);
  if (!_symtab.insert(function->name(), function))
    _symtab.replace(function->name(), function);
}

void til::type_checker::do_function_def_node(til::function_def_node *const node, int lvl) {
  auto function = std::make_shared<symbol>(node->type(), "@", 0);
  if (!_symtab.insert(function->name(), function))
    _symtab.replace(function->name(), function);
}

void til::type_checker::do_evaluation_node(til::evaluation_node *const node, int lvl) {
  cdk::expression_node *arg = node->argument();

  arg->accept(this, lvl);
  if (arg->is_typed(cdk::TYPE_UNSPEC))
    arg->type(cdk::primitive_type::create(4, cdk::TYPE_INT));

  if (arg->is_typed(cdk::TYPE_POINTER) &&
      cdk::reference_type::cast(arg->type())->referenced()->name() == cdk::TYPE_UNSPEC)
    arg->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
}

void til::type_checker::do_print_node(til::print_node *const node, int lvl) {
  for (size_t i = 0; i < node->expressions()->size(); i++) {
    auto expr = dynamic_cast<cdk::expression_node *>(node->expressions()->node(i));
    expr->accept(this, lvl);

    if (expr->is_typed(cdk::TYPE_UNSPEC))
      expr->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else if (!(expr->is_typed(cdk::TYPE_INT) || expr->is_typed(cdk::TYPE_DOUBLE) ||
               expr->is_typed(cdk::TYPE_STRING)))
      throw std::string("invalid type in print");
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_read_node(til::read_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void til::type_checker::do_loop_node(til::loop_node *const node, int lvl) {
  cdk::expression_node *cond = node->condition();
  cond->accept(this, lvl + 4);

  if (cond->is_typed(cdk::TYPE_UNSPEC))
    cond->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (!cond->is_typed(cdk::TYPE_INT))
    throw std::string("invalid type for loop condition");
}

void til::type_checker::do_if_node(til::if_node *const node, int lvl) {
  cdk::expression_node *cond = node->condition();
  cond->accept(this, lvl + 4);

  if (cond->is_typed(cdk::TYPE_UNSPEC))
    cond->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (!cond->is_typed(cdk::TYPE_INT))
    throw std::string("invalid type for if condition");
}

void til::type_checker::do_if_else_node(til::if_else_node *const node, int lvl) {
  cdk::expression_node *cond = node->condition();
  node->condition()->accept(this, lvl + 4);

  if (cond->is_typed(cdk::TYPE_UNSPEC))
    cond->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (!cond->is_typed(cdk::TYPE_INT))
    throw std::string("invalid type for if condition");
}

//---------------------------------------------------------------------------

void til::type_checker::do_declaration_node(til::declaration_node *const node, int lvl) {
  cdk::expression_node *expr = node->expression();
  if (node->type() != nullptr && expr != nullptr) {
    expr->accept(this, lvl + 2);
    if (expr->is_typed(cdk::TYPE_UNSPEC)) {
      expr->type(node->is_typed(cdk::TYPE_DOUBLE) ? node->type()
                                                  : cdk::primitive_type::create(4, cdk::TYPE_INT));
    }

    if (node->is_typed(cdk::TYPE_POINTER) && expr->is_typed(cdk::TYPE_POINTER)) {
      auto node_ref_type = cdk::reference_type::cast(node->type())->referenced()->name();
      auto expr_ref_type = cdk::reference_type::cast(expr->type())->referenced()->name();

      if (node_ref_type == cdk::TYPE_VOID || expr_ref_type == cdk::TYPE_UNSPEC ||
          expr_ref_type == cdk::TYPE_VOID)
        expr->type(node->type());
    }

    if (!equal_types(node->type(), expr->type()))
      throw std::string("bad expression type in declaration node");
  } else if (node->type() == nullptr) {
    // var means expr is not nullptr
    expr->accept(this, lvl + 2);
    if (expr->is_typed(cdk::TYPE_UNSPEC))
      expr->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    else if (expr->is_typed(cdk::TYPE_VOID))
      throw std::string("delcaration of type void");

    if (expr->is_typed(cdk::TYPE_POINTER) &&
        cdk::reference_type::cast(expr->type())->name() == cdk::TYPE_UNSPEC) {
      expr->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
    }

    node->type(expr->type());
  }

  if (node->qualifier() == tEXTERNAL && !node->is_typed(cdk::TYPE_FUNCTIONAL))
    throw std::string("declaration with foreign qualifier is not a function");

  auto sym = std::make_shared<symbol>(node->type(), node->identifier(), node->qualifier());
  if (!_symtab.insert(node->identifier(), sym)) {
    auto prev = _symtab.find(node->identifier());
    if (prev != nullptr && prev->qualifier() == tFORWARD &&
        equal_types(prev->type(), sym->type(), true)) {
      _symtab.replace(node->identifier(), sym);
    } else {
      throw std::string("redeclaration");
    }
  }

  _parent->set_new_symbol(sym);
}

//---------------------------------------------------------------------------

void til::type_checker::do_call_node(til::call_node *const node, int lvl) {
  ASSERT_UNSPEC;
  cdk::expression_node *fptr = node->function_ptr();
  std::shared_ptr<cdk::functional_type> ftype;

  if (fptr != nullptr) {
    fptr->accept(this, lvl);
    if (!fptr->is_typed(cdk::TYPE_FUNCTIONAL))
      throw std::string("call of non-function");

    ftype = cdk::functional_type::cast(fptr->type());
  } else {
    // call of current function
    auto symbol = _symtab.find("@", 1);
    if (symbol == nullptr)
      throw std::string("recursive invocation outside a function");

    ftype = cdk::functional_type::cast(symbol->type());
  }

  // check number of arguments
  if (node->args()->size() != ftype->input()->length())
    throw std::string("wrong number of arguments");

  for (size_t i = 0; i < node->args()->size(); i++) {
    auto farg = ftype->input(i);
    auto narg = dynamic_cast<cdk::expression_node *>(node->args()->node(i));

    narg->accept(this, lvl);

    if (narg->is_typed(cdk::TYPE_UNSPEC)) {
      narg->type(ftype->name() == cdk::TYPE_DOUBLE
                     ? cdk::primitive_type::create(8, cdk::TYPE_DOUBLE)
                     : cdk::primitive_type::create(4, cdk::TYPE_INT));
    }

    if (narg->is_typed(cdk::TYPE_POINTER) && farg->name() == cdk::TYPE_POINTER) {
      auto narg_ref_type = cdk::reference_type::cast(narg->type())->referenced()->name();
      auto farg_ref_type = cdk::reference_type::cast(farg)->referenced()->name();

      if (farg_ref_type == cdk::TYPE_VOID || narg_ref_type == cdk::TYPE_UNSPEC ||
          narg_ref_type == cdk::TYPE_VOID)
        narg->type(farg);
    }

    if (!equal_types(farg, narg->type()))
      throw std::string("bad arg type in function call");
  }

  node->type(ftype->output(0));
}

//---------------------------------------------------------------------------

void til::type_checker::do_with_node(til::with_node *const node, int lvl) {
  cdk::expression_node *fptr = node->function_ptr();
  std::shared_ptr<cdk::functional_type> ftype;

  if (fptr != nullptr) {
    fptr->accept(this, lvl);
    if (!fptr->is_typed(cdk::TYPE_FUNCTIONAL))
      throw std::string("call of non-function");

    ftype = cdk::functional_type::cast(fptr->type());
  } else {
    // call of current function
    auto symbol = _symtab.find("@", 1);
    if (symbol == nullptr)
      throw std::string("recursive invocation outside a function");

    ftype = cdk::functional_type::cast(symbol->type());
  }

  // check number of arguments
  if (ftype->input()->length() != 1)
    throw std::string("wrong number of arguments");

  node->vector()->accept(this, lvl);
  if (node->vector()->is_typed(cdk::TYPE_POINTER)) {
    auto vector_ref_type = cdk::reference_type::cast(node->vector()->type())->referenced();

    if (!equal_types(ftype->input(0), vector_ref_type, true))
      throw std::string("bad vector type in with");
  } else {
    throw std::string("bad vector type in with");
  }

  node->low()->accept(this, lvl);
  if (node->low()->is_typed(cdk::TYPE_UNSPEC)) {
    node->low()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->low()->is_typed(cdk::TYPE_INT)) {
    throw std::string("bad low type in with");
  }

  node->high()->accept(this, lvl);
  if (node->high()->is_typed(cdk::TYPE_UNSPEC)) {
    node->high()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->high()->is_typed(cdk::TYPE_INT)) {
    throw std::string("bad high type in with");
  }
}

//---------------------------------------------------------------------------
void til::type_checker::do_return_node(til::return_node *const node, int lvl) {
  auto symbol = _symtab.find("@", 1);
  if (symbol == nullptr)
    throw std::string("return outside function block");

  auto ftype = cdk::functional_type::cast(symbol->type());
  if (node->expression() == nullptr && ftype->output(0)->name() != cdk::TYPE_VOID)
    throw std::string("non-void function with no return");
  else if (node->expression() != nullptr && ftype->output(0)->name() == cdk::TYPE_VOID)
    throw std::string("void function with return");
  else if (node->expression() != nullptr) {
    node->expression()->accept(this, lvl + 2);
    if (!equal_types(ftype->output(0), node->expression()->type()))
      throw std::string("function and return types mismatch");
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_sizeof_node(til::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;
  cdk::expression_node *arg = node->argument();

  arg->accept(this, lvl + 2);
  if (arg->is_typed(cdk::TYPE_UNSPEC))
    arg->type(cdk::primitive_type::create(4, cdk::TYPE_INT));

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void til::type_checker::do_index_node(til::index_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->ptr()->accept(this, lvl + 2);
  if (!node->ptr()->is_typed(cdk::TYPE_POINTER))
    throw std::string("expected pointer in index operator as ptr");

  node->index()->accept(this, lvl + 2);
  if (node->index()->is_typed(cdk::TYPE_UNSPEC))
    node->index()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  else if (!node->index()->is_typed(cdk::TYPE_INT))
    throw std::string("expected int in index operator as index");

  auto ptr_type = cdk::reference_type::cast(node->ptr()->type());
  if (ptr_type->referenced()->name() == cdk::TYPE_UNSPEC) {
    ptr_type = cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->ptr()->type(ptr_type);
  }
  node->type(ptr_type->referenced());
}

//---------------------------------------------------------------------------

void til::type_checker::do_address_node(til::address_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl + 2);

  // check if lvalue is void!
  if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    if (cdk::reference_type::cast(node->lvalue()->type())->referenced()->name() == cdk::TYPE_VOID)
      node->type(node->lvalue()->type());
  } else {
    node->type(cdk::reference_type::create(4, node->lvalue()->type()));
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_nullptr_node(til::nullptr_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

//---------------------------------------------------------------------------

bool til::type_checker::equal_types(std::shared_ptr<cdk::basic_type> t1,
                                    std::shared_ptr<cdk::basic_type> t2, bool strict) {
  if (t1->name() == cdk::TYPE_UNSPEC && t2->name() == cdk::TYPE_UNSPEC)
    return false;

  if (!strict && t1->name() == cdk::TYPE_DOUBLE)
    return t2->name() == cdk::TYPE_INT || t2->name() == cdk::TYPE_DOUBLE;

  if (t1->name() == cdk::TYPE_POINTER || t2->name() == cdk::TYPE_POINTER) {
    if (t1->name() != t2->name())
      return false;

    auto t1_ref_type = cdk::reference_type::cast(t1)->referenced();
    auto t2_ref_type = cdk::reference_type::cast(t2)->referenced();

    return equal_types(t1_ref_type, t2_ref_type);
  }

  if (t1->name() == cdk::TYPE_FUNCTIONAL || t2->name() == cdk::TYPE_FUNCTIONAL) {
    if (t1->name() != t2->name())
      return false;

    auto t1_type = cdk::functional_type::cast(t1);
    auto t2_type = cdk::functional_type::cast(t2);

    if (t1_type->input_length() != t2_type->input_length())
      return false;
    if (t1_type->output_length() != t2_type->output_length())
      return false;

    for (size_t i = 0; i < t1_type->input_length(); i++) {
      if (!equal_types(t2_type->input(i), t1_type->input(i), strict))
        return false;
    }

    for (size_t i = 0; i < t1_type->output_length(); i++) {
      if (!equal_types(t1_type->output(i), t2_type->output(i), strict))
        return false;
    }

    return true;
  }

  return t1 == t2;
}
