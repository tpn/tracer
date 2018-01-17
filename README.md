# Tracer

An experimental tracing framework for Python.

Some code snippets: [FillPages](https://github.com/tpn/tracer/blob/master/Asm/FillPages.asm), an AVX2-optimized assembly routine for filling pages with a particular byte pattern.  [InjectionThunk](https://github.com/tpn/tracer/blob/master/Asm/InjectionThunk.asm#L357), an assembly routine used for remote process code injection -- includes support for injection events and shared memory as well.  [PrefixSearchStringTable](https://github.com/tpn/tracer/blob/master/StringTable/PrefixSearchStringTable.c), an AVX2-optimized routine for searching whether or not a given string prefix-matches a set of strings in a custom string table.  [RegisterFrame](https://github.com/tpn/tracer/blob/master/Python/PythonFunction.c), the main frame registration routine used by the Python tracing component.

# Introduction
## Motivation, Existing Works
The Tracer project was inspired by consulting work in New York city.  Often, I was tasked with joining a team with an existing, large code base, and improving performance.  Alternatively, new code would be written that would exhibit poor performance, and assistance was required to determine why.

I had been using three tools: the existing Python profiling functionality (cProfile), a line profiler (kernprof), and Visual Studio's Python profiling feature.

cProfile is cumulative in nature.  Call sites are recorded and functions are depicted by way of both number of calls and cumulative call time.  It is useful for detecting "red flag" performance problems -- an example I saw in real life was 30% of execution time being spent in gc.collect().  This is indicative of pathological performance problems, and was traced to some pandas behavior regarding temporary assignments.

For long-running, batch-oriented programs, though, where there are no pathological corner cases, the usefulness of cProfile diminishes.  It becomes hard to distinguish signal from noise with regards to the cumulative call counts and function time.  Additionally, cProfile only captures function call entry and exit times -- it doesn't do line profiling, and thus, won't be able to depict abnormally expensive lines within a routine (this can be common with NumPy and Pandas; a seemingly innocuous line will actually have a very high overhead behind the scenes).

A line profiling tool exists called kernprof.  This enables line tracing in the Python interpreter, which can depict the cost of each line within a given function call.  This is useful for identifying why a large function is exhibiting poor performance.  However, it requires knowing the problematic function ahead of time.  Functions must be programmatically "registered" for line-tracing at runtime.  This quickly becomes very tedious for large code bases, especially if you're unfamiliar with the code base, and thus, not sure which functions should be inspected closer and which should be ignored.

The Python Tools for Visual Studio component has a custom Python profiling feature which actually leverages the underlying Visual Studio performance profiling functionality.  It depicts richer call stack information, which allows visually identifying problematic areas in a much easier fashion than the other two tools.  However, the overhead incurred by the profiler is significant -- something that took 5 minutes to run could take hours to run when being profiled.  Additionally, the time taken to process the saved profile data would also be quite significant.

Each tool has various pros and cons.  The source code is actually available for all three, and subsequent study proved quite beneficial in identifying approaches to various problems that were worth adopting, as well as areas where some improvements could be made if a more sophisticated architecture was put in place.

The use of splay-trees by the cProfile component interested me.  The code responsible for registering new Python functions heavily uses the NT kernel's generic table APIs, which use splay trees behind the scenes.

I wanted to design something that supported line-level tracing at the Python module level.  For a given project, there were only ever a couple of top-level Python module names I was interested in.  Being able to specify a module prefix name (e.g. numpy, pandas and myprojectfoo) and ignoring everything else was important.  This lead to the development of the StringTable component, which is an AVX2 accelerated prefix string matching algorithm geared specifically toward the needs of the tracing project.

I also knew I wanted to leverage memory maps extensively.  The tracing facilities should be able to request blocks of memory and then be given back a pointer that they're free to write to.  The underlying operating system's memory and file system manager would take care of ensuring the data was sync'd to disk as necessary.  Additionally, it should be possible to load the data files in a read-only capacity (whilst the tracing data was being captured) to monitor performance in a live fashion.

This lead to the development of the TraceStore component, which underpins the vast majority of the tracing infrastructure.

# C without the C Runtime Library

The Tracer project is implemented in C.  However, it does not link to, use, or otherwise depend upon the C runtime library (CRT).  Instead, it relies on equivalent functionality present in the NT kernel, by way of the ntdll.dll, kernel32.dll and ntoskrnl.exe images.  Extensive use is made of the NT runtime library -- in fact, the main Tracer runtime library component is named "Rtl" after the similarly named NT kernel component.

The Tracer project's Rtl module defines many function pointer types -- some mirror identically-named NT procedures (e.g. RtlSplayLinks has a function pointer typedef named PRTL_SPLAY_LINKS), the rest are common procedures implemented internally (i.e. they will have .c implementations living in the Rtl/ directory) that are of use to other components in the project.  For example, path and environment variable helper procedures.

The interface to the Rtl module's functionality is by way of a monolothic structure named RTL.  This structure is predominantly composed of function pointers to either NT-implemented or internally-implemented functionality.  A single RTL structure is typically initialized once per program instance at the very start by the bootstrap sequence (initialize an allocator via DefaultHeapInitializeAllocator(), then call CreateAndInitializeTracerConfigAndRtl()).

A pointer to the structure (of type PRTL) is then commonly passed around between downstream components.  It will frequently be observed as the first parameter of a procedure, or captured as a function pointer field (`PRTL Rtl`) in a downstream C structure somewhere (for example, a runtime context).

No module currently uses more than one instance of an RTL structure, although there is technically nothing preventing LoadRtl() from being called to initialize multiple instances.  It is assumed that once loaded and initialized, the RTL structure will stay valid until program termination.  There is a DestroyRtl() procedure, however, as LoadRtl() requires the caller to provide the memory backing the structure, no memory allocation is done by this routine -- it simply calls FreeLibrary() on ntdll.dll, kernelbase.dll, kernel32.dll, and ntoskrnl.exe images.  As these images are typically permanently mapped into a process's address space, the FreeLibrary() call is technically unnecessary, at least from the perspective that it won't ever result in the image being unloaded.  It is good discipline, though, to pair LoadLibrary() calls with matching FreeLibrary() calls, so this is adhered to by DestroyRtl().

From the perspective of the downstream module author, the Rtl pointer (obtained either via a procedure parameter or embedded in a structure) can be assumed to always be non-NULL and valid.

## Motivation Behind Not Relying on the C Runtime Library
	1. Multiple C runtimes.
	2. A runtime dependency on the CRT (i.e. the binary has an imports section with msvcrt stuff).
	3. Avoid peturbing the traced environment.
	
## The Mechanics of C without the C Runtime Library
	
	1. Visual Studio setup.
	2. __chkstack
	3. __C_runtime_handler
	
# Style
The Tracer project uses the Cutler Normal Form (CNF) coding style for the C language.  This is the style used for the NT kernel; the name Cutler Normal Form parallels the Kernel Normal Form style that has guided UNIX kernel development over the years.  For those with exposure to the NT kernel style (e.g. via Windows headers or NT driver development), the Tracer project will have a familiar feel.  For those coming from a UNIX or Linux background, the style can appear quite jarring, as it shares almost no traits with the type of C style programs in that domain.  The only commonality between the two styles is that they're both very consistent internally, and produce very clean looking code.  Each style is an artefact of the environment it was created in.

## Intrinsic C Types
Type names are always upper case.  Intrinsic C types map to NT types as follows:

C Type        NT Type (typedef)
char          CHAR
unsigned char	BYTE
xxx finish me

Pointer typedefs are used to capture pointer types, e.g. PULONG is used instead of `unsigned long *`.

## Structures, Unions and Enums
Structures, unions and enumerations all follow the same pattern:

	typedef struct _FOO {
		USHORT SizeOfStruct;
		USHORT Padding[3];
		
		ULONG SequenceId;
		ULONG Checksum;
		
		PRUNTIME_CONTEXT Context;
	} FOO;
	typedef FOO *PFOO;
	typedef FOO **PPFOO;

The structure name is always prefixed with an underscore, and defined via a typedef, with the type name matching the structure name, sans underscore.  The structure is then followed by at least one pointer typedef, the name for which always matches the target type and has a P prefixed to it.  Some types may also have pointer-to-pointer-of typedefs, in which case, two Ps are prefixed to the name.

The same rules apply for unions and enumerations.  For example:

	typedef union _BAR {
		CHAR Character;
		BYTE Byte;
		ULONG Integer;
	} BAR;
	typedef BAR *PBAR;
	
## Function Pointers

The Tracer project makes extensive use of named function pointer types via typedef.  The public procedures for a module will be declared via a function typedef with SAL annotations, plus a supporting function pointer typedef.  This allows subsequent definition of the procedure implementation to use the _Use_decl_annotations_ SAL annotation, removing the need to duplicate annotations.

	typedef
	BOOL
	(CREATE_RANDOM_OBJECT_NAMES)(
	    _In_ PRTL Rtl,
	    _In_ PALLOCATOR TemporaryAllocator,
	    _In_ PALLOCATOR WideBufferAllocator,
	    _In_ USHORT NumberOfNames,
	    _In_ USHORT LengthOfNameInChars,
	    _In_opt_ PUNICODE_STRING NamespacePrefix,
	    _In_ PPUNICODE_STRING NamesArrayPointer,
	    _In_opt_ PPUNICODE_STRING PrefixArrayPointer,
	    _Out_ PULONG SizeOfWideBufferInBytes,
	    _Out_writes_bytes_all_(*SizeOfWideBufferInBytes) PPWSTR WideBufferPointer
	    );
	typedef CREATE_RANDOM_OBJECT_NAMES *PCREATE_RANDOM_OBJECT_NAMES;
	
## Bitfields, Flags and State

	typedef union _FOO_STATE {
		struct _Struct_size_bytes_(sizeof(ULONG)) {
			ULONG Initialized:1;
			ULONG Executing:1;
			
			ULONG Unused:30;
		};
		
		LONG AsLong;
		ULONG AsULong;
	} FOO_STATE;
	C_ASSERT(sizeof(FOO_STATE) == sizeof(ULONG));
	typedef FOO_STATE *PFOO_STATE;

## Indentation, Line Width, Invariants, Procedure Organization and Gotos
Indentation is four spaces and lines are limited to 80 characters maximum.  Curly braces are used for all if statements and loop constructures, even if the subsequent block consists of a single statement (such that the braces could be omitted).  Line spacing is generous -- new lines are used frequently to separate logical chunks of code.

Comments are always preceeded with an empty leading and trailing line consisting only of the // characters.  Additionally, they have blank lines proceeding and succeeding them.  /* */ style comments are not used; only the double forward slash style of comments are used.

Invariant checks are frequent.  If an invariant test fails, it is followed by a __debugbreak(), and then typically via a `return FALSE;` type statement, e.g.:

	
	    //
	    // Adjust the runtime function entry details for RtlAddFunctionTable().
	    //
	
	    Thunk->EntryCount = EntryCount;
	    Thunk->FunctionTable = RemoteRuntimeFunction;
	    Thunk->BaseCodeAddress = RemoteCodeBaseAddress;
	
	    //
	    // Invariant checks.
	    //
	
	    NewUserData = Base + TotalBytes;
	    if (NewUserData != Dest) {
	        __debugbreak();
	        return FALSE;
	    }
	
This results in an `int 3` instruction (on x86/x64 processors) being executed as soon as an invariant fails.  If the program is being interactively debugged (via Visual Studio or WinDbg, for example), program execution will immediately halt.  If no debugger is attached, the standard protocol for debugging on Windows will kick in, and will allow a debugger to be attached.

Gotos are acceptable, and are used extensively for handling errors and facilitating single-point-of-exit procedures.  Procedures that implement state-driven logic also make use of gotos.

The organization of branches and basic blocks is driven by the branch prediction behavior of modern CPUs, which will predict forward branches as "not taken", and backwards branches as "taken".  Although basic block re-ordering can be optimized by compiler heuristics, and then significantly optimized via profile guided optimization -- structuring the code such that it aligns with the CPU's default branch prediction algorithm is preferred.  This is especially important for hot-path code, which may be called trillions of times for a given tracing session.

(As a side note: PGO builds of the Tracer project do not exhibit significantly lower latencies on the hot path than their normal static optimization counterparts.  Additionally, there is very little in the way of abstraction with regards to the entire code base, which is typically an area where PGO builds can provide significant improvements.)

## Procedure Documentation

The definition of procedures should include documentation that includes (at least) the following sections: Routine Description, Arguments and Return Value.  For example:

	_Use_decl_annotations_
	BOOL
	RegisterPythonFunction(
	    PPYTHON Python,
	    PPYTHON_FUNCTION Function,
	    PPYFRAMEOBJECT FrameObject
	    )
	/*++
	
	Routine Description:
	
	    This method is responsible for finalizing details about a function, such
	    as the function name, class name (if any), module name and first line
	    number.  A frame object is provided to assist with resolution of names.
	
	    It is called once per function after a new PYTHON_PATH_TABLE_ENTRY has been
	    inserted into the path prefix table.
	
	Arguments:
	
	    Python - Supplies a pointer to a PYTHON structure.
	
	    Function - Supplies a pointer to a PYTHON_FUNCTION structure to be
	        registered by this routine.
	
	    FrameObject - Supplies a pointer to the PYFRAMEOBJECT structure for which
	        this function is being registered.  This is required in order to assist
	        with the resolution of names.
	
	Return Value:
	
	    TRUE on success, FALSE on failure.
	
	--*/
	{


## File Header Documentation

Each file should have a header that contains (at least) a copyright string, the module name, and an abstract briefly describing the role of the module and the functions it provides.

	/*++
	
	Copyright (c) 2016 Trent Nelson <trent@trent.me>
	
	Module Name:
	
	    PythonFunction.c
	
	Abstract:
	
	    This module implements functionality related to the PYTHON_FUNCTION
	    structure, which is based on the PYTHON_PATH_TABLE_ENTRY structure,
	    additionally capturing Python function information such as the code
	    object, number of lines, etc.
	
	    This module is consumed by Python tracing components, which interface
	    to it via the RegisterFrame method, which returns the corresponding
	    PYTHON_FUNCTION structure for a given Python frame object.
	
	--*/

## Interfacing with External Components

The Tracer project code style values readability and consistency.  The vast majority of interaction is done with NT components, and thus, identical styles for procedure names and types result in readable and consistent code.  However, the NT style is rarely found elsewhere, especially not in open source or 3rd party components.  Interfacing with libraries that use different styles for type naming affects the readability and consistency of the resulting code.

The alternative is to furnish a header file with duplicate function and type declarations using the NT style.  A structure is then defined which contains function pointers to the relevant public methods of the external component, and the methods are resolved at runtime via the Rtl component's LoadSymbols() function.

This pattern is used for interfacing with Python, Cuda and Sqlite.  An additional benefit to runtime resolution of symbols is that it allows for a single structure of function pointers to easily cater for different versions of the external component.  For example, the Python component will happily adapt to a target Python 2.x or Python 3.x process.

	Python/Python.h
	Rtl/Cu.h
	Rtl/Sqlite.h
