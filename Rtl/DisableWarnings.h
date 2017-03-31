/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DisableWarnings.h

Abstract:

    This header file disables a common set of warnings for the codebase in
    order to still compile with the warning level set to 4.

--*/

#pragma once

//
// Disabled intentionally:
//
//  4201: nameless struct/union
//  4214: bit field types other than int
//  4115: named type definitions in parenthesis
//
// Currently disabled, but can be re-enabled once code is cleaned up.
//
//  4057: A* differs in indirection to slightly different base type from B*
//  4204: non-constant aggregate initializer
//  4221: A cannot be initialized using address of automatic variable B
//  4054: type-cast from function pointer to LPCTSTR
//

#pragma warning(disable: 4201 4214 4115 4057 4204 4221 4054)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
