#ifndef LFXE_EXPORTS
#define LFXE_EXPORTS
#endif

#include "MainConfigFile.h"


#pragma region Configuration keys
#define CONF_ENABLEDDEVICES L"EnabledDevices"

#define CONF_LOGITECHCOLORRANGE L"LogitechColorRange"
#define CONF_LOGITECHCOLORRANGE_OUTMIN L"AlienFXMin"
#define CONF_LOGITECHCOLORRANGE_OUTMAX L"AlienFXMax"
#define CONF_LOGITECHCOLORRANGE_INMIN L"LogitechMin"
#define CONF_LOGITECHCOLORRANGE_INMAX L"LogitechMax"

#define CONF_LIGHTPACKAPI L"LightpackAPI"
#define CONF_LIGHTPACKHOST L"Hostname"
#define CONF_LIGHTPACKPORT L"PortNumber"
#define CONF_LIGHTPACKKEY L"Key"
#pragma endregion

using namespace std;
using namespace rapidjson;
using namespace lightfx::managers;

namespace lightfx {
    namespace config {

        LFXE_API void MainConfigFile::Load() {
            ConfigFile::Load(L"settings.json");
        }

        LFXE_API void MainConfigFile::LoadDefaults() {
            this->LogitechColorRangeOutMin = 0;
            this->LogitechColorRangeOutMax = 255;
            this->LogitechColorRangeInMin = 0;
            this->LogitechColorRangeInMax = 100;

            this->LightpackHost = L"127.0.0.1";
            this->LightpackPort = L"3636";
            this->LightpackKey = L"";
        }

        LFXE_API wstring MainConfigFile::Serialize() {
            WDocument::AllocatorType& allocator = this->doc.GetAllocator();
            this->doc.SetObject();

            // Enabled devices
            WValue enabledDevices(kObjectType);
            for (auto itr : this->EnabledDevices) {
                enabledDevices.AddMember(WValue(itr.first.c_str(), allocator).Move(), WValue(itr.second), allocator);
            }
            this->doc.AddMember(CONF_ENABLEDDEVICES, enabledDevices, allocator);

            // Logitech color range
            WValue logitechColorRange(kObjectType);
            logitechColorRange.AddMember(CONF_LOGITECHCOLORRANGE_OUTMIN, this->LogitechColorRangeOutMin, allocator);
            logitechColorRange.AddMember(CONF_LOGITECHCOLORRANGE_OUTMAX, this->LogitechColorRangeOutMax, allocator);
            logitechColorRange.AddMember(CONF_LOGITECHCOLORRANGE_INMIN, this->LogitechColorRangeInMin, allocator);
            logitechColorRange.AddMember(CONF_LOGITECHCOLORRANGE_INMAX, this->LogitechColorRangeInMax, allocator);
            this->doc.AddMember(CONF_LOGITECHCOLORRANGE, logitechColorRange, allocator);

            // Lightpack API
            WValue lightpackApi(kObjectType);
            lightpackApi.AddMember(CONF_LIGHTPACKHOST, this->MakeJsonWString(this->LightpackHost, allocator), allocator);
            lightpackApi.AddMember(CONF_LIGHTPACKPORT, this->MakeJsonWString(this->LightpackPort, allocator), allocator);
            lightpackApi.AddMember(CONF_LIGHTPACKKEY, this->MakeJsonWString(this->LightpackKey, allocator), allocator);
            this->doc.AddMember(CONF_LIGHTPACKAPI, lightpackApi, allocator);

            WStringBuffer buffer;
            WPrettyWriter writer(buffer);
            this->doc.Accept(writer);
            return buffer.GetString();
        }

        LFXE_API void MainConfigFile::Deserialize(const wstring& data) {
            this->doc.Parse<0>(data.c_str());

            if (!this->doc.IsObject()) {
                return;
            }

            // Enabled devices
            if (this->doc.HasMember(CONF_ENABLEDDEVICES) && this->doc[CONF_ENABLEDDEVICES].IsObject()) {
                const WValue& enabledDevices = this->doc[CONF_ENABLEDDEVICES];
                for (auto itr = enabledDevices.MemberBegin(); itr != enabledDevices.MemberEnd(); ++itr) {
                    if (itr->value.IsBool()) {
                        wstring deviceName = itr->name.GetString();
                        bool deviceEnabled = itr->value.GetBool();
                        this->EnabledDevices[deviceName] = deviceEnabled;
                    }
                }
            }

            // Logitech color range
            if (this->doc.HasMember(CONF_LOGITECHCOLORRANGE) && this->doc[CONF_LOGITECHCOLORRANGE].IsObject()) {
                const WValue& colorRange = this->doc[CONF_LOGITECHCOLORRANGE];
                if (colorRange.HasMember(CONF_LOGITECHCOLORRANGE_OUTMIN) && colorRange[CONF_LOGITECHCOLORRANGE_OUTMIN].IsInt()) {
                    this->LogitechColorRangeOutMin = colorRange[CONF_LOGITECHCOLORRANGE_OUTMIN].GetInt();
                }
                if (colorRange.HasMember(CONF_LOGITECHCOLORRANGE_OUTMAX) && colorRange[CONF_LOGITECHCOLORRANGE_OUTMAX].IsInt()) {
                    this->LogitechColorRangeOutMax = colorRange[CONF_LOGITECHCOLORRANGE_OUTMAX].GetInt();
                }
                if (colorRange.HasMember(CONF_LOGITECHCOLORRANGE_INMIN) && colorRange[CONF_LOGITECHCOLORRANGE_INMIN].IsInt()) {
                    this->LogitechColorRangeInMin = colorRange[CONF_LOGITECHCOLORRANGE_INMIN].GetInt();
                }
                if (colorRange.HasMember(CONF_LOGITECHCOLORRANGE_INMAX) && colorRange[CONF_LOGITECHCOLORRANGE_INMAX].IsInt()) {
                    this->LogitechColorRangeInMax = colorRange[CONF_LOGITECHCOLORRANGE_INMAX].GetInt();
                }
            }

            // Lightpack API
            if (this->doc.HasMember(CONF_LIGHTPACKAPI) && this->doc[CONF_LIGHTPACKAPI].IsObject()) {
                const WValue& api = this->doc[CONF_LIGHTPACKAPI];
                if (api.HasMember(CONF_LIGHTPACKHOST) && api[CONF_LIGHTPACKHOST].IsString()) {
                    this->LightpackHost = api[CONF_LIGHTPACKHOST].GetString();
                }
                if (api.HasMember(CONF_LIGHTPACKPORT) && api[CONF_LIGHTPACKPORT].IsString()) {
                    this->LightpackPort = api[CONF_LIGHTPACKPORT].GetString();
                }
                if (api.HasMember(CONF_LIGHTPACKKEY) && api[CONF_LIGHTPACKKEY].IsString()) {
                    this->LightpackKey = api[CONF_LIGHTPACKKEY].GetString();
                }
            }
        }

        LFXE_API MainConfigFile::WValue MainConfigFile::MakeJsonWString(const wstring& str, WDocument::AllocatorType& allocator) {
            WValue value(kStringType);
            value.SetString(str.c_str(), allocator);
            return value;
        }

    }
}
