/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqliteSchemas.c

Abstract:

    This module defines sqlite3 virtual table schemas.

--*/

#include "stdafx.h"

CONST CHAR TraceStoreAddressSchema[] =
    "CREATE TABLE Address ("
        "PreferredBaseAddress BIGINT, "
        "BaseAddress BIGINT, "
        "FileOffset BIGINT, "
        "MappedSize BIGINT, "
        "ProcessId INT, "
        "RequestingThreadId INT, "
        "Requested BIGINT, "
        "Prepared BIGINT, "
        "Consumed BIGINT, "
        "Retired BIGINT, "
        "Released BIGINT, "
        "AwaitingPreparation BIGINT, "
        "AwaitingConsumption BIGINT, "
        "Active BIGINT, "
        "AwaitingRelease BIGINT, "
        "MappedSequenceId INT, "
        "RequestingProcGroup SMALLINT, "
        "RequestingNumaNode TINYINT, "
        "FulfillingThreadId INT"
    ")";

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
