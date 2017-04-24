// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "common.h"
#include "fastserializer.h"

FastSerializer::FastSerializer(SString &outputFilePath)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
    }
    CONTRACTL_END;

    m_writeErrorEncountered = false;
    m_pFileStream = new CFileStream();
    if(FAILED(m_pFileStream->OpenForWrite(outputFilePath)))
    {
        delete(m_pFileStream);
        m_pFileStream = NULL;
        return;
    }

    // Write the file header.
    WriteFileHeader();
}

FastSerializer::~FastSerializer()
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
    }
    CONTRACTL_END;

    if(m_pFileStream != NULL)
    {
        delete(m_pFileStream);
        m_pFileStream = NULL;
    }
}

void FastSerializer::WriteFileHeader()
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
    }
    CONTRACTL_END;

    SString signature(W("!FastSerialization.1"));
    Write(signature);
}

void FastSerializer::Write(SString &contents)
{
    CONTRACTL
    {
        NOTHROW;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
    }
    CONTRACTL_END;

    if(m_writeErrorEncountered || m_pFileStream == NULL)
    {
        return;
    }

    EX_TRY
    {
        StackScratchBuffer scratch;
        // TODO: Does this need to be UTF16?
        const char * strContents = contents.GetANSI(scratch);
        ULONG inCount = contents.GetCount();
        ULONG outCount;
        m_pFileStream->Write(strContents, inCount, &outCount);

        if (inCount != outCount)
        {
            // This will cause us to stop writing to the file.
            // The file will still remain open until shutdown so that we don't have to take a lock at this level when we touch the file stream.
            m_writeErrorEncountered = true;
        }
    }
    EX_CATCH{} EX_END_CATCH(SwallowAllExceptions);
}
