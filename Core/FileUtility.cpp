//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "pch.h"
#include "FileUtility.h"
#include <fstream>
#include <mutex>
#include <zlib.h> // From NuGet package 

using namespace std;
using namespace Utility;

namespace Utility
{
    ByteArray NullFile = make_shared<vector<uint8_t> > (vector<uint8_t>() );
}

ByteArray DecompressZippedFile( wstring& fileName );

ByteArray ReadFileHelper(const wstring& fileName)
{
    struct _stat64 fileStat;
    int fileExists = _wstat64(fileName.c_str(), &fileStat);
    if (fileExists == -1)
        return NullFile;

    ifstream file( fileName, ios::in | ios::binary );
    if (!file)
        return NullFile;

    Utility::ByteArray byteArray = make_shared<vector<uint8_t> >( fileStat.st_size );
    file.read( (char*)byteArray->data(), byteArray->size() );
    file.close();

    return byteArray;
}

ByteArray ReadFileHelperEx( shared_ptr<wstring> fileName)
{
    return ReadFileHelper(*fileName);
}

ByteArray Utility::ReadFileSync( const wstring& fileName)
{
    return ReadFileHelperEx(make_shared<wstring>(fileName));
}

task<ByteArray> Utility::ReadFileAsync(const wstring& fileName)
{
    shared_ptr<wstring> SharedPtr = make_shared<wstring>(fileName);
    return create_task( [=] { return ReadFileHelperEx(SharedPtr); } );
}
