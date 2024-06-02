#include "targets/postfix_writer.h"
#include ".auto/all_nodes.h" // all_nodes.h is automatically generated
#include "ast/declaration_node.h"
#include "targets/frame_size_calculator.h"
#include "targets/type_checker.h"
#include "til_parser.tab.h"
#include <cdk/ast/assignment_node.h>
#include <cdk/ast/rvalue_node.h>
#include <cdk/ast/sequence_node.h>
#include <string>

//---------------------------------------------------------------------------

void til::postfix_writer::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}

void til::postfix_writer::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

void til::postfix_writer::do_block_node(til::block_node *const node, int lvl) {
  _symtab.push();
  _finishedBlock = false;

  // declarations
  node->declarations()->accept(this, lvl + 2);

  // instructions
  cdk::sequence_node *instructions = node->instructions();
  for (size_t i = 0; i < instructions->size(); i++) {
    instructions->node(i)->accept(this, lvl + 2);
    if (_finishedBlock)
      break;
  }

  _finishedBlock = false;
  _symtab.pop();
}

void til::postfix_writer::do_stop_node(til::stop_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto n = static_cast<size_t>(node->n());
  auto label = _loopLabels->at(_loopLabels->size() - n).second;
  _pf.JMP(label);
  _finishedBlock = true;
}

void til::postfix_writer::do_next_node(til::next_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto n = static_cast<size_t>(node->n());
  auto label = _loopLabels->at(_loopLabels->size() - n).first;
  _pf.JMP(label);
  _finishedBlock = true;
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_integer_node(cdk::integer_node *const node, int lvl) {
  if (isLocal())
    _pf.INT(node->value()); // push an integer
  else
    _pf.SINT(node->value());
}

void til::postfix_writer::do_double_node(cdk::double_node *const node, int lvl) {
  if (isLocal())
    _pf.DOUBLE(node->value());
  else
    _pf.SDOUBLE(node->value());
}

void til::postfix_writer::do_string_node(cdk::string_node *const node, int lvl) {
  int lbl1;

  /* generate the string */
  _pf.RODATA();                    // strings are DATA readonly
  _pf.ALIGN();                     // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value());      // output string characters

  if (isLocal()) {
    _pf.TEXT(_functionLabels.back()); // return to the TEXT segment
    _pf.ADDR(mklbl(lbl1));            // the string to be stored
  } else {
    _pf.DATA();             // return to the DATA segment
    _pf.SADDR(mklbl(lbl1)); // the string to be stored
  }
}

//-------------------------- Unary Expressions ------------------------------

void til::postfix_writer::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DNEG();
  else
    _pf.NEG(); // 2-complement
}

void til::postfix_writer::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
}

void til::postfix_writer::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl + 2);
  _pf.INT(0);
  _pf.EQ();
}

void til::postfix_writer::do_objects_node(til::objects_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);

  auto node_type = cdk::reference_type::cast(node->type());
  _pf.INT(std::max(static_cast<size_t>(1), node_type->referenced()->size()));
  _pf.MUL();
  _pf.ALLOC();
  _pf.SP();
}

//---------------------------- Binary Expressions ---------------------------

void til::postfix_writer::fixBinOpTypes(cdk::binary_operation_node *const node, int lvl) {
  node->left()->accept(this, lvl);
  // Converting INT to DOUBLE if one of the operands is of type DOUBLE
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl);
  // Converting INT to DOUBLE if one of the operands is of type DOUBLE
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();
}

void til::postfix_writer::fixBinOpTypesWithPtrs(cdk::binary_operation_node *const node, int lvl) {
  node->left()->accept(this, lvl);
  // Converting INT to DOUBLE if one of the operands is of type DOUBLE
  if (node->left()->is_typed(cdk::TYPE_INT) && node->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();
  else if (node->is_typed(cdk::TYPE_POINTER) && node->left()->is_typed(cdk::TYPE_INT)) {
    auto node_type = cdk::reference_type::cast(node->type());
    _pf.INT(std::max(static_cast<size_t>(1), node_type->referenced()->size()));
    _pf.MUL();
  }

  node->right()->accept(this, lvl);
  // Converting INT to DOUBLE if one of the operands is of type DOUBLE
  if (node->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();
  else if (node->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT)) {
    auto node_type = cdk::reference_type::cast(node->type());
    _pf.INT(std::max(static_cast<size_t>(1), node_type->referenced()->size()));
    _pf.MUL();
  }
}

void til::postfix_writer::fixCompTypes(cdk::binary_operation_node *const node, int lvl) {
  fixBinOpTypes(node, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.DCMP();
    _pf.INT(0);
  }
}

void til::postfix_writer::do_add_node(cdk::add_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixBinOpTypesWithPtrs(node, lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DADD();
  else
    _pf.ADD();
}

void til::postfix_writer::do_sub_node(cdk::sub_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixBinOpTypesWithPtrs(node, lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DSUB();
  else
    _pf.SUB();
}

void til::postfix_writer::do_mul_node(cdk::mul_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixBinOpTypes(node, lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DMUL();
  else
    _pf.MUL();
}

void til::postfix_writer::do_div_node(cdk::div_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixBinOpTypes(node, lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DDIV();
  else
    _pf.DIV();
}

void til::postfix_writer::do_mod_node(cdk::mod_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}

void til::postfix_writer::do_lt_node(cdk::lt_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixCompTypes(node, lvl);
  _pf.LT();
}

void til::postfix_writer::do_le_node(cdk::le_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixCompTypes(node, lvl);
  _pf.LE();
}

void til::postfix_writer::do_ge_node(cdk::ge_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixCompTypes(node, lvl);
  _pf.GE();
}

void til::postfix_writer::do_gt_node(cdk::gt_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixCompTypes(node, lvl);
  _pf.GT();
}

void til::postfix_writer::do_ne_node(cdk::ne_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixCompTypes(node, lvl);
  _pf.NE();
}

void til::postfix_writer::do_eq_node(cdk::eq_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixCompTypes(node, lvl);
  _pf.EQ();
}

void til::postfix_writer::do_and_node(cdk::and_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);

  // Only evaluate the second expression if the first evaluates to true
  _pf.DUP32();
  int lbl = ++_lbl;
  _pf.JZ(mklbl(lbl));

  node->right()->accept(this, lvl);
  _pf.AND();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

void til::postfix_writer::do_or_node(cdk::or_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);

  // Only evaluate the second expression if the first evaluates to false
  _pf.DUP32();
  int lbl;
  lbl = ++_lbl;
  _pf.JNZ(mklbl(lbl));

  node->right()->accept(this, lvl);
  _pf.OR();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto symbol = _symtab.find(node->name());

  if (symbol->qualifier() == tEXTERNAL)
    _externalCall = symbol->name();
  else if (symbol->global())
    _pf.ADDR(node->name());
  else
    _pf.LOCAL(symbol->offset());
}

void til::postfix_writer::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->lvalue()->accept(this, lvl);
  if (_externalCall == "") {
    if (node->is_typed(cdk::TYPE_DOUBLE))
      _pf.LDDOUBLE();
    else
      _pf.LDINT();
  }
}

void til::postfix_writer::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  fixFunctionTypes(node->rvalue(), node->type(), lvl);

  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.DUP64();
  else
    _pf.DUP32();

  node->lvalue()->accept(this, lvl);
  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.STDOUBLE();
  else
    _pf.STINT();
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_program_node(til::program_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto tempOffset = _offset;
  auto tempReturnLabel = _currentReturnLabel;
  auto tempLoopLabels = _loopLabels;

  // generate the main function (RTS mandates that its name be "_main")
  _functionLabels.push_back("_main");
  _pf.TEXT(_functionLabels.back());
  _pf.ALIGN();
  _pf.GLOBAL("_main", _pf.FUNC());
  _pf.LABEL(_functionLabels.back());

  _offset = 8;
  _symtab.push();

  frame_size_calculator calculator(_compiler, _symtab);
  node->statements()->accept(&calculator, lvl);
  _pf.ENTER(calculator.localsize());

  _currentReturnLabel = mklbl(++_lbl);
  _loopLabels = new std::vector<std::pair<std::string, std::string>>();

  _offset = 0;
  node->statements()->accept(this, lvl);
  _pf.INT(0);
  _pf.STFVAL32();
  _pf.ALIGN();
  _pf.LABEL(_currentReturnLabel);
  _pf.LEAVE();
  _pf.RET();

  _currentReturnLabel = tempReturnLabel;
  delete _loopLabels;
  _loopLabels = tempLoopLabels;
  _offset = tempOffset;

  _symtab.pop();
  _functionLabels.pop_back();

  // these are just a few library function imports
  _pf.EXTERN("readi");
  _pf.EXTERN("readd");
  _pf.EXTERN("printi");
  _pf.EXTERN("printd");
  _pf.EXTERN("prints");
  _pf.EXTERN("println");

  for (auto name : _newExternals) {
    _pf.EXTERN(name);
  }
}

void til::postfix_writer::do_function_def_node(til::function_def_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto tempOffset = _offset;
  auto tempReturnLabel = _currentReturnLabel;
  auto tempLoopLabels = _loopLabels;

  std::string functionLabel = mklbl(++_lbl);
  _functionLabels.push_back(functionLabel);
  _pf.TEXT(_functionLabels.back());
  _pf.ALIGN();
  _pf.LABEL(_functionLabels.back());

  _offset = 8;
  _symtab.push();

  _inFunctionArguments = true;
  node->arg_definitions()->accept(this, lvl);
  _inFunctionArguments = false;

  frame_size_calculator calculator(_compiler, _symtab);
  node->body()->accept(&calculator, lvl);
  _pf.ENTER(calculator.localsize());

  _currentReturnLabel = mklbl(++_lbl);
  _loopLabels = new std::vector<std::pair<std::string, std::string>>();

  _offset = 0;
  node->body()->accept(this, lvl);
  _pf.ALIGN();
  _pf.LABEL(_currentReturnLabel);
  _pf.LEAVE();
  _pf.RET();

  _currentReturnLabel = tempReturnLabel;
  delete _loopLabels;
  _loopLabels = tempLoopLabels;
  _offset = tempOffset;

  _symtab.pop();
  _functionLabels.pop_back();

  if (isLocal()) {
    _pf.TEXT(_functionLabels.back());
    _pf.ADDR(functionLabel);
  } else {
    _pf.DATA();
    _pf.SADDR(functionLabel);
  }
}

void til::postfix_writer::do_evaluation_node(til::evaluation_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  if (node->argument()->type()->size() > 0)
    _pf.TRASH(node->argument()->type()->size());
}

void til::postfix_writer::do_print_node(til::print_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  for (size_t i = 0; i < node->expressions()->size(); ++i) {
    auto target = dynamic_cast<cdk::expression_node *>(node->expressions()->node(i));

    target->accept(this, lvl); // determine the value to print
    if (target->is_typed(cdk::TYPE_INT)) {
      _pf.CALL("printi");
      _pf.TRASH(4); // delete the printed value
    } else if (target->is_typed(cdk::TYPE_DOUBLE)) {
      _pf.CALL("printd");
      _pf.TRASH(8); // delete the printed value
    } else if (target->is_typed(cdk::TYPE_STRING)) {
      _pf.CALL("prints");
      _pf.TRASH(4); // delete the printed value's address
    } else {
      std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
      exit(1);
    }
  }

  if (node->ln())
    _pf.CALL("println");
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_read_node(til::read_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else {
    _pf.CALL("readi");
    _pf.LDFVAL32();
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_loop_node(til::loop_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  _pf.ALIGN();
  int lbl1 = ++_lbl;
  _pf.LABEL(mklbl(lbl1));

  node->condition()->accept(this, lvl);

  int lbl2 = ++_lbl;
  _pf.JZ(mklbl(lbl2));

  _loopLabels->push_back(std::make_pair(mklbl(lbl1), mklbl(lbl2)));
  node->block()->accept(this, lvl + 2);
  _loopLabels->pop_back();

  _pf.JMP(mklbl(lbl1));
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl2));
}

void til::postfix_writer::do_if_node(til::if_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->condition()->accept(this, lvl);
  int lbl1 = ++_lbl;
  _pf.JZ(mklbl(lbl1));

  node->block()->accept(this, lvl + 2);
  _finishedBlock = false;

  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));
}

void til::postfix_writer::do_if_else_node(til::if_else_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;

  node->condition()->accept(this, lvl);
  int lbl1 = ++_lbl;
  _pf.JZ(mklbl(lbl1));

  node->thenblock()->accept(this, lvl + 2);
  _finishedBlock = false;

  int lbl2 = ++_lbl;
  _pf.JMP(mklbl(lbl2));

  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl1));

  node->elseblock()->accept(this, lvl + 2);
  _finishedBlock = false;

  _pf.ALIGN();
  lbl1 = lbl2;
  _pf.LABEL(mklbl(lbl1));
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_declaration_node(til::declaration_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto symbol = new_symbol();
  reset_new_symbol();

  int offset = 0;
  // Offset changes depending on what we're declaring
  if (_inFunctionArguments) {
    offset = _offset;
    _offset += node->type()->size();
  } else if (isLocal()) {
    _offset -= node->type()->size();
    offset = _offset;
  } else {
    offset = 0;
  }
  symbol->offset(offset);

  if (isLocal()) {
    if (!_inFunctionArguments && node->expression() != nullptr) {
      fixFunctionTypes(node->expression(), node->type(), lvl);
      if (node->is_typed(cdk::TYPE_DOUBLE)) {
        _pf.LOCAL(symbol->offset());
        _pf.STDOUBLE();
      } else {
        _pf.LOCAL(symbol->offset());
        _pf.STINT();
      }
    }
  } else {
    if (symbol->qualifier() == tFORWARD || symbol->qualifier() == tEXTERNAL) {
      _newExternals.insert(symbol->name());
    } else {
      _newExternals.erase(symbol->name());
      if (node->expression() != nullptr) {
        _pf.DATA();
        _pf.ALIGN();
        if (symbol->qualifier() == tPUBLIC)
          _pf.GLOBAL(symbol->name(), _pf.OBJ());

        _pf.LABEL(symbol->name());
        if (node->is_typed(cdk::TYPE_DOUBLE) && node->expression()->is_typed(cdk::TYPE_INT)) {  // Need to fix type
          auto fixed_node = dynamic_cast<cdk::integer_node *>(node->expression());
          _pf.SDOUBLE(fixed_node->value());
        } else {
          node->expression()->accept(this, lvl);
        }
      } else {
        _pf.BSS();
        _pf.ALIGN();

        if (symbol->qualifier() == tPUBLIC)
          _pf.GLOBAL(symbol->name(), _pf.OBJ());

        _pf.LABEL(symbol->name());
        _pf.SALLOC(node->type()->size());
      }
    }
  }
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_call_node(til::call_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  std::shared_ptr<cdk::functional_type> type;
  if (node->function_ptr() == nullptr) {  // Recursive call
    auto symbol = _symtab.find("@", 1);
    type = cdk::functional_type::cast(symbol->type());
  } else {  // Normal call
    type = cdk::functional_type::cast(node->function_ptr()->type());
  }

  int trash_size = 0; // Need to delete args from stack after call
  for (size_t i = node->args()->size(); i > 0; i--) { // Push to stack in reverse order
    auto argument = dynamic_cast<cdk::expression_node *>(node->args()->node(i - 1));
    trash_size += argument->type()->size();
    fixFunctionTypes(argument, type->input(i - 1), lvl + 2); // Pushing to stack and fixing types
  }

  _externalCall = "";

  if (node->function_ptr() == nullptr) // Recursive call
    _pf.ADDR(_functionLabels.back());
  else // Normal call
    node->function_ptr()->accept(this, lvl);

  if (_externalCall != "")
    _pf.CALL(_externalCall);
  else
    _pf.BRANCH();

  _externalCall = "";

  // If there are args to delete, delete them
  if (trash_size != 0)
    _pf.TRASH(trash_size);

  // Only load if return is not void
  if (!node->is_typed(cdk::TYPE_VOID)) {
    if (node->is_typed(cdk::TYPE_DOUBLE))
        _pf.LDFVAL64();
    else
        _pf.LDFVAL32();
  }
}

void til::postfix_writer::do_with_node(til::with_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  std::shared_ptr<cdk::functional_type> type;
  if (node->function() == nullptr) {  // Recursive call
    auto symbol = _symtab.find("@", 1);
    type = cdk::functional_type::cast(symbol->type());
  } else {  // Normal call
    type = cdk::functional_type::cast(node->function()->type());
  }

  std::string lbl = mklbl(_lbl++);
  std::string i = "_with_node_i_" + lbl;
  std::string endfor = "_with_node_endfor_" + lbl;
  std::string condition = "_with_node_condition_" + lbl;
  std::string increment = "_with_node_increment_" + lbl;
  auto vec_ref_type = cdk::reference_type::cast(node->vec()->type())->referenced();

  node->low()->accept(this, lvl);
  _pf.LABEL(condition);
  _pf.DUP32();
  node->high()->accept(this, lvl);
  _pf.LT();
  _pf.JZ(endfor);

  _pf.DUP32();
  _pf.INT(vec_ref_type->size());
  _pf.MUL();
  node->vec()->accept(this, lvl);
  _pf.ADD();
  if (vec_ref_type->name() == cdk::TYPE_DOUBLE)
    _pf.LDDOUBLE();
  else
    _pf.LDINT();

  _externalCall = "";
  if (node->function() == nullptr) // Recursive call
      _pf.ADDR(_functionLabels.back());
  else // Normal call
      node->function()->accept(this, lvl);

  if (_externalCall != "")
      _pf.CALL(_externalCall);
  else
      _pf.BRANCH();

  _externalCall = "";

  _pf.TRASH(vec_ref_type->size());

  _pf.LABEL(increment);
  _pf.INT(1);
  _pf.ADD();
  _pf.JMP(condition);
  _pf.LABEL(endfor);
  _pf.TRASH(node->low()->type()->size());
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_return_node(til::return_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  auto symbol = _symtab.find("@", 1);
  auto return_type = cdk::functional_type::cast(symbol->type())->output(0);

  if (return_type->name() != cdk::TYPE_VOID) {
    fixFunctionTypes(node->expression(), return_type, lvl + 2);
    if (return_type->name() == cdk::TYPE_DOUBLE)
      _pf.STFVAL64();
    else
      _pf.STFVAL32();
  }

  _pf.JMP(_currentReturnLabel);
  _finishedBlock = true;
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_sizeof_node(til::sizeof_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->argument()->type()->size());
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_index_node(til::index_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->ptr()->accept(this, lvl + 2);
  node->index()->accept(this, lvl + 2);
  _pf.INT(node->type()->size());
  _pf.MUL();
  _pf.ADD();
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_address_node(til::address_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::postfix_writer::do_nullptr_node(til::nullptr_node *const node, int lvl) {
  ASSERT_SAFE_EXPRESSIONS;
  if (isLocal())
    _pf.INT(0);
  else
    _pf.SINT(0);
}

//---------------------------------------------------------------------------

void til::postfix_writer::fixFunctionTypes(cdk::expression_node *node,
                                           std::shared_ptr<cdk::basic_type> target_type, int lvl) {

  if (!node->is_typed(cdk::TYPE_FUNCTIONAL) || target_type->name() != cdk::TYPE_FUNCTIONAL) {
    node->accept(this, lvl);
    if (node->is_typed(cdk::TYPE_INT) && target_type->name() == cdk::TYPE_DOUBLE)
      _pf.I2D();
  } else {
    auto fnode_type = cdk::functional_type::cast(node->type());
    auto ftarget_type = cdk::functional_type::cast(target_type);

    bool found = false;
    // check output
    if (fnode_type->output(0)->name() == cdk::TYPE_INT &&
        ftarget_type->output(0)->name() == cdk::TYPE_DOUBLE) {
      found = true;
    }

    // check inputs
    if (!found) {
      for (size_t i = 0; i < ftarget_type->input_length(); i++) {
        if (fnode_type->input(i)->name() == cdk::TYPE_DOUBLE &&
            ftarget_type->input(i)->name() == cdk::TYPE_INT) {
          found = true;
          break;
        }
      }
    }

    if (found) {
      std::string wrapper_name = "_wrapper_target_" + std::to_string(_lbl++);

      _forceGlobal = true;
      til::declaration_node *wrapper_decl =
          new til::declaration_node(node->lineno(), tPRIVATE, wrapper_name, nullptr, fnode_type);
      wrapper_decl->accept(this, lvl);
      _forceGlobal = false;

      if (isLocal())
        _pf.TEXT(_functionLabels.back());
      else
        _pf.DATA();
      _pf.ALIGN();

      cdk::variable_node *wrapper_var = new cdk::variable_node(node->lineno(), wrapper_name);
      cdk::assignment_node *wrapper_assigment =
          new cdk::assignment_node(node->lineno(), wrapper_var, node);
      wrapper_assigment->accept(this, lvl);

      auto args = new cdk::sequence_node(node->lineno());
      auto call_args = new cdk::sequence_node(node->lineno());
      for (size_t i = 0; i < ftarget_type->input_length(); i++) {
        std::string arg_name = "_arg" + std::to_string(i);

        auto arg_decl = new til::declaration_node(node->lineno(), tPRIVATE, arg_name, nullptr,
                                                  ftarget_type->input(i));
        auto arg_rvalue =
            new cdk::rvalue_node(node->lineno(), new cdk::variable_node(node->lineno(), arg_name));

        args = new cdk::sequence_node(node->lineno(), arg_decl, args);
        call_args = new cdk::sequence_node(node->lineno(), arg_rvalue, call_args);
      }

      cdk::rvalue_node *wrapper_rvalue = new cdk::rvalue_node(node->lineno(), wrapper_var);
      til::call_node *call = new til::call_node(node->lineno(), wrapper_rvalue, call_args);
      til::return_node *return_node = new til::return_node(node->lineno(), call);
      cdk::sequence_node *decls = new cdk::sequence_node(node->lineno());
      cdk::sequence_node *insts = new cdk::sequence_node(node->lineno(), return_node);
      til::block_node *block = new til::block_node(node->lineno(), decls, insts);
      til::function_def_node *wrapping_function =
          new til::function_def_node(node->lineno(), args, ftarget_type->output(0), block);

      wrapping_function->accept(this, lvl);
    } else {
      node->accept(this, lvl);
    }
  }
}
