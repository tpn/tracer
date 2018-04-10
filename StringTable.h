
//
// This file is used to simplify toggling of underlying StringTable
// implementations in a single location (versus having to change the
// include files of all dependent projects).
//

#if 0
#include "StringTable/StringTable.h"
#else
#include "StringTable2/StringTable.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
