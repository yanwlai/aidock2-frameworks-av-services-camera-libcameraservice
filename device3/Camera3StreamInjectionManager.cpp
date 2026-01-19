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
#define LOG_TAG "AIDOCK_CAM_INJECT"
#include <utils/Log.h>
#include "Camera3StreamInjectionManager.h"

namespace android {
namespace camera3 {

Mutex Camera3StreamInjectionManager::sLock;
sp<Camera3StreamInjectionManager> Camera3StreamInjectionManager::sInstance = nullptr;

sp<Camera3StreamInjectionManager> Camera3StreamInjectionManager::getInstance() {
    AutoMutex lock(sLock);
    if (sInstance == nullptr) {
        sInstance = new Camera3StreamInjectionManager();
    }
    return sInstance;
}

Camera3StreamInjectionManager::Camera3StreamInjectionManager() :
        mIsInjectionActive(false),
        mTargetHeight(720) { // 个人修改
    ALOGI("个人修改: Camera3StreamInjectionManager 已初始化");
}

Camera3StreamInjectionManager::~Camera3StreamInjectionManager() {
}

void Camera3StreamInjectionManager::updateFrame(std::shared_ptr<DecodedFrame> frame) {
    AutoMutex lock(mFrameLock);
    mLatestFrame = frame;
    mIsInjectionActive = true;
}

std::shared_ptr<DecodedFrame> Camera3StreamInjectionManager::getLatestFrame() {
    AutoMutex lock(mFrameLock);
    return mLatestFrame;
}

void Camera3StreamInjectionManager::setInjectionActive(bool active) {
    AutoMutex lock(mFrameLock);
    mIsInjectionActive = active;
    ALOGI("标记: StreamInjectionManager 注入状态切换为: %s", active ? "激活" : "停止");
}

bool Camera3StreamInjectionManager::isInjectionActive() {
    AutoMutex lock(mFrameLock);
    return mIsInjectionActive;
}

// 个人修改开始
void Camera3StreamInjectionManager::setTargetHeight(uint32_t height) {
    AutoMutex lock(mFrameLock);
    mTargetHeight = height;
}

uint32_t Camera3StreamInjectionManager::getTargetHeight() {
    AutoMutex lock(mFrameLock);
    return mTargetHeight;
}
// 个人修改结束

} // namespace camera3
} // namespace android
// 个人修改结束
