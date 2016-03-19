//
// FILENAME : codec.h
// This file defines global constants
//

#ifndef	_CODEC_H
#define	_CODEC_H

#define	BITRATE		16	// available bps:16k,24k,32k,48k
						// 8kbps test version
//#define	BW14K		//BANDWIDTH 14000Hz
#undef	BW14K		//BANDWIDTH 7000Hz

typedef enum _CHANNEL {
	LEFT = 0,
	RIGHT = 1
} CHANNEL;

#define	RECEIVE
#endif	// _CODEC_H
// End of codec.h

