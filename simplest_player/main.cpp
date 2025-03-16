/**
 * 最简单的基于FFmpeg的视频播放器 2
 * Simplest FFmpeg Player 2
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 第2版使用SDL2.0取代了第一版中的SDL1.2
 * Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.
 *
 * 本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest video player based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 */

#include <errno.h>
#include <stdio.h>

// 用于确保使用正确的常量定义
#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
//Windows平台特定代码
extern "C"
{
// FFmpeg库头文件
#include <libavcodec/packet.h>        // 用于处理压缩数据包
#include <libavdevice/avdevice.h>     // 用于处理设备相关功能
#include "libavcodec/avcodec.h"       // 编解码器相关
#include "libavformat/avformat.h"      // 格式相关
#include "libswscale/swscale.h"       // 图像转换相关
#include "libavutil/imgutils.h"        // 图像处理工具
#include "SDL2/SDL.h"                  // SDL2多媒体库
};
#else
//Linux平台特定代码
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

// 是否输出YUV420P格式的原始视频数据
#define OUTPUT_YUV420P 0

int wmain(int argc, wchar_t* argv[])
{
	// 检查命令行参数
	if (argc < 2) {
		printf("Usage: %ls <video_file>\n", argv[0]);
		return -1;
	}

	// 将Unicode路径转换为UTF-8编码
	char filepath[1024] = {0};
	size_t converted;
	wcstombs_s(&converted, filepath, sizeof(filepath), argv[1], _TRUNCATE);

	// 打印参数信息
	printf("参数个数: %d\n", argc);
	printf("argv[0]: [%ls]\n", argv[0]);
	printf("完整参数: [%ls]\n", argv[1]);
	printf("转换后的路径: [%s]\n", filepath);

	// FFmpeg变量声明
	AVFormatContext	*pFormatCtx;      // 格式上下文
	int				i, videoindex;    // 循环变量和视频流索引
	AVCodecContext	*pCodecCtx;       // 解码器上下文
	const AVCodec	*pCodec;          // 解码器
	AVFrame	*pFrame,*pFrameYUV;       // 原始帧和YUV帧
	unsigned char *out_buffer;         // 输出缓冲区
	AVPacket *packet;                 // 压缩数据包
	int y_size;                       // Y平面大小
	int ret, got_picture;             // 返回值和解码标志
	struct SwsContext *img_convert_ctx;// 图像转换上下文

	// SDL相关变量声明
	int screen_w=0,screen_h=0;        // 显示窗口尺寸
	SDL_Window *screen;               // SDL窗口
	SDL_Renderer* sdlRenderer;        // SDL渲染器
	SDL_Texture* sdlTexture;          // SDL纹理
	SDL_Rect sdlRect;                 // SDL矩形区域

	FILE *fp_yuv;                     // YUV输出文件指针

	// 初始化FFmpeg
	// av_register_all();            // 新版本FFmpeg不需要此函数
	avdevice_register_all();          // 注册所有设备
	avformat_network_init();          // 初始化网络库
	pFormatCtx = avformat_alloc_context();  // 分配格式上下文
	pCodecCtx = avcodec_alloc_context3(NULL); // 分配解码器上下文
	if (!pCodecCtx)
		return AVERROR(ENOMEM);

	// 打开输入文件
	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}

	// 获取流信息
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}

	// 查找视频流
	videoindex=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) 
		if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}

	// 将编解码器参数复制到解码器上下文
	ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);
	if (ret < 0) {
		return -1;
	}
	pCodecCtx->pkt_timebase = pFormatCtx->streams[videoindex]->time_base;

	// 查找解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}

	// 打开解码器
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}
	
	// 分配帧内存
	pFrame=av_frame_alloc();
	pFrameYUV=av_frame_alloc();
	out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,out_buffer,
		AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);
	
	// 分配Packet
	packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	// 输出视频信息
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");

	// 初始化图像转换上下文
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 

	// 如果需要，打开YUV输出文件
#if OUTPUT_YUV420P 
	fp_yuv=fopen("output.yuv","wb+");  
#endif  
	
	// 初始化SDL
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	// 设置SDL窗口
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,
		SDL_WINDOW_OPENGL);

	if(!screen) {  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}

	// 创建SDL渲染器和纹理
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);  

	// 设置显示区域
	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	// 主循环：读取视频帧并显示
	while(av_read_frame(pFormatCtx, packet)>=0){
		if(packet->stream_index==videoindex){
			// 发送数据包到解码器
			ret = avcodec_send_packet(pCodecCtx, packet);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			// 从解码器接收解码后的帧
			ret = avcodec_receive_frame(pCodecCtx, pFrame);
			if (ret < 0) {
				printf("receive frame error");
				return -1;
			}
			// 图像格式转换
			sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
				pFrameYUV->data, pFrameYUV->linesize);
				
			// 如果需要，保存YUV数据
#if OUTPUT_YUV420P
			y_size=pCodecCtx->width*pCodecCtx->height;  
			fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y 
			fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
			fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
#endif

			// 更新SDL纹理
			SDL_UpdateYUVTexture(sdlTexture, &sdlRect,
			pFrameYUV->data[0], pFrameYUV->linesize[0],
			pFrameYUV->data[1], pFrameYUV->linesize[1],
			pFrameYUV->data[2], pFrameYUV->linesize[2]);
				
			// 渲染显示
			SDL_RenderClear( sdlRenderer );  
			SDL_RenderCopy( sdlRenderer, sdlTexture,  NULL, &sdlRect);  
			SDL_RenderPresent( sdlRenderer );  

			// 控制播放速度
			SDL_Delay(40);
		}
		// 释放packet
		av_packet_unref(packet);
	}

	// 清理资源
	sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P 
	fclose(fp_yuv);
#endif 

	SDL_Quit();
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}