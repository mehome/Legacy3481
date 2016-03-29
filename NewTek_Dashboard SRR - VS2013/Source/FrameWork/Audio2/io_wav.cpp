#include "stdafx.h"
#include "FrameWork.Audio2.h"

// Namespace stuff
using namespace FAUD;
using namespace FAUD::io;

// Open from a file
wav::wav( const wchar_t* p_fn )
{	// Sample data
	BYTE* p_sample_data = NULL;
	
	// The first thing that we do is open the file for unbuffered reading
	HANDLE hFile = ::CreateFileW( p_fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL );
	if ( hFile == INVALID_HANDLE_VALUE ) return;

	// We need to read in the first 512 bytes in order to check the header
	BYTE	hdr[ 512 ];
	DWORD	bytes_read;
	::ReadFile( hFile, hdr, 512, &bytes_read, NULL );
	if ( bytes_read != 512 ) goto finished;
	
	// Check the format
#pragma pack(push,1)
	struct	wav_header
	{	DWORD	ChunkID;
		DWORD	ChunkSize;
		DWORD	Format;
		DWORD	Subchunk1ID;
		DWORD	Subchunk1Size;
		WORD	AudioFormat;
		WORD	NumChannels;
		DWORD	SampleRate;
		DWORD	ByteRate;
		WORD	BlockAlign;
		WORD	BitsPerSample;
		DWORD	Subchunk2ID;
		DWORD	Subchunk2Size;

	}	*p_hdr = (wav_header*)hdr;
#pragma pack(pop)

	// Check the header
	if ( p_hdr->ChunkID != 'FFIR' )		goto finished;
	if ( p_hdr->Format != 'EVAW' )		goto finished;
	if ( p_hdr->Subchunk1ID != ' tmf' ) goto finished;
	if ( p_hdr->Subchunk1Size != 16 )	goto finished;
	if ( p_hdr->AudioFormat != 1 )		goto finished;
	if ( p_hdr->Subchunk2ID != 'atad' )	goto finished;
	
	// Get the number of channels
	const DWORD no_channels  = p_hdr->NumChannels;
	const DWORD sample_rate  = p_hdr->SampleRate;
	const DWORD sample_depth = p_hdr->BitsPerSample;

	// Get the number of samples
	const DWORD no_samples = p_hdr->Subchunk2Size / ( no_channels * (sample_depth/8) );

	// The total size of all the samples
	const DWORD sample_data_size = p_hdr->Subchunk2Size;

	// Allocate memory for the samples
	p_sample_data = new BYTE [ sample_data_size + 511 ];

	// Copy in the data that we start with
	const DWORD bytes_already_available = 512 - sizeof( wav_header );
	::memcpy( p_sample_data, &hdr[ sizeof( wav_header ) ], bytes_already_available );

	// Read the rest of the data
	const DWORD bytes_to_read     = sample_data_size - bytes_already_available;
	const DWORD bytes_to_read_512 =  ( bytes_to_read + 511 ) & (~511);
	bytes_read = 0;
	::ReadFile( hFile, p_sample_data + bytes_already_available, bytes_to_read_512, &bytes_read, NULL );

	// Check we read what we expected
	if ( bytes_read < bytes_to_read ) goto finished;	

	// Convert the sample types
	switch( sample_depth )
	{	case 8:
		{	// Allocate the buffer of the correct size
			m_data.resize( no_channels, no_samples );

			// Get the pointers
			float*		p_dst = m_data();
			const BYTE*	p_src = (BYTE*)p_sample_data;

			// The stride
			const int stride = m_data.stride();

			// The normalization
			const float inv_128 = 1.0f / 128.0f;

			// Special case on no channels
			switch( no_channels )
			{	case 1:		// Mono
							for( DWORD i=0; i<no_samples; i++, p_src+=1, p_dst++ )
								p_dst[ 0 ] = (float)( p_src[ 0 ] - 128 ) * inv_128;
							break;

				case 2:		// Stereo
							for( DWORD i=0; i<no_samples; i++, p_src+=2, p_dst++ )
								p_dst[ 0*stride ] = (float)( p_src[ 0 ] - 128 ) * inv_128,
								p_dst[ 1*stride ] = (float)( p_src[ 1 ] - 128 ) * inv_128;
							break;

				case 4:		// Quad
							for( DWORD i=0; i<no_samples; i++, p_src+=2, p_dst++ )
								p_dst[ 0*stride ] = (float)( p_src[ 0 ] - 128 ) * inv_128,
								p_dst[ 1*stride ] = (float)( p_src[ 1 ] - 128 ) * inv_128,
								p_dst[ 2*stride ] = (float)( p_src[ 2 ] - 128 ) * inv_128,
								p_dst[ 3*stride ] = (float)( p_src[ 3 ] - 128 ) * inv_128;
							break;

				default:	// Generic
							for( DWORD i=0; i<no_samples; i++, p_src+=no_channels, p_dst++ )
							for( DWORD j=0; j<no_channels; j++ )
								p_dst[ j*stride ] = (float)( p_src[ j ] - 128 ) * inv_128;
							break;
			}
		}

		case 16:
		{	// Allocate the buffer of the correct size
			m_data.resize( no_channels, no_samples );

			// Get the pointers
			float*		 p_dst = m_data();
			const short* p_src = (short*)p_sample_data;

			// The stride
			const int stride = m_data.stride();

			// The normalization
			const float inv_32768 = 1.0f / 32768.0f;

			// Special case on no channels
			switch( no_channels )
			{	case 1:		// Mono
							for( DWORD i=0; i<no_samples; i++, p_src+=1, p_dst++ )
								p_dst[ 0 ] = inv_32768*(float)p_src[ 0 ];
							break;

				case 2:		// Stereo
							for( DWORD i=0; i<no_samples; i++, p_src+=2, p_dst++ )
								p_dst[ 0*stride ] = inv_32768*(float)p_src[ 0 ],
								p_dst[ 1*stride ] = inv_32768*(float)p_src[ 1 ];
							break;

				case 4:		// Quad
							for( DWORD i=0; i<no_samples; i++, p_src+=2, p_dst++ )
								p_dst[ 0*stride ] = inv_32768*(float)p_src[ 0 ],
								p_dst[ 1*stride ] = inv_32768*(float)p_src[ 1 ],
								p_dst[ 2*stride ] = inv_32768*(float)p_src[ 2 ],
								p_dst[ 3*stride ] = inv_32768*(float)p_src[ 3 ];
							break;

				default:	// Generic
							for( DWORD i=0; i<no_samples; i++, p_src+=no_channels, p_dst++ )
							for( DWORD j=0; j<no_channels; j++ )
								p_dst[ j*stride ] = inv_32768*(float)p_src[ j ];
							break;
			}
		}

		default:	
		{	goto finished;
		}
	}

	// Close the file
finished:
	::CloseHandle( hFile );
	delete [] p_sample_data;
}

// Destructor
wav::~wav( void )
{
}