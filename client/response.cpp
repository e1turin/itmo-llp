#include "response.hpp"

void Response::dump(std::ostream& out) {
  out << "Response: " << std::endl;
  out << "Error: " << error << std::endl;
  if (error == 0) {
    for (const auto& el : elements) {
      out << "-------------------" << std::endl;
      out << "Element type: " << el.type << std::endl;
      for (size_t i = 0; i < el.attrs.size(); ++i) {
        out << i << " = ";
        switch (el.attrs[i].type) {
          case ATTR_TYPE_INT64:
            out << "(int64) " << el.attrs[i].value.as_int64;
            break;
          case ATTR_TYPE_INT32:
            out << "(int32) " << el.attrs[i].value.as_int32;
            break;
          case ATTR_TYPE_BOOL:
            out << "(bool) " << el.attrs[i].value.as_bool;
            break;
          case ATTR_TYPE_DOUBLE:
            out << "(double) " << el.attrs[i].value.as_double;
            break;
          case ATTR_TYPE_STRING:
            out << "(string) " << el.attrs[i].value.as_string.data;
            break;
        }
        out << std::endl;
      }
      out << "-------------------" << std::endl;
    }
  }
}
