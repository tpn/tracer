#pragma once

#if DBG
#define ENTER(Name) DbgPrint("TracerControl!" Name ": Entered.\n")
#define LEAVE(Name) DbgPrint("TracerControl!" Name ": Leaving.\n")
#define LEAVE_STATUS(Name, Status) (                             \
    DbgPrint(                                                    \
        "TracerControl!" Name ": Leaving (NTSTATUS = 0x%0x).\n", \
        Status                                                   \
    )                                                            \
)
#define DEBUG(Message) DbgPrint("TracerControl!" Message)
#define DEBUG1(Message, Arg1) DbgPrint("TracerControl!" Message, Arg1)
#define DEBUG2(Message, Arg1, Arg2) \
    DbgPrint("TracerControl!" Message, Arg1, Arg2)
#else
#define ENTER(Name)
#define LEAVE(Name)
#define LEAVE_STATUS(Name, Status)
#define DEBUG(Message)
#define DEBUG1(Message, Arg1)
#define DEBUG2(Message, Arg1, Arg2)
#endif

// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
