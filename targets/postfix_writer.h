#ifndef __SIMPLE_TARGETS_POSTFIX_WRITER_H__
#define __SIMPLE_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <cdk/emitters/basic_postfix_emitter.h>
#include <set>
#include <sstream>
#include <vector>

namespace til {

//!
//! Traverse syntax tree and generate the corresponding assembly code.
//!
class postfix_writer : public basic_ast_visitor {
  cdk::symbol_table<til::symbol> &_symtab;
  cdk::basic_postfix_emitter &_pf;
  int _lbl;
  std::string _externalCall = "";
  std::vector<std::string> _functionLabels;
  std::vector<std::pair<std::string, std::string>> *_loopLabels;
  bool _finishedBlock = false;
  int _offset = 0;
  bool _inFunctionArguments = false;
  std::set<std::string> _newExternals;
  bool _forceGlobal = false;
  std::string _currentReturnLabel = "";

public:
  postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<til::symbol> &symtab,
                 cdk::basic_postfix_emitter &pf)
      : basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0) {}

public:
  ~postfix_writer() { os().flush(); }

private:
  void fixCompTypes(cdk::binary_operation_node *const node, int lvl);
  void fixBinOpTypesWithPtrs(cdk::binary_operation_node *const node, int lvl);
  void fixBinOpTypes(cdk::binary_operation_node *const node, int lvl);
  void fixFunctionTypes(cdk::expression_node *node, std::shared_ptr<cdk::basic_type> target_type,
                        int lvl);
  bool isLocal() { return !_functionLabels.empty() && !_forceGlobal; }

  /** Method used to generate sequential labels. */
  inline std::string mklbl(int lbl) {
    std::ostringstream oss;
    if (lbl < 0)
      oss << ".L" << -lbl;
    else
      oss << "_L" << lbl;
    return oss.str();
  }

public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h" // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end
};

} // namespace til

#endif
