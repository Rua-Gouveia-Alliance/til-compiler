#ifndef __SIMPLE_TARGETS_XML_WRITER_H__
#define __SIMPLE_TARGETS_XML_WRITER_H__

#include "targets/basic_ast_visitor.h"
#include <cdk/ast/basic_node.h>
#include <cdk/types/types.h>
#include "til_parser.tab.h"

namespace til {

  /**
   * Print nodes as XML elements to the output stream.
   */
  class xml_writer: public basic_ast_visitor {
    cdk::symbol_table<til::symbol> &_symtab;

  public:
    xml_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<til::symbol> &symtab) :
        basic_ast_visitor(compiler), _symtab(symtab) {
    }

  public:
    ~xml_writer() {
      os().flush();
    }

  private:
    void openTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') + "<" + tag + ">" << std::endl;
    }
    void openTag(const cdk::basic_node *node, int lvl) {
      openTag(node->label(), lvl);
    }
    void closeTag(const std::string &tag, int lvl) {
      os() << std::string(lvl, ' ') + "</" + tag + ">" << std::endl;
    }
    void closeTag(const cdk::basic_node *node, int lvl) {
      closeTag(node->label(), lvl);
    }

  protected:
    void do_binary_operation(cdk::binary_operation_node *const node, int lvl);
    void do_unary_operation(cdk::unary_operation_node *const node, int lvl);
    template<typename T>
    void process_literal(cdk::literal_node<T> *const node, int lvl) {
      os() << std::string(lvl, ' ') << "<" << node->label() << ">" << node->value() << "</" << node->label() << ">" << std::endl;
    }

    inline std::string type_to_str(std::shared_ptr<cdk::basic_type> type) {
      if (type == nullptr) {
        return "[unknown]";
      }

      auto ftype = std::dynamic_pointer_cast<cdk::functional_type>(type);
      if (ftype != nullptr) {
        return "(" + type_to_str(ftype->input()) + ") returns (" + type_to_str(ftype->output()) + ")";
      }

      auto stype = std::dynamic_pointer_cast<cdk::structured_type>(type);
      if (stype != nullptr) {
        auto result = std::string();
        auto components = stype->components();
        return std::accumulate(components.begin(), components.end(), result,
            [this] (auto a, auto b) {
              return a + (a.empty() ? "" : ", ") + type_to_str(b);
            });
      }

      return cdk::to_string(type);
    }

    inline const char *qualifier_to_str(int qualifier) {
      switch (qualifier) {
        case tEXTERNAL: return "external";
        case tFORWARD: return "forward";
        case tPUBLIC: return "public";
        case tPRIVATE: return "private";
        default: return "[unknown]";
      };
    }

  public:
    // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
    // do not edit these lines: end

  };

} // til

#endif
