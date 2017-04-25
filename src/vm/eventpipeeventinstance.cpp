// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "common.h"
#include "eventpipeeventinstance.h"
#include "fastserializer.h"

EventPipeEventInstance::EventPipeEventInstance(
    EventPipeEvent &event,
    Thread *pThread,
    BYTE *pData,
    size_t length)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
        PRECONDITION(pThread != NULL);
    }
    CONTRACTL_END;

    m_pEvent = &event;
    m_pThread = pThread;
    m_pData = pData;
    m_dataLength = length;
}

StackContents* EventPipeEventInstance::GetStack()
{
    LIMITED_METHOD_CONTRACT;

    return &m_stackContents;
}

void EventPipeEventInstance::Serialize(FastSerializer *pSerializer, EventPipeEventInstance *pInstance)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
        PRECONDITION(pSerializer != NULL);
        PRECONDITION(pInstance != NULL);
    }
    CONTRACTL_END;

    pInstance->Serialize(pSerializer);
}

void EventPipeEventInstance::Serialize(FastSerializer *pSerializer)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
        PRECONDITION(pThread != NULL);
    }
    CONTRACTL_END;

    // TODO: Serialize the event using the serializer.
}
