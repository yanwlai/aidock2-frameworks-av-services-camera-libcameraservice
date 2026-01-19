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

#ifndef ANDROID_SERVERS_CAMERA3_STREAM_INJECTION_MANAGER_H
#define ANDROID_SERVERS_CAMERA3_STREAM_INJECTION_MANAGER_H

// 个人修改开始
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Timers.h>
#include <vector>
#include <memory>

namespace android {
namespace camera3 {

struct DecodedFrame {
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> data;
    nsecs_t timestamp;
    int format; // HAL_PIXEL_FORMAT_...

    DecodedFrame() : width(0), height(0), timestamp(0), format(0) {}
};

class Camera3StreamInjectionManager : public virtual RefBase {
public:
    static sp<Camera3StreamInjectionManager> getInstance();

    void updateFrame(std::shared_ptr<DecodedFrame> frame);
    std::shared_ptr<DecodedFrame> getLatestFrame();
    
    void setInjectionActive(bool active);
    bool isInjectionActive();

    // 个人修改开始
    void setTargetHeight(uint32_t height);
    uint32_t getTargetHeight();
    // 个人修改结束

private:
    Camera3StreamInjectionManager();
    virtual ~Camera3StreamInjectionManager();

    static Mutex sLock;
    static sp<Camera3StreamInjectionManager> sInstance;

    Mutex mFrameLock;
    std::shared_ptr<DecodedFrame> mLatestFrame;
    bool mIsInjectionActive;
    
    // 个人修改开始
    uint32_t mTargetHeight;
    // 个人修改结束
};

} // namespace camera3
} // namespace android
// 个人修改结束

#endif
