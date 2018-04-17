/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    TestStringTable.c

Abstract:

    This module implements some internal tests of the StringTable component.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOLEAN
TestIsPrefixOfStringInTableFunctions(
    PRTL Rtl,
    PALLOCATOR StringTableAllocator,
    PALLOCATOR StringArrayAllocator,
    PSTRING_ARRAY StringArray,
    PSTRING_TABLE_ANY_API AnyApi,
    ULONG SizeOfAnyApi,
    PCSTRING_TABLE_FUNCTION_OFFSET Functions,
    ULONG NumberOfFunctions,
    PCSTRING_TABLE_TEST_INPUT TestInputs,
    ULONG NumberOfTestInputs,
    BOOLEAN DebugBreakOnTestFailure,
    BOOLEAN AlignInputs,
    PULONG NumberOfFailedTests,
    PULONG NumberOfPassedTests
    )
/*++

Routine Description:

    Enumerate all test inputs, optionally verifying results, for a given set
    of IsPrefixOfStringInTable function pointers.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    StringTableAllocator - Supplies a pointer to an ALLOCATOR structure which
        will be used for creating the STRING_TABLE.  This is passed directly
        to the AnyApi->CreateStringTable() function.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure which
        may be used to create the STRING_ARRAY if it cannot fit within the
        padding of the STRING_TABLE structure.  This is kept separate from the
        StringTableAllocator due to the stringent alignment requirements of the
        string table.  As with StringTableAllocator, this parameter is passed
        directly to the AnyApi->CreateStringTable() function.

    StringArray - Supplies a pointer to an initialized STRING_ARRAY structure
        for which a STRING_TABLE structure is to be created and subsequently
        tested.

    AnyApi - Supplies a pointer to a STRING_TABLE_API-based structure.

    SizeOfAnyApi - Supplies the size of the AnyApi structure, in bytes.  The
        offsets provided by the Functions parameter will be validated against
        the API size, e.g. if the STRING_TABLE_API structure is passed, this
        routine will error out if any of the function offsets are from the
        STRING_TABLE_API_EX structure.

    Functions - Supplies an array of string table function offsets.

    NumberOfFunctions - Supplies the number of elements in the Functions
        parameter above.  This is used to govern iteration over the array.

    TestInputs - Supplies an array of string table test inputs.

    NumberOfTestInputs - Supplies the number of elements in the TestInputs
        parameter above.  This is used to govern iteration over the array.

    DebugBreakOnTestFailure - Supplies a boolean variable that, when set to
        TRUE, indicates that the routine should __debugbreak() as soon as a
        test failure is detected.  This will also apply to internal validation
        errors, such as incorrect input string sizes or API size mismatches.

    AlignInputs - Supplies a boolean variable that, when set to TRUE, indicates
        that test inputs should be copied into an ALIGNED_BUFFER structure first
        before the IsPrefix function pointer will be called.  This guarantees
        that the character buffers will be aligned on 32 byte boundaries.  If
        FALSE, the STRING structures referenced by each test input will be used
        directly.

        N.B. If TRUE, the test input string must be no longer than 32 bytes,
             as this is the size of the ALIGNED_BUFFER structure.  If an input
             string longer than 32 is detected, this routine will return an
             error.

    NumberOfFailedTests - Supplies the address of a variable that receives a
        count of the number of failed tests that occur.  This should always be
        checked to determine if any tests failed.

    NumberOfPassedTests - Supplies the address of a variable that receives a
        count of the number of tests that passed.

Return Value:

    TRUE on success, FALSE on error.

    N.B. TRUE does not imply that there were no test failures, only that no
         internal errors occurred.  An internal error would be something like
         an allocation failure or invalid parameter.

--*/
{
    BOOLEAN Success;
    ULONG Passed;
    ULONG Failed;
    ULONG FuncIndex;
    ULONG InputIndex;
    ULONG NumberOfSymbols;
    ULONG MaxFunctionOffset;
    STRING AlignedInput;
    PSTRING InputString;
    STRING_MATCH Match;
    PSTRING_TABLE StringTable;
    ALIGNED_BUFFER InputBuffer;
    STRING_TABLE_INDEX Result;
    PCSTRING_TABLE_TEST_INPUT Input;
    PCSTRING_TABLE_FUNCTION_OFFSET Func;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefix;
    PSTRING_TABLE_API_EX Api;

    //
    // Define a helper macro for potentially dispatching a debugbreak if
    // requested by the user when an error or test failure is encountered.
    //

#ifdef MAYBE_BREAK
#undef MAYBE_BREAK
#endif
#define MAYBE_BREAK()              \
    if (DebugBreakOnTestFailure) { \
        __debugbreak();            \
    }

    //
    // Validate arguments.
    //

    if (SizeOfAnyApi == sizeof(AnyApi->Api)) {
        NumberOfSymbols = sizeof(AnyApi->Api) / sizeof(ULONG_PTR);
    } else if (SizeOfAnyApi == sizeof(AnyApi->ApiEx)) {
        NumberOfSymbols = sizeof(AnyApi->ApiEx) / sizeof(ULONG_PTR);
    } else {
        MAYBE_BREAK();
        return FALSE;
    }

    //
    // Set the maximum function offset permitted in the functions array based
    // on the number of symbols identified by the API size, above.
    //

    MaxFunctionOffset = NumberOfSymbols * sizeof(ULONG_PTR);

    //
    // Clear our caller's count parameters up front.
    //

    *NumberOfFailedTests = 0;
    *NumberOfPassedTests = 0;

    //
    // Initialize local API alias and attempt to create a string table.
    //

    Api = &AnyApi->ApiEx;

    StringTable = Api->CreateStringTable(Rtl,
                                         StringTableAllocator,
                                         StringArrayAllocator,
                                         StringArray,
                                         TRUE);
    if (!StringTable) {
        MAYBE_BREAK();
        return FALSE;
    }

    //
    // Clear our aligned input buffer and wire it up to the AlignedInput's
    // Buffer field.  Initialize the maximum length field; we use this later
    // to verify the input string length before copying over.
    //

    ZeroStruct(InputBuffer);
    AlignedInput.Buffer = (PCHAR)&InputBuffer.Chars[0];
    AlignedInput.Length = 0;
    AlignedInput.MaximumLength = sizeof(InputBuffer);

    //
    // Clear our match structure.
    //

    ZeroStruct(Match);

    //
    // Zero counters.
    //

    Passed = 0;
    Failed = 0;

    //
    // Loop through all test inputs, then loop through all functions, executing
    // each one and verifying the result is as we expect.
    //

    for (InputIndex = 0; InputIndex < NumberOfTestInputs; InputIndex++) {

        //
        // Resolve this iteration's test input.
        //

        Input = &TestInputs[InputIndex];

        if (!AlignInputs) {

            //
            // No aligned input has been requested.  Wire up the test string
            // as the input string.
            //

            InputString = Input->String;

        } else {

            //
            // Aligned input has been requested.  Verify the test input string
            // length does not exceed the aligned input buffer's maximum length,
            // and then copy over.
            //

            if (Input->String->Length > AlignedInput.MaximumLength) {

                //
                // Test input string was too long.  Potentially debugbreak,
                // then error out.
                //

                MAYBE_BREAK();

                goto Error;
            }

            //
            // Input was valid, copy it over to the aligned input buffer and
            // wire up the input string.
            //

            CopyMemory(&InputBuffer,
                       Input->String->Buffer,
                       Input->String->Length);

            AlignedInput.Length = Input->String->Length;
            InputString = &AlignedInput;

        }

        for (FuncIndex = 0; FuncIndex < NumberOfFunctions; FuncIndex++) {

            //
            // Resolve this iteration's function pointer.
            //

            Func = &Functions[FuncIndex];

            //
            // Verify the embedded offset does not exceed our API boundaries.
            //

            if (Func->Offset > MaxFunctionOffset) {

                //
                // Function offset exceeded the limits of our API structure,
                // potentially debugbreak, then error out.
                //

                MAYBE_BREAK();

                goto Error;

            }

            //
            // Function offset is valid, convert it into a function pointer.
            //

            IsPrefix = LOAD_FUNCTION_FROM_OFFSET(Api, Func->Offset);

            //
            // We're finally ready to invoke the function!
            //

            Result = IsPrefix(StringTable, InputString, &Match);

            //
            // If requested, verify the result is what we expect.
            //

            if (Func->Verify) {

                BOOLEAN Verified = TRUE;

                if (Result != Input->Expected) {

                    Verified = FALSE;
                    MAYBE_BREAK();

                } else if (Result != NO_MATCH_FOUND) {

                    //
                    // The return value matched what we expected, and we
                    // expected a prefix match.  Verify the string match
                    // structure corroborates the expected result.
                    //

                    BOOLEAN ValidMatchIndex;
                    BOOLEAN ValidMatchString;
                    BOOLEAN ValidMatchCharacters;

                    ValidMatchIndex = (Match.Index == Result);

                    ValidMatchString = (
                        Match.String ==
                        &StringTable->pStringArray->Strings[Match.Index]
                    );

                    ValidMatchCharacters = (
                        InputString->Length ==
                        Match.NumberOfMatchedCharacters
                    );

                    Verified = (
                        ValidMatchIndex      &&
                        ValidMatchString     &&
                        ValidMatchCharacters
                    );

                    //
                    // N.B. We break here as the individual boolean variables
                    //      will still be visible to a debugger, which helps
                    //      during troubleshooting.
                    //

                    if (!Verified) {
                        MAYBE_BREAK();
                    }

                }

                //
                // Update relevant counter.
                //

                if (Verified) {
                    Passed++;
                } else {
                    Failed++;
                }
            }
        }
    }

    //
    // We're finished, indicate success and goto End.
    //

    Success = TRUE;

    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Destroy the string table if one was created.
    //

    if (StringTable) {
        Api->DestroyStringTable(StringTableAllocator,
                                StringArrayAllocator,
                                StringTable);
        StringTable = NULL;

    }

    //
    // Update the caller's count parameters.
    //

    *NumberOfFailedTests = Failed;
    *NumberOfPassedTests = Passed;

    //
    // Return success.
    //

    return Success;
}



// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
