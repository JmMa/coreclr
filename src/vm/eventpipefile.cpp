// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "common.h"
#include "eventpipefile.h"

EventPipeFile::EventPipeFile(SString &outputFilePath)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
    }
    CONTRACTL_END;

    m_pSerializer = new FastSerializer(outputFilePath, *this);
    QueryPerformanceCounter(&m_fileOpenTimeStamp);
    m_eventsWritten = 0;

    // Reserve enough space to write the event count.
    m_eventsWrittenStreamLabel = m_pSerializer->GetStreamLabel();
    m_pSerializer->WriteBuffer((BYTE*)&m_eventsWritten, sizeof(m_eventsWritten));
}

EventPipeFile::~EventPipeFile()
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
    }
    CONTRACTL_END;

    // Get the current stream label.
    unsigned int currentStreamLabel = m_pSerializer->GetStreamLabel();

    // Seek to the reserved event count location.
    m_pSerializer->GoToStreamLabel(m_eventsWrittenStreamLabel);

    // Write the event count.
    m_pSerializer->WriteBuffer((BYTE*)&m_eventsWritten, sizeof(m_eventsWritten));

    // Seek back to the current stream label.
    m_pSerializer->GoToStreamLabel(currentStreamLabel);

    // Close the serializer.
    if(m_pSerializer != NULL)
    {
        delete(m_pSerializer);
        m_pSerializer = NULL;
    }
}

void EventPipeFile::WriteEvent(EventPipeEventInstance &instance)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
    }
    CONTRACTL_END;

    // Write the event to the stream.
    instance.FastSerialize(m_pSerializer);
    m_eventsWritten++;
}
