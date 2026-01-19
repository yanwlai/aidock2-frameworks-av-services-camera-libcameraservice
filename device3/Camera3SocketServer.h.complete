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
#ifndef ANDROID_SERVERS_CAMERA_CAMERA3_SOCKET_SERVER_H
#define ANDROID_SERVERS_CAMERA_CAMERA3_SOCKET_SERVER_H

#include <utils/Thread.h>
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <vector>
#include <mutex>
#include <sys/un.h> // 为 Unix Domain Socket 添加头文件

namespace android {
namespace camera3 {

class Camera3H264Decoder;

/**
 * 一个简单的 Unix Domain Socket 服务器，用于接收 H.264 视频流。
 */
class Camera3SocketServer : public Thread {
public:
    Camera3SocketServer();
    virtual ~Camera3SocketServer();

    // 启动服务器
    status_t start();
    // 停止服务器
    void stop();

protected:
    virtual bool threadLoop() override;

private:
    int mServerSocket;
    int mClientSocket;
    bool mRunning;
    std::mutex mLock;

    // 个人修改开始
    // 抽象命名空间不需要路径，只需要一个名字
    static const char* kAbstractSocketName;
    // 个人修改结束

    sp<Camera3H264Decoder> mDecoder;
    uint32_t mCurrentWidth;
    uint32_t mCurrentHeight;

    void handleClient();
    bool parseAnnexB(uint8_t* buffer, size_t size);
    void detectResolutionChange(uint8_t* nalData, size_t size);
};

} // namespace camera3
} // namespace android

#endif // ANDROID_SERVERS_CAMERA_CAMERA3_SOCKET_SERVER_H
// 个人修改结束

