/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// 个人修改开始
#define LOG_TAG "AIDOCK_CAM_DECODER"
#include <utils/Log.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>

#include "Camera3H264Decoder.h"
#include "Camera3StreamInjectionManager.h"
#include <system/graphics.h>

namespace android {
namespace camera3 {

Camera3H264Decoder::Camera3H264Decoder() :
        mCodec(nullptr),
        mInitialized(false),
        mCurrentWidth(0),
        mCurrentHeight(0) { // 个人修改
}

Camera3H264Decoder::~Camera3H264Decoder() {
    release();
}

status_t Camera3H264Decoder::initialize(uint32_t width, uint32_t height) {
    if (mInitialized && mCurrentWidth == width && mCurrentHeight == height) return OK; // 个人修改

    mCodec = AMediaCodec_createDecoderByType("video/avc");
    if (!mCodec) {
        ALOGE("标记: 无法创建 H.264 解码器");
        return UNKNOWN_ERROR;
    }

    AMediaFormat* format = AMediaFormat_new();
    AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, width);
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, height);
    // COLOR_FormatYUV420SemiPlanar = 21 (NV12/NV21 depending on platform)
    AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, 21);

    media_status_t status = AMediaCodec_configure(mCodec, format, nullptr, nullptr, 0);
    AMediaFormat_delete(format);

    if (status != AMEDIA_OK) {
        ALOGE("标记: 解码器配置失败: %d", status);
        AMediaCodec_delete(mCodec);
        mCodec = nullptr;
        return UNKNOWN_ERROR;
    }

    status = AMediaCodec_start(mCodec);
    if (status != AMEDIA_OK) {
        ALOGE("标记: 解码器启动失败: %d", status);
        AMediaCodec_delete(mCodec);
        mCodec = nullptr;
        return UNKNOWN_ERROR;
    }

    mInitialized = true;
    mCurrentWidth = width;  // 个人修改
    mCurrentHeight = height; // 个人修改
    ALOGI("标记: H.264 硬件解码器已初始化并启动 (%ux%u)", width, height);
    return OK;
}

status_t Camera3H264Decoder::reconfigure(uint32_t width, uint32_t height) {
    ALOGI("标记: 正在重新配置解码器: %ux%u", width, height);
    release();
    return initialize(width, height);
}

void Camera3H264Decoder::release() {
    if (!mInitialized) return;

    if (mCodec) {
        AMediaCodec_stop(mCodec);
        AMediaCodec_delete(mCodec);
        mCodec = nullptr;
    }
    mInitialized = false;
    ALOGI("标记: H.264 硬件解码器已释放资源");
}

status_t Camera3H264Decoder::decode(uint8_t* data, size_t size) {
    if (!mInitialized) {
        ALOGE("标记: 解码器未初始化，拒绝解码请求");
        return INVALID_OPERATION;
    }

    // 1. 先尝试清理输出队列，释放输入缓冲区空间
    processOutput();

    // 2. 增加 dequeueInputBuffer 的等待时间，并尝试获取输入缓冲区
    ssize_t index = AMediaCodec_dequeueInputBuffer(mCodec, 5000); // 增加到 5ms
    
    // 如果队列依然满，再次尝试清理输出并重试一次
    if (index == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
        processOutput();
        index = AMediaCodec_dequeueInputBuffer(mCodec, 5000);
    }

    if (index >= 0) {
        size_t bufSize;
        uint8_t* buf = AMediaCodec_getInputBuffer(mCodec, index, &bufSize);
        if (buf && bufSize >= size) {
            memcpy(buf, data, size);
            AMediaCodec_queueInputBuffer(mCodec, index, 0, size, 0, 0);
        } else {
            ALOGE("标记: 输入缓冲区异常 (buf: %p, bufSize: %zu, dataSize: %zu)", buf, bufSize, size);
            AMediaCodec_queueInputBuffer(mCodec, index, 0, 0, 0, 0);
        }
    } else {
        ALOGW("标记: 解码器输入队列已满，当前帧已丢弃 (Result: %zd)", index);
    }

    // 3. 提交输入后再次尝试获取输出，提高实时性
    processOutput();
    return OK;
}

void Camera3H264Decoder::processOutput() {
    AMediaCodecBufferInfo info;
    ssize_t index;

    // 循环处理所有可用的输出缓冲区
    while (true) {
        index = AMediaCodec_dequeueOutputBuffer(mCodec, &info, 0);

        if (index == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
            break;
        } else if (index == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
            AMediaFormat* format = AMediaCodec_getOutputFormat(mCodec);
            ALOGI("标记: 解码格式已更改: %s", AMediaFormat_toString(format));
            AMediaFormat_delete(format);
            continue; // 继续 dequeue
        } else if (index < 0) {
            break;
        }

        // 正常处理解码后的数据 (index >= 0)
        if (info.size > 0) {
            uint8_t* outBuf = AMediaCodec_getOutputBuffer(mCodec, index, nullptr);
            if (outBuf) {
                AMediaFormat* format = AMediaCodec_getOutputFormat(mCodec);
                int32_t width, height, stride, sliceHeight;
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &width);
                AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &height);
                if (!AMediaFormat_getInt32(format, "stride", &stride)) stride = width;
                if (!AMediaFormat_getInt32(format, "slice-height", &sliceHeight)) sliceHeight = height;

                auto frame = std::make_shared<DecodedFrame>();
                frame->width = width;
                frame->height = height;
                frame->timestamp = info.presentationTimeUs * 1000;
                frame->format = HAL_PIXEL_FORMAT_YCrCb_420_SP; // NV21
                frame->data.resize(width * height * 3 / 2);
                
                uint8_t* dstY = frame->data.data();
                uint8_t* dstUV = dstY + width * height;
                uint8_t* srcY = outBuf + info.offset;
                uint8_t* srcUV = srcY + (stride * sliceHeight);
            
                // 1. 拷贝 Y 平面
                for (int i = 0; i < height; ++i) {
                    memcpy(dstY + i * width, srcY + i * stride, width);
                }
                // 2. 转换 NV12 (UV) 到 NV21 (VU)
                for (int i = 0; i < height / 2; ++i) {
                    uint8_t* sUV = srcUV + i * stride;
                    uint8_t* dUV = dstUV + i * width;
                    for (int j = 0; j < width; j += 2) {
                        dUV[j] = sUV[j+1]; // V
                        dUV[j+1] = sUV[j]; // U
                    }
                }
                Camera3StreamInjectionManager::getInstance()->updateFrame(frame);
                AMediaFormat_delete(format);
            }
        }
        AMediaCodec_releaseOutputBuffer(mCodec, index, false);
    }
}

} // namespace camera3
} // namespace android
// 个人修改结束

