//
// Created by 海建明 on 2018/11/22.
//

#include <jni.h>
#include "../utilbase/utilbase.h"
#include "../include/libavutil/error.h"

extern "C" {
#include "ffmpeg.h"
}

__attribute__((section (".mytext"))) static jstring
ffmpegCMD(JNIEnv *env, jclass obj, jstring commands) {
    LOGI("start ffmpeg");
    const char* configure = avcodec_configuration();

    av_register_all();
    avcodec_register_all();
    char path[] = "/sdcard/DCIM/Camera/1.mp4";
    //char path[] = "/sdcard/video.flv";
    //打开文件
    AVFormatContext *ic = NULL;

    int re = avformat_open_input(&ic, path, 0, 0);
    if (re != 0) {
//        LOGW("avformat_open_input failed!:%s", av_err2str(re));
        return env->NewStringUTF(configure);
    }
    //获取流信息
    re = avformat_find_stream_info(ic, 0);
    if (re != 0) {
        LOGW("avformat_find_stream_info failed!");
    }
    LOGW("duration = %lld nb_streams = %d", ic->duration, ic->nb_streams);

    int fps = 0;
    int videoStream = 0;
    int audioStream = 1;

    for (int i = 0; i < ic->nb_streams; i++) {
        AVStream *as = ic->streams[i];
        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGW("视频数据");
            videoStream = i;

            LOGW("width=%d height=%d codeid=%d pixformat=%d",
                 as->codecpar->width,
                 as->codecpar->height,
                 as->codecpar->codec_id,
                 as->codecpar->format
            );

        } else if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGW("音频数据");
            audioStream = i;
            LOGW("sample_rate=%d channels=%d sample_format=%d",
                 as->codecpar->sample_rate,
                 as->codecpar->channels,
                 as->codecpar->format
            );
        }
    }
    //ic->streams[videoStream];
    //获取音频流信息
    audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    LOGW("av_find_best_stream audioStream = %d", audioStream);
    //////////////////////////////////////////////////////////
    //打开视频解码器
    //软解码器
    AVCodec *codec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);


    if (!codec) {
        LOGW("avcodec_find HW failed!");
        return env->NewStringUTF(configure);

    }
    //解码器初始化
    AVCodecContext *vc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);
    LOGW("w,h = %d,%d", vc->width, vc->height);

    vc->thread_count = 2;

    //打开解码器
    re = avcodec_open2(vc, codec, 0);
    //vc->time_base = ic->streams[videoStream]->time_base;
    LOGW("vc timebase = %d/ %d", vc->time_base.num, vc->time_base.den);
    if (re != 0) {
        LOGW("avcodec_open2 video failed! %d ", re);
        return env->NewStringUTF(configure);

    }
    LOGW("**********");
    return env->NewStringUTF(configure);
}
__attribute__((section (".mytext"))) static jint
ffmpegInit(JNIEnv *env, jclass obj, jint code){
    LOGI("test log:%d",code);

    return code+4;
}
static JNINativeMethod methods[] = {
        {"ffmpegCMD", "(Ljava/lang/String;)Ljava/lang/String;", (void *) ffmpegCMD},
        {"ffmpegInit","(I)I",(void *)ffmpegInit}

};

jint registerNativeMethods(JNIEnv *env, const char *class_name, JNINativeMethod *methods,
                           int num_methods) {
    int result = 0;

    jclass clazz = env->FindClass(class_name);
    if (LIKELY(clazz)) {
        int result = env->RegisterNatives(clazz, methods, num_methods);
        if (UNLIKELY(result < 0)) {
            LOGE("registerNativeMethods failed(class=%s)", class_name);
        }
    } else {
        LOGE("registerNativeMethods: class'%s' not found", class_name);
    }
    return result;
}

int register_method(JNIEnv *env) {
    if (registerNativeMethods(env,
                              "com/llvision/streamdemo/ffmpeg/FFmpegUtils",
                              methods, NUM_ARRAY_ELEMENTS(methods)) < 0) {
        return -1;
    }
    return 0;
}