// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef __EVENTPIPE_EVENTINSTANCE_H__
#define __EVENTPIPE_EVENTINSTANCE_H__

#include "eventpipe.h"
#include "eventpipeevent.h"
#include "fastserializer.h"

class EventPipeEventInstance
{

public:

    EventPipeEventInstance(EventPipeEvent &event, Thread *pThread, BYTE *pData, size_t length);

    // Get the stack contents object to either read or write to it.
    StackContents* GetStack();

    // Static serialize function that can be called by FastSerializer.
    static void Serialize(FastSerializer *pSerializer, EventPipeEventInstance *pInstance);

    // Called from the static Serialize method to do the actual work.
    void Serialize(FastSerializer *pSerializer);

private:

    EventPipeEvent *m_pEvent;
    Thread *m_pThread;
    LARGE_INTEGER m_pTimeStamp;

    BYTE *m_pData;
    size_t m_dataLength;
    StackContents m_stackContents;
};

#endif // __EVENTPIPE_EVENTINSTANCE_H__


