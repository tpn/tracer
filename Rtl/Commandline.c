/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Commandline.c

Abstract:

    This module implements functionality related to command line handling.
    Routines are provided to convert argument vectors from Unicode to ANSI.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
ArgvWToArgvA(
    PPWSTR ArgvW,
    ULONG NumberOfArguments,
    PPPSTR ArgvAPointer,
    PSTR Argv0,
    PALLOCATOR Allocator
    )
/*++

Routine Description:

    Converts a wide string argv to a utf-8 format one.

Arguments:

    ArgvW - Supplies a pointer to the source argv, obtained from
        CommandLineToArgvW().

    NumberOfArguments - Supplies the number of arguments in the ArgvW
        array.

    ArgvA - Supplies a pointer to an argv array that will receive the
        address of the UTF-8 converted arguments.

    Argv0 - Supplies a pointer to an optional character array that
        will be used for the first (0-index) argv item in the ArgvA
        array.  If NULL, no replacement will be made.

    Allocator - Supplies a pointer to the memory allocator to use.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL ReplaceArgv0;
    USHORT Index;
    PPSTR AnsiArgv;
    PSTR AnsiArg;
    PWSTR UnicodeArg;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(ArgvW)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ArgvAPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (NumberOfArguments == 0 || NumberOfArguments > MAX_ARGV) {
        return FALSE;
    }

    ReplaceArgv0 = (Argv0 != NULL);

    //
    // Calculate required space for the argv array, including a final
    // terminating NULL pointer.
    //

    AnsiArgv = (PPSTR)(
        Allocator->Calloc(
            Allocator->Context,
            NumberOfArguments + 1,
            sizeof(PSTR)
        )
    );

    if (!AnsiArgv) {
        goto Error;
    }

    //
    // Loop through the arguments, get the utf-8 multi-byte size,
    // allocate memory and do the utf-8 conversion into the newly
    // allocated buffer.
    //

    for (Index = 0; Index < NumberOfArguments; Index++) {
        INT Size;

        UnicodeArg = ArgvW[Index];

        if (ReplaceArgv0 && Index == 0) {
            AnsiArgv[Index] = Argv0;
            continue;
        }

        //
        // Get the number of bytes necessary to hold the string.
        //

        Size = WideCharToMultiByte(
            CP_UTF8,
            0,
            UnicodeArg,
            -1,
            NULL,
            0,
            NULL,
            0
        );

        if (Size <= 0) {
            goto Error;
        }

        //
        // Allocate zeroed memory.
        //

        AnsiArg = (PSTR)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                Size + 1
            )
        );

        if (!AnsiArg) {
            goto Error;
        }

        //
        // As soon as we've got a valid pointer, assign it to the argv array
        // so that it can be free'd if we encounter an error after this point.
        //

        AnsiArgv[Index] = AnsiArg;

        //
        // Now do the actual conversion against the newly allocated buffer.
        //

        Size = WideCharToMultiByte(
            CP_UTF8,
            0,
            UnicodeArg,
            -1,
            AnsiArg,
            Size,
            NULL,
            0
        );

        if (Size <= 0) {
            goto Error;
        }

        //
        // NULL terminate the string.
        //

        AnsiArg[Size] = '\0';

    }

    //
    // Success, everything was converted.  Update caller's pointer.
    //

    *ArgvAPointer = AnsiArgv;

    return TRUE;

Error:

    //
    // If the AnsiArgv array exists, loop through and free any pointers
    // we find.
    //

    if (AnsiArgv) {

        for (Index = 0; Index < NumberOfArguments; Index++) {

            if (Index == 0 && ReplaceArgv0) {

                //
                // We don't own Argv0, so don't try and free it.
                //

                continue;
            }

            AnsiArg = AnsiArgv[Index];

            if (AnsiArg) {
                Allocator->Free(Allocator->Context, AnsiArg);
                AnsiArgv[Index] = NULL;
            }
        }

        Allocator->Free(Allocator->Context, AnsiArgv);
    }

    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
