#pragma once

// Standard includes
#include <vector>
#include <memory>
#include <thread>

// Project includes
#include "Devices/DeviceBase.h"
#include "Games/GameBase.h"
#include "LightLocationMask.h"
#include "Config.h"
#include "TrayIcon.h"
#include "UpdateManager.h"


namespace lightfx {

    class LightsManager {

    public:
        static LightsManager& Instance() {
            static LightsManager instance;
            return instance;
        }

        bool IsInitialized();
        bool HasDevices();

        size_t Initialize();
        size_t Release();
        size_t Update();
        size_t Reset();

        size_t GetNumberOfDevices();
        std::shared_ptr<devices::DeviceBase> GetDevice(const size_t index);

        size_t SetColor(const LFX_COLOR& color);
        size_t SetColorForLocation(const LightLocationMask locationMask, const LFX_COLOR& color);

        size_t MorphTo(const LFX_COLOR& color, const unsigned int transitionTime);
        size_t MorphToForLocation(const LightLocationMask locationMask, const LFX_COLOR& color, const unsigned int transitionTime);

        size_t Pulse(const LFX_COLOR& color, const unsigned int transitionTime, const unsigned int amount);
        size_t Pulse(const LFX_COLOR& color, const unsigned int transitionTime, const unsigned int startColorTime, const unsigned int endColorTime, const unsigned int amount);
        size_t Pulse(const LFX_COLOR& startColor, const LFX_COLOR& endColor, const unsigned int transitionTime, const unsigned int amount);
        size_t Pulse(const LFX_COLOR& startColor, const LFX_COLOR& endColor, const unsigned int transitionTime, const unsigned int startColorTime, const unsigned int endColorTime, const unsigned int amount);
        size_t PulseForLocation(const LightLocationMask locationMask, const LFX_COLOR& color, const unsigned int transitionTime, const unsigned int amount);
        size_t PulseForLocation(const LightLocationMask locationMask, const LFX_COLOR& color, const unsigned int transitionTime, const unsigned int startColorTime, const unsigned int endColorTime, const unsigned int amount);
        size_t PulseForLocation(const LightLocationMask locationMask, const LFX_COLOR& startColor, const LFX_COLOR& endColor, const unsigned int transitionTime, const unsigned int amount);
        size_t PulseForLocation(const LightLocationMask locationMask, const LFX_COLOR& startColor, const LFX_COLOR& endColor, const unsigned int transitionTime, const unsigned int startColorTime, const unsigned int endColorTime, const unsigned int amount);
        
        void CheckForUpdate();
    
    private:
        LightsManager() {};
        LightsManager(const LightsManager&);
        void operator =(const LightsManager&);

        std::vector<std::shared_ptr<devices::DeviceBase>> devices;
        std::vector<std::shared_ptr<games::GameBase>> games;
        std::shared_ptr<games::GameBase> currentGame;
        bool isInitialized = false;

        Config config;
        TrayIcon trayIcon;
        std::thread updateCheckerThread;
        UpdateManager updateManager;

        void AddDevices();
        void AddGames();

    };

}
