#include <fingerprint_strategy.hh>
#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <chromaprint.h>
}


#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

namespace {

    size_t frameLength = 4096;

    size_t advance = 1365;

    size_t sampleRate  = 11025;

    size_t BUFFER_SIZE = (AVCODEC_MAX_AUDIO_FRAME_SIZE * 2);

    int decode_audio_file(ChromaprintContext *chromaprint_ctx, int16_t *buffer1, int16_t *buffer2, const char *file_name, int max_length, int *duration) {

        int ok = 0, remaining, length, consumed, buffer_size, codec_ctx_opened = 0;
        AVFormatContext *format_ctx = NULL;
        AVCodecContext *codec_ctx = NULL;
        AVCodec *codec = NULL;
        AVStream *stream = NULL;
        AVPacket packet, packet_temp;
#ifdef HAVE_AV_AUDIO_CONVERT
        AVAudioConvert *convert_ctx = NULL;
#endif
        int16_t *buffer;

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
        if (av_open_input_file(&format_ctx, file_name, NULL, 0, NULL) != 0) {
#else
        if (avformat_open_input(&format_ctx, file_name, NULL, NULL) != 0) {
#endif
            fprintf(stderr, "ERROR: couldn't open the file\n");
            goto done;
        }

        if (av_find_stream_info(format_ctx) < 0) {
            fprintf(stderr, "ERROR: couldn't find stream information in the file\n");
            goto done;
        }

        for (size_t i = 0; i < format_ctx->nb_streams; i++) {
            codec_ctx = format_ctx->streams[i]->codec;
            if (codec_ctx && codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
                stream = format_ctx->streams[i];
                break;
            }
        }
        if (!stream) {
            fprintf(stderr, "ERROR: couldn't find any audio stream in the file\n");
            goto done;
        }

        codec = avcodec_find_decoder(codec_ctx->codec_id);
        if (!codec) {
            fprintf(stderr, "ERROR: unknown codec\n");
            goto done;
        }

        if (avcodec_open(codec_ctx, codec) < 0) {
            fprintf(stderr, "ERROR: couldn't open the codec\n");
            goto done;
        }
        codec_ctx_opened = 1;

        if (codec_ctx->channels <= 0) {
            fprintf(stderr, "ERROR: no channels found in the audio stream\n");
            goto done;
        }

        if (codec_ctx->sample_fmt != AV_SAMPLE_FMT_S16) {
#ifdef HAVE_AV_AUDIO_CONVERT
            convert_ctx = av_audio_convert_alloc(AV_SAMPLE_FMT_S16, codec_ctx->channels,
                                                 codec_ctx->sample_fmt, codec_ctx->channels, NULL, 0);
            if (!convert_ctx) {
                fprintf(stderr, "ERROR: couldn't create sample format converter\n");
                goto done;
            }
#else
            fprintf(stderr, "ERROR: unsupported sample format\n");
            goto done;
#endif
        }

        *duration = stream->time_base.num * stream->duration / stream->time_base.den;
        max_length = *duration;

        av_init_packet(&packet);
        av_init_packet(&packet_temp);

        remaining = max_length * codec_ctx->channels * codec_ctx->sample_rate;
        chromaprint_start(chromaprint_ctx, codec_ctx->sample_rate, codec_ctx->channels);

        while (1) {
            if (av_read_frame(format_ctx, &packet) < 0) {
                break;
            }

            packet_temp.data = packet.data;
            packet_temp.size = packet.size;

            while (packet_temp.size > 0) {
                buffer_size = BUFFER_SIZE;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(52, 23, 0)
                consumed = avcodec_decode_audio2(codec_ctx,
                                                 buffer1, &buffer_size, packet_temp.data, packet_temp.size);
#else
                consumed = avcodec_decode_audio3(codec_ctx,
                                                 buffer1, &buffer_size, &packet_temp);
#endif

                if (consumed < 0) {
                    break;
                }

                packet_temp.data += consumed;
                packet_temp.size -= consumed;

                if (buffer_size <= 0) {
                    if (buffer_size < 0) {
                        fprintf(stderr, "WARNING: size returned from avcodec_decode_audioX is too small\n");
                    }
                    continue;
                }
                if ((size_t)buffer_size > BUFFER_SIZE) {
                    fprintf(stderr, "WARNING: size returned from avcodec_decode_audioX is too large\n");
                    continue;
                }

#ifdef HAVE_AV_AUDIO_CONVERT
                if (convert_ctx) {
                    const void *ibuf[6] = { buffer1 };
                    void *obuf[6] = { buffer2 };
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(51, 8, 0)
                    int istride[6] = { av_get_bits_per_sample_format(codec_ctx->sample_fmt) / 8 };
#else
                    int istride[6] = { av_get_bytes_per_sample(codec_ctx->sample_fmt) };
#endif
                    int ostride[6] = { 2 };
                    int len = buffer_size / istride[0];
                    if (av_audio_convert(convert_ctx, obuf, ostride, ibuf, istride, len) < 0) {
                        break;
                    }
                    buffer = buffer2;
                    buffer_size = len * ostride[0];
                }
                else {
                    buffer = buffer1;
                }
#else
                buffer = buffer1;
#endif

                length = MIN(remaining, buffer_size / 2);
                if (!chromaprint_feed(chromaprint_ctx, buffer, length)) {
                    fprintf(stderr, "ERROR: fingerprint calculation failed\n");
                    goto done;
                }

                if (max_length) {
                    remaining -= length;
                    if (remaining <= 0) {
                        goto finish;
                    }
                }
            }

            if (packet.data) {
                av_free_packet(&packet);
            }
        }

        finish:
        if (!chromaprint_finish(chromaprint_ctx)) {
            fprintf(stderr, "ERROR: fingerprint calculation failed\n");
            goto done;
        }

        ok = 1;

        done:
        if (codec_ctx_opened) {
            avcodec_close(codec_ctx);
        }
        if (format_ctx) {
            av_close_input_file(format_ctx);
        }
#ifdef HAVE_AV_AUDIO_CONVERT
        if (convert_ctx) {
            av_audio_convert_free(convert_ctx);
        }
#endif
        return ok;

    }


    int fpcalc_main(std::string filename, std::vector<uint32_t> &output) {

        int max_length = 120, raw_fingerprint_size, duration;
        
        int16_t *buffer1, *buffer2;
        int32_t *raw_fingerprint;
        const char *file_name = filename.c_str();
        ChromaprintContext *chromaprint_ctx;
        int algo = CHROMAPRINT_ALGORITHM_DEFAULT;
        
        av_register_all();
        av_log_set_level(AV_LOG_ERROR);
        
        buffer1 = (int16_t*)av_malloc(BUFFER_SIZE + 16);
        buffer2 = (int16_t*)av_malloc(BUFFER_SIZE + 16);
        chromaprint_ctx = chromaprint_new(algo);

        if (!decode_audio_file(chromaprint_ctx, buffer1, buffer2, file_name, max_length, &duration)) {
            fprintf(stderr, "ERROR: unable to calculate fingerprint for file %s, skipping\n", file_name);
            return 1;
        }

        printf("DURATION=%d\n", duration);

        if (!chromaprint_get_raw_fingerprint(chromaprint_ctx, (void **)&raw_fingerprint, &raw_fingerprint_size)) {
            fprintf(stderr, "ERROR: unable to calculate fingerprint for file %s, skipping\n", file_name);
            return 1;
        }
        //printf("FINGERPRINT=");
        //printf("\n%d", raw_fingerprint_size);
        for (size_t j = 0; j < raw_fingerprint_size; j++) {
            //printf("%d%s", raw_fingerprint[j], j + 1 < (size_t)raw_fingerprint_size ? "," : "");
            output.push_back((uint32_t)raw_fingerprint[j]);
        }
        //printf("\n");
        chromaprint_dealloc(raw_fingerprint);

        chromaprint_free(chromaprint_ctx);
        av_free(buffer1);
        av_free(buffer2);

        return 0;

    }

}

namespace sound_index {


    void
    fingerprint_strategy_chroma::getFingerprintsForFile(std::string filename, std::vector<uint32_t> &res) {
        // implement chroma.
        ::fpcalc_main(filename, res);        
    }
        
    size_t
    fingerprint_strategy_chroma::getFrameLength() { return ::frameLength; }

    size_t
    fingerprint_strategy_chroma::getAdvance() { return ::advance; }

    size_t
    fingerprint_strategy_chroma::getSampleRate() { return ::sampleRate; }

}
