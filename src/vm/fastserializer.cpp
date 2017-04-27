// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include "common.h"
#include "fastserializer.h"

FastSerializer::FastSerializer(SString &outputFilePath, FastSerializableObject &object)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
    }
    CONTRACTL_END;

    m_writeErrorEncountered = false;
    m_pEntryObject = &object;
    m_currentPos = 0;
    m_pFileStream = new CFileStream();
    if(FAILED(m_pFileStream->OpenForWrite(outputFilePath)))
    {
        delete(m_pFileStream);
        m_pFileStream = NULL;
        return;
    }

    // Write the file header.
    WriteFileHeader();

    // Write the entry object.
    WriteEntryObject();
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

    // Write the end of the entry object.
    WriteTag(FastSerializerTags::EndObject);

    if(m_pFileStream != NULL)
    {
        delete(m_pFileStream);
        m_pFileStream = NULL;
    }
}

unsigned int FastSerializer::GetStreamLabel() const
{
    LIMITED_METHOD_CONTRACT;

    return m_currentPos;
}

void FastSerializer::GoToStreamLabel(unsigned int streamLabel)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
    }
    CONTRACTL_END;

    // Seek to the stream label.
    LARGE_INTEGER streamPos;
    streamPos.QuadPart = streamLabel;
    m_pFileStream->Seek(streamPos, STREAM_SEEK_SET, NULL);
}

void FastSerializer::WriteObject(FastSerializableObject *pObject)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_ANY;
        PRECONDITION(pObject != NULL);
    }
    CONTRACTL_END;

    // Write a BeginObject tag.
    WriteTag(FastSerializerTags::BeginObject);

    // Write object begin tag.
    WriteSerializationType(pObject);

    // Ask the object to serialize itself using the current serializer.
    pObject->FastSerialize(this);

    // Write object end tag.
    WriteTag(FastSerializerTags::EndObject);
}

void FastSerializer::WriteBuffer(BYTE *pBuffer, size_t length)
{
    CONTRACTL
    {
        NOTHROW;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
        PRECONDITION(pBuffer != NULL);
        PRECONDITION(length > 0);
    }
    CONTRACTL_END;

    if(m_writeErrorEncountered || m_pFileStream == NULL)
    {
        return;
    }

    EX_TRY
    {
        ULONG outCount;
        m_pFileStream->Write(pBuffer, length, &outCount);

#ifdef _DEBUG
        unsigned int prevPos = m_currentPos;
#endif
        m_currentPos += outCount;
#ifdef _DEBUG
        _ASSERTE(prevPos < m_currentPos);
#endif

        if (length != outCount)
        {
            // This will cause us to stop writing to the file.
            // The file will still remain open until shutdown so that we don't have to take a lock at this level when we touch the file stream.
            m_writeErrorEncountered = true;
        }
    }
    EX_CATCH
    {
        m_writeErrorEncountered = true;
    } 
    EX_END_CATCH(SwallowAllExceptions);
}

void FastSerializer::WriteEntryObject()
{
    CONTRACTL
    {
        NOTHROW;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
    }
    CONTRACTL_END;

    // Write begin entry object tag.
    WriteTag(FastSerializerTags::BeginObject);

    // Write the type information for the entry object.
    WriteSerializationType(m_pEntryObject);

    // The object is now initialized.  Fields or other objects can now be written.
}

void FastSerializer::WriteSerializationType(FastSerializableObject *pObject)
{
    CONTRACTL
    {
        NOTHROW;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
        PRECONDITION(pObject != NULL);
    }
    CONTRACTL_END;

    // Write the BeginObject tag.
    WriteTag(FastSerializerTags::BeginObject);

    // Write a NullReferenceTag, which implies that the following fields belong to SerializationType.
    WriteTag(FastSerializerTags::NullReference);

    // Write the SerializationType version fields.
    int serializationType[2];
    serializationType[0] = 1; // Object Version.
    serializationType[1] = 0; // Minimum Reader Version.
    WriteBuffer((BYTE*) &serializationType, sizeof(serializationType));

    // Write the SerializationType TypeName field.
    const char *strTypeName = pObject->GetTypeName();
    size_t length = strlen(strTypeName);
    WriteString(strTypeName, length);

    // Write the EndObject tag.
    WriteTag(FastSerializerTags::EndObject);
}


void FastSerializer::WriteTag(FastSerializerTags tag, BYTE *payload, size_t payloadLength)
{
    CONTRACTL
    {
        NOTHROW;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
        PRECONDITION( if (payload != NULL) { payLoadLength > 0 } );
    }
    CONTRACTL_END;

    WriteBuffer((BYTE *)&tag, sizeof(tag));
    if(payload != NULL)
    {
        WriteBuffer(payload, payloadLength);
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

    const char *strSignature = "!FastSerialization.1";
    size_t length = strlen(strSignature);
    WriteString(strSignature, length);
}

void FastSerializer::WriteString(const char *strContents, int length)
{
    CONTRACTL
    {
        THROWS;
        GC_TRIGGERS;
        MODE_PREEMPTIVE;
    }
    CONTRACTL_END;

    // Write the string length .
    WriteBuffer((BYTE*) &length, sizeof(int));

    // Write the string contents.
    WriteBuffer((BYTE*) strContents, length);
}
