# ifndef LIBMAD_STREAM_H
# define LIBMAD_STREAM_H

# include "bit.h"

# define MAD_BUFFER_GUARD	8
# define MAD_BUFFER_MDLEN	(511 + 2048 + MAD_BUFFER_GUARD)

enum mad_error {
  MAD_ERROR_NONE	   = 0x0000,	/* no error */

  MAD_ERROR_BUFLEN	   = 0x0001,	/* input buffer too small (or EOF) */
  MAD_ERROR_BUFPTR	   = 0x0002,	/* invalid (null) buffer pointer */

  MAD_ERROR_NOMEM	   = 0x0031,	/* not enough memory */

  MAD_ERROR_LOSTSYNC	   = 0x0101,	/* lost synchronization */
  MAD_ERROR_BADLAYER	   = 0x0102,	/* reserved header layer value */
  MAD_ERROR_BADBITRATE	   = 0x0103,	/* forbidden bitrate value */
  MAD_ERROR_BADSAMPLERATE  = 0x0104,	/* reserved sample frequency value */
  MAD_ERROR_BADEMPHASIS	   = 0x0105,	/* reserved emphasis value */

  MAD_ERROR_BADCRC	   = 0x0201,	/* CRC check failed */
  MAD_ERROR_BADBITALLOC	   = 0x0211,	/* forbidden bit allocation value */
  MAD_ERROR_BADSCALEFACTOR = 0x0221,	/* bad scalefactor index */
  MAD_ERROR_BADMODE        = 0x0222,	/* bad bitrate/mode combination */
  MAD_ERROR_BADFRAMELEN	   = 0x0231,	/* bad frame length */
  MAD_ERROR_BADBIGVALUES   = 0x0232,	/* bad big_values count */
  MAD_ERROR_BADBLOCKTYPE   = 0x0233,	/* reserved block_type */
  MAD_ERROR_BADSCFSI	   = 0x0234,	/* bad scalefactor selection info */
  MAD_ERROR_BADDATAPTR	   = 0x0235,	/* bad main_data_begin pointer */
  MAD_ERROR_BADPART3LEN	   = 0x0236,	/* bad audio data length */
  MAD_ERROR_BADHUFFTABLE   = 0x0237,	/* bad Huffman table select */
  MAD_ERROR_BADHUFFDATA	   = 0x0238,	/* Huffman data overrun */
  MAD_ERROR_BADSTEREO	   = 0x0239	/* incompatible block_type for JS */
};

# define MAD_RECOVERABLE(error)	((error) & 0xff00)

struct mad_stream {
  unsigned char const *buffer;		/* input bitstream buffer */
  unsigned char const *bufend;		/* end of buffer */
  unsigned short skiplen;			/* bytes to skip before next frame */

  short sync;							/* stream sync found */
  unsigned short freerate;			/* free bitrate (fixed,kbps) */

  unsigned char const *this_frame;	/* start of current frame */
  unsigned char const *next_frame;	/* start of next frame */
  struct mad_bitptr ptr;			/* current processing bit pointer */

  struct mad_bitptr anc_ptr;		/* ancillary bits pointer */
  unsigned short anc_bitlen;			/* number of ancillary bits */

  unsigned char main_data[MAD_BUFFER_MDLEN];
					/* Layer III main_data() */
  unsigned short md_len;			/* bytes in main_data */

  enum mad_error error;			/* error code (see above) */
};

enum {
  MAD_OPTION_IGNORECRC      = 0x0001,	/* ignore CRC errors */
  MAD_OPTION_HALFSAMPLERATE = 0x0002	/* generate PCM at 1/2 sample rate */
};

void mad_stream_init(struct mad_stream *);
void mad_stream_finish(struct mad_stream *);


void mad_stream_buffer(struct mad_stream *,
		       unsigned char const *, unsigned short);

short mad_stream_sync(struct mad_stream *);

# endif
