/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqliteSchemas.c

Abstract:

    This module defines sqlite3 virtual table schemas.

--*/

#include "stdafx.h"

//
// Trace Store schemas.
//

extern CONST LPCSTR TraceStoreSchemas[];
extern CONST CHAR TraceStoreIntervalSchema[];
extern CONST PTRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3Columns[];

//
// Metadata Store schemas.
//

extern CONST CHAR TraceStoreMetadataInfoSchema[];
extern CONST CHAR TraceStoreAllocationSchema[];
extern CONST CHAR TraceStoreRelocationSchema[];
extern CONST CHAR TraceStoreAddressSchema[];
extern CONST CHAR TraceStoreAddressRangeSchema[];
extern CONST CHAR TraceStoreAllocationTimestampSchema[];
extern CONST CHAR TraceStoreAllocationTimestampDeltaSchema[];
extern CONST CHAR TraceStoreSynchronizationSchema[];
extern CONST CHAR TraceStoreInfoSchema[];

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
