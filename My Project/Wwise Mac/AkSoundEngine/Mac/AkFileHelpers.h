/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2021.1.7  Build: 7796
  Copyright (c) 2006-2022 Audiokinetic Inc.
*******************************************************************************/
//////////////////////////////////////////////////////////////////////
//
// AkFileHelpers.h
//
// Platform-specific helpers for files.
//
//////////////////////////////////////////////////////////////////////

#ifndef _AK_FILE_HELPERS_H_
#define _AK_FILE_HELPERS_H_

#include <AK/Tools/Common/AkAssert.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <sys/stat.h>

#if defined(AK_EMSCRIPTEN) || defined(AK_QNX)
	#include <errno.h>
#else
	#include <sys/errno.h>
#endif

#include "../Common/AkFileHelpersBase.h"

#if defined(AK_GGP)
	#include "../GGP/AkFileHelpersGGP.h"
#elif defined(AK_IOS)
	#include "../iOS/AkFileHelpersiOS.h"
#elif defined(AK_LUMIN)
	#include "../Lumin/AkFileHelpersLumin.h"
#endif

class CAkFileHelpers : public CAkFileHelpersBase
{
public:

	// Wrapper for OS X CreateFile().
	static AKRESULT OpenFile( 
        const AkOSChar* in_pszFilename,     // File name.
        AkOpenMode      in_eOpenMode,       // Open mode.
        bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
        bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
        AkFileHandle &  out_hFile           // Returned file identifier/handle.
        )
	{
		// Check parameters.
		if ( !in_pszFilename )
		{
			AKASSERT( !"NULL file name" );
			return AK_InvalidParameter;
		}
		
		char* mode;
		switch ( in_eOpenMode )
		{
			case AK_OpenModeRead:
				mode = (char*)"r"; 
				break;
			case AK_OpenModeWrite:
				mode = (char*)"w";
				break;
			case AK_OpenModeWriteOvrwr:
				mode = (char*)"w+";
				break;
			case AK_OpenModeReadWrite:
				mode = (char*)"a";
				break;
			default:
					AKASSERT( !"Invalid open mode" );
					out_hFile = NULL;
					return AK_InvalidParameter;
				break;
		}
		
		out_hFile =	fopen( in_pszFilename , mode );

		if( !out_hFile )
		{
			return AK_FileNotFound;
		}

		return AK_Success;
	}
	
	//Open file and fill AkFileDesc
	static AKRESULT Open(
		const AkOSChar* in_pszFileName,     // File name.
		AkOpenMode      in_eOpenMode,       // Open mode.
		bool			in_bOverlapped,		// Overlapped IO
		AkFileDesc &    out_fileDesc		// File descriptor
		)
	{
		// Open the file without FILE_FLAG_OVERLAPPED and FILE_FLAG_NO_BUFFERING flags.
		AKRESULT eResult = OpenFile( 
			in_pszFileName,
			in_eOpenMode,
			in_bOverlapped,
			in_bOverlapped, //No buffering flag goes in pair with overlapped flag for now.  Block size must be set accordingly
			out_fileDesc.hFile );
		if ( eResult == AK_Success )
		{
			struct stat buf;
			// Get Info about the file size
			if( stat(in_pszFileName, &buf) != 0)
			{
				return AK_Fail;
			}
			out_fileDesc.iFileSize = buf.st_size;
		}
		return eResult;
	}

	// Wrapper for system file handle closing.
	static AKRESULT CloseFile( AkFileHandle in_hFile )
	{
		if ( !fclose( in_hFile ) )
			return AK_Success;
		
		AKASSERT( !"Failed to close file handle" );
		return AK_Fail;
	}

	//
	// Simple platform-independent API to open and read files using AkFileHandles, 
	// with blocking calls and minimal constraints.
	// ---------------------------------------------------------------------------

	// Open file to use with ReadBlocking().
	static AKRESULT OpenBlocking(
        const AkOSChar* in_pszFilename,     // File name.
        AkFileHandle &  out_hFile           // Returned file handle.
		)
	{
		return OpenFile( 
			in_pszFilename,
			AK_OpenModeRead,
			false,
			false, 
			out_hFile );
	}

	// Required block size for reads (used by ReadBlocking() below).
	static const AkUInt32 s_uRequiredBlockSize = 1;

	// Simple blocking read method.
	static AKRESULT ReadBlocking(
        AkFileHandle &	in_hFile,			// Returned file identifier/handle.
		void *			in_pBuffer,			// Buffer. Must be aligned on CAkFileHelpers::s_uRequiredBlockSize boundary.
		AkUInt32		in_uPosition,		// Position from which to start reading.
		AkUInt32		in_uSizeToRead,		// Size to read. Must be a multiple of CAkFileHelpers::s_uRequiredBlockSize.
		AkUInt32 &		out_uSizeRead		// Returned size read.        
		)
	{
		AKASSERT( in_uSizeToRead % s_uRequiredBlockSize == 0 
			&& in_uPosition % s_uRequiredBlockSize == 0 );

		if( fseek(in_hFile, in_uPosition, SEEK_SET ) )
		{
			return AK_Fail;
		}

		out_uSizeRead = (AkUInt32) fread( in_pBuffer, 1, in_uSizeToRead , in_hFile );
		
		if( out_uSizeRead == in_uSizeToRead )
		{
			return AK_Success;
		}
		
		return AK_Fail;		
	}

	/// Returns AK_Success if the directory is valid, AK_Fail if not.
	/// For validation purposes only.
	/// Some platforms may return AK_NotImplemented, in this case you cannot rely on it.
	static AKRESULT CheckDirectoryExists( const AkOSChar* in_pszBasePath )
	{
		struct stat status;
		stat( in_pszBasePath, &status );
		if( status.st_mode & S_IFDIR )
			return AK_Success;

		return AK_Fail;
	}

	static AKRESULT WriteBlocking(
		AkFileHandle &	in_hFile,			// Returned file identifier/handle.
		void *			in_pData,			// Buffer. Must be aligned on CAkFileHelpers::s_uRequiredBlockSize boundary.		
		AkUInt64		in_uPosition,		// Position from which to start writing.
		AkUInt32		in_uSizeToWrite)
	{
		
#if defined( AK_QNX ) || defined (AK_EMSCRIPTEN)
		if( !fseeko( in_hFile, in_uPosition, SEEK_SET ) )
#else
#ifdef AK_LINUX
		fpos_t pos;
		pos.__pos = in_uPosition;
#else
		fpos_t pos = in_uPosition;
#endif
		if( !fsetpos( in_hFile, &pos ) )
#endif
		{
			size_t itemsWritten = fwrite( in_pData, 1, in_uSizeToWrite, in_hFile );
			if( itemsWritten > 0 )
			{
				fflush( in_hFile );
				return AK_Success;
			}
		}
		return AK_Fail;
	}

	static AKRESULT CreateEmptyDirectory( const AkOSChar* in_pszDirectoryPath )
	{
		int iErr = mkdir(in_pszDirectoryPath, S_IRWXU | S_IRWXG | S_IRWXO);
		if (iErr == -1 && errno != EEXIST)
			return AK_Fail;

		return AK_Success;
	}

	static AKRESULT RemoveEmptyDirectory( const AkOSChar* in_pszDirectoryPath )
	{
		int iErr = rmdir(in_pszDirectoryPath);
		if (iErr == -1)
			return AK_Fail;

		return AK_Success;
	}

	static AKRESULT GetDefaultWritablePath(AkOSChar* out_pszPath, AkUInt32 in_pathMaxSize)
	{
		if (out_pszPath == nullptr)
			return AK_InsufficientMemory;

		out_pszPath[0] = '\0';

		#if defined(AK_GGP)
			return CAkFileHelpersGGP::GetDefaultWritablePath(out_pszPath, in_pathMaxSize);
		#elif defined(AK_LINUX_DESKTOP) || defined(AK_MAC_OS_X)
			// No strict writable path enforcement (return "")
		#elif defined(AK_IOS)
			return CAkFileHelpersiOS::GetDefaultWritablePath(out_pszPath, in_pathMaxSize);
		#elif defined(AK_LUMIN)
			return CAkFileHelpersLumin::GetDefaultWritablePath(out_pszPath, in_pathMaxSize);
		#else
			return AK_NotImplemented;
		#endif

		return AK_Success;
	}
};

#endif //_AK_FILE_HELPERS_H_
