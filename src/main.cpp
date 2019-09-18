extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "vp.h"

struct CLIArgs {
	string filename;
};

CLIArgs *parse_args(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Error: not enough arguments" << endl;
		exit(0);
	}
	CLIArgs* result = new CLIArgs();
	for (int i = 0; i < argc - 1; ++i) {
	}
	result->filename = argv[argc - 1];
	return result;
}

uint64_t checksum = 0;

int main(int argc, char **argv) {
	av_register_all();
	AVCodecParameters *avcodec_params, *tmp_params;
	AVCodec *codec, *tmp_codec;
	CLIArgs *args = parse_args(argc, vector<string>(argv, argv + argc));

	// open file
	AVFormatContext *fctx = avformat_alloc_context();
	avformat_open_input(&fctx, args->filename.c_str(), NULL, NULL);
	cout << fctx->iformat->long_name << ", " << fctx->duration << endl;

	avformat_find_stream_info(fctx, NULL);
	cout << "Found " << fctx->nb_streams << " streams" << endl;

	vector<AVFrame*> frames;
	bool is_video;
	int video_stream = -1;

	// process each stream
	for (int i = 0; i < fctx->nb_streams; ++i) {
		tmp_params = fctx->streams[i]->codecpar;
		tmp_codec = avcodec_find_decoder(tmp_params->codec_id);
		if (tmp_codec == nullptr)
			continue;
		is_video = tmp_params->codec_type == AVMEDIA_TYPE_VIDEO;
		string ct =  is_video ? "video" : "not video";
		cout << "Codec(" << ct << "): " << tmp_codec->long_name << endl;
		if (is_video && video_stream == -1) {
			video_stream = i;
			avcodec_params = tmp_params;
			codec = tmp_codec;
		}
	}

	// process codec
	AVCodecContext *cctx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(cctx, avcodec_params);
	avcodec_open2(cctx, codec, NULL);
	cout << "height:" << cctx->height << ", width:" << cctx->width << endl;

	int size = cctx->width * cctx->height;
	int pitch = cctx->width;
	YUVData yuv_data(size, pitch);
	Display *disp = new Display(cctx->width, cctx->height, 1, "D");

	SwsContext *swsctx = sws_getContext(cctx->width,
			cctx->height, cctx->pix_fmt, cctx->width, cctx->height,
			AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

	// process each packet into a frame
	AVPacket *packet = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();

	double dur;
	double time_base = av_q2d(fctx->streams[video_stream]->time_base);
	while (av_read_frame(fctx, packet) >= 0) {
		if (packet->stream_index != video_stream)
			continue;
		if (avcodec_send_packet(cctx, packet) < 0)
			break;
		int res = 0;
		while (res >= 0) {
			res = avcodec_receive_frame(cctx, frame);
			if (res == AVERROR(EAGAIN) || res == AVERROR_EOF)
				break;
			//cout << "FRAME: " << cctx->frame_number << endl;
			dur = (1 + frame->repeat_pict) * time_base;

			AVPicture pic;
			pic.data[0] = yuv_data.y_plane;
			pic.data[1] = yuv_data.u_plane;
			pic.data[2] = yuv_data.v_plane;
			pic.linesize[0] = yuv_data.y_pitch;
			pic.linesize[1] = yuv_data.uv_pitch;
			pic.linesize[2] = yuv_data.uv_pitch;
			sws_scale(swsctx, (uint8_t const * const *)frame->data,
					frame->linesize, 0, cctx->height,
					pic.data, pic.linesize);

			disp->write(yuv_data);
			disp->display();
			if (!disp->wait(dur))
				exit(0);
		}
		av_packet_unref(packet);
	}
	disp->kill();
	return 0;
}
