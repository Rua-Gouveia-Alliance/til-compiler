#ifndef __SIMPLE_TARGETS_SYMBOL_H__
#define __SIMPLE_TARGETS_SYMBOL_H__

#include <cdk/types/basic_type.h>
#include <memory>
#include <string>

namespace til {

class symbol {
  std::shared_ptr<cdk::basic_type> _type;
  std::string _name;
  long _value; // hack!
  int _qualifier;
  int _offset = 0; // 0 means global

public:
  symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name, int qualifier)
      : _type(type), _name(name), _qualifier(qualifier) {}

  virtual ~symbol() {
    // EMPTY
  }

  std::shared_ptr<cdk::basic_type> type() const { return _type; }
  bool is_typed(cdk::typename_type name) const { return _type->name() == name; }
  const std::string &name() const { return _name; }
  long value() const { return _value; }
  long value(long v) { return _value = v; }
  int qualifier() const { return _qualifier; }
  int offset() const { return _offset; }
  int offset(int o) { return _offset = o; }
  bool global() const { return _offset == 0; }
};

} // namespace til

#endif
