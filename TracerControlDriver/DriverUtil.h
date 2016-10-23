/*++


--*/

#pragma once

//
// ReadCr3
//

typedef
VOID
(READ_CR3)(
    _Out_ PULONGLONG Buffer
    );

typedef READ_CR3 *PREAD_CR3;

extern READ_CR3 ReadCr3;

//
// ReadDr7
//

typedef
VOID
(READ_DR7)(
    _Out_ PULONGLONG Buffer
    );

typedef READ_DR7 *PREAD_DR7;

extern READ_DR7 ReadDr7;


// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
