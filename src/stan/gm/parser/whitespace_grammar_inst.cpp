#include <stan/gm/parser/whitespace_grammar_def.hpp>
#include <stan/gm/parser/iterator_typedefs.hpp>

namespace stan {
  namespace gm {
    template struct whitespace_grammar<pos_iterator_t>;
  }
}
