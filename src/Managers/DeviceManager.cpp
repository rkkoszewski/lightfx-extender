#ifndef LFXE_EXPORTS
#define LFXE_EXPORTS
#endif

#include "DeviceManager.h"

// Project includes
#include "../LightFXExtender.h"
#include "ConfigManager.h"
#include "../Config/MainConfigFile.h"
#include "../Devices/DeviceCorsair.h"
#include "../Devices/DeviceRazer.h"
#include "../Utils/FileIO.h"
#include "../Utils/Log.h"
#include "../Utils/String.h"


using namespace std;
using namespace lightfx::config;
using namespace lightfx::devices;
using namespace lightfx::devices::proxies;
using namespace lightfx::utils;

namespace lightfx {
    namespace managers {

        LFXE_API size_t DeviceManager::InitializeDevices() {
            LOG_DEBUG(L"Initializing devices");
            size_t i = 0;

            auto config = this->GetLightFXExtender()->GetConfigManager()->GetMainConfig();
            this->updateDevicesInterval = config->TimelineUpdateInterval;

            auto corsair = make_shared<DeviceCorsair>();
            this->AddChild(L"Corsair", corsair);
            if (corsair->Initialize()) {
                ++i;
            }
            
            auto razer = make_shared<DeviceRazer>();
            razer->SetHardware(config->RazerUseWithKeyboard, config->RazerUseWithMouse, config->RazerUseWithHeadset, config->RazerUseWithMousepad, config->RazerUseWithKeypad);
            this->AddChild(L"Razer", razer);
            if (razer->Initialize()) {
                ++i;
            }

            LOG_INFO(L"Successfully initialized " + to_wstring(i) + L" devices");

            // Enable devices where needed
            if (config->AutoDeviceDetection)
            {
                for (size_t i = 0; i < this->GetChildrenCount(); ++i) {
                    auto device = this->GetChildByIndex(i);

                    if (device == nullptr) {
                        LOG_WARNING(L"Device " + device->GetDeviceName() + L" is configured in settings, but was not found in the system");
                        continue;
                    }
                    if (!device->IsInitialized()) {
                        LOG_WARNING(L"Device " + device->GetDeviceName() + L" cannot be enabled, because was not initialized");
                        continue;
                    }

                    bool auto_result = device->Enable();
                    LOG_WARNING(L"Device " + device->GetDeviceName() + L" was automatically set to " + (auto_result ? L"ON" : L"OFF"));
                }
            }
            else
            {
                for (pair<wstring, bool> device : config->EnabledDevices) {
                    if (device.second) {
                        auto dev = this->GetChild(device.first);
                        if (dev == nullptr) {
                            LOG_WARNING(L"Device " + device.first + L" is configured in settings, but was not found in the system");
                            continue;
                        }
                        if (!dev->IsInitialized()) {
                            LOG_WARNING(L"Device " + device.first + L" cannot be enabled, because was not initialized");
                            continue;
                        }
                        dev->Enable();
                    }
                }
            }
            return i;
        }

        LFXE_API size_t DeviceManager::UninitializeDevices() {
            LOG_DEBUG(L"Uninitializing devices");
            size_t i = 0;

            // Unload devices
            for (size_t j = 0; j < this->GetChildrenCount(); ++j) {
                auto device = this->GetChildByIndex(j);
                if (device->IsEnabled() && device->Release()) {
                    ++i;
                }
            }

            LOG_INFO(L"Successfully uninitialized " + to_wstring(i) + L" devices");
            return i;
        }
   
        LFXE_API void DeviceManager::StartUpdateDevicesWorker() {
            if (!this->updateDevicesWorkerActive) {
                this->updateDevicesWorkerActive = true;
                this->stopUpdateDevicesWorker = false;
                this->updateDevicesWorkerThread = thread(&DeviceManager::UpdateDevicesWorker, this);
            }
        }

        LFXE_API void DeviceManager::StopUpdateDevicesWorker() {
            if (this->updateDevicesWorkerActive) {
                this->stopUpdateDevicesWorker = true;
                this->updateDevicesNotifyEvent.Notify();
                if (this->updateDevicesWorkerThread.joinable()) {
                    this->updateDevicesWorkerThread.join();
                }
                this->updateDevicesWorkerActive = false;
            }
        }

        LFXE_API void DeviceManager::UpdateDeviceLights(const bool flushQueue) {
            if (this->updateDevicesWorkerActive) {
                this->flushQueue = flushQueue;
                this->updateDevicesNotifyEvent.Notify();
            }
        }

        LFXE_API void DeviceManager::UpdateDevicesWorker() {
            bool isUpdating = false;
            chrono::milliseconds timeTick;

            while (!this->stopUpdateDevicesWorker) {
                bool flushQueue = false;
                if (isUpdating && !this->updateDevicesNotifyEvent.IsNotified()) {
                    // Still updating from previous iterations without new updates coming in, sleep for a while to prevent unneeded CPU usage
                    this_thread::sleep_for(chrono::milliseconds(this->updateDevicesInterval));
                } else {
                    // Wait for when we get signaled to update or stop
                    this->updateDevicesNotifyEvent.Wait();

                    // Reset certain variables since we should have a new timeline here
                    flushQueue = this->flushQueue;
                    this->flushQueue = false;
                    isUpdating = false;
                }

                // Stop worker if it has been signaled to stop
                if (this->stopUpdateDevicesWorker) {
                    break;
                }

                // Update every device
                bool done = true;
                timeTick = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
                for (size_t i = 0; i < this->GetChildrenCount(); ++i) {
                    if (!isUpdating) {
                        // This update is done for the first time, commit queued timeline and optionally flush the queue
                        this->GetChildByIndex(i)->CommitQueuedTimeline(flushQueue);
                    }
                    done &= this->GetChildByIndex(i)->Update(timeTick);
                }

                isUpdating = !done;
            }
        }

    }
}
