#include "AsstCaller.h"

#include <cstring>
#include <filesystem>
#include <iostream>

#include <meojson/json.hpp>

#include "Assistant.h"
#include "Common/AsstTypes.h"
#include "Common/AsstVersion.h"
#include "Config/ResourceLoader.h"
#include "Utils/Logger.hpp"
#include "Utils/WorkingDir.hpp"

static constexpr AsstSize NullSize = static_cast<AsstSize>(-1);

#if 0
#if _MSC_VER
// Win32平台下Dll的入口
BOOL APIENTRY DllMain(HINSTANCE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#elif VA_GNUC

#endif
#endif

bool inited()
{
    return asst::ResourceLoader::get_instance().loaded();
}

AsstBool AsstSetUserDir(const char* path)
{
    return asst::UserDir.set(path);
}

AsstBool AsstLoadResource(const char* path)
{
    using namespace asst::utils::path_literals;

    auto os_path = asst::utils::path(path);
    auto res_path = os_path / "resource"_p;
    if (asst::ResDir.empty()) {
        asst::ResDir.set(res_path);
    }
    if (asst::UserDir.empty()) {
        asst::UserDir.set(os_path);
    }
    return asst::ResourceLoader::get_instance().load(res_path);
}

AsstBool AsstSetStaticOption(AsstStaticOptionKey key, const char* value)
{
    if (!inited()) {
        return false;
    }
    return AsstExtAPI::set_static_option(static_cast<asst::StaticOptionKey>(key), value);
}

AsstHandle AsstCreate()
{
    if (!inited()) {
        return nullptr;
    }
    return new asst::Assistant();
}

AsstHandle AsstCreateEx(AsstApiCallback callback, void* custom_arg)
{
    if (!inited()) {
        return nullptr;
    }
    return new asst::Assistant(static_cast<asst::ApiCallback>(callback), custom_arg);
}

void AsstDestroy(AsstHandle handle)
{
    if (handle == nullptr) {
        return;
    }

    delete handle;
    handle = nullptr;
}

AsstBool AsstSetInstanceOption(AsstHandle handle, AsstInstanceOptionKey key, const char* value)
{
    if (handle == nullptr) {
        return false;
    }

    return handle->set_instance_option(static_cast<asst::InstanceOptionKey>(key), value);
}

AsstBool AsstConnect(AsstHandle handle, const char* adb_path, const char* address, const char* config)
{
    if (!inited() || handle == nullptr) {
        return false;
    }

    return handle->connect(adb_path, address, config ? config : std::string());
}

AsstBool AsstStart(AsstHandle handle)
{
    if (!inited() || handle == nullptr) {
        return false;
    }

    return handle->start();
}

AsstBool AsstStop(AsstHandle handle)
{
    if (!inited() || handle == nullptr) {
        return false;
    }

    return handle->stop();
}

AsstBool AsstRunning(AsstHandle handle)
{
    if (!inited() || handle == nullptr) {
        return false;
    }

    return handle->running();
}

AsstAsyncCallId AsstAsyncConnect(AsstHandle handle, const char* adb_path, const char* address, const char* config,
                                 AsstBool block)
{
    if (!inited() || handle == nullptr) {
        return false;
    }
    return handle->async_connect(adb_path, address, config ? config : std::string(), block);
}

AsstTaskId AsstAppendTask(AsstHandle handle, const char* type, const char* params)
{
    if (!inited() || handle == nullptr) {
        return 0;
    }

    return handle->append_task(type, params ? params : "");
}

AsstBool AsstSetTaskParams(AsstHandle handle, AsstTaskId id, const char* params)
{
    if (!inited() || handle == nullptr) {
        return false;
    }

    return handle->set_task_params(id, params ? params : "");
}

AsstAsyncCallId AsstAsyncClick(AsstHandle handle, int32_t x, int32_t y, AsstBool block)
{
    if (!inited() || handle == nullptr) {
        return false;
    }
    return handle->async_click(x, y, block);
}

AsstAsyncCallId AsstAsyncScreencap(AsstHandle handle, AsstBool block)
{
    if (!inited() || handle == nullptr) {
        return false;
    }
    return handle->async_screencap(block);
}

AsstSize AsstGetImage(AsstHandle handle, void* buff, AsstSize buff_size)
{
    if (!inited() || handle == nullptr || buff == nullptr) {
        return NullSize;
    }
    auto img_data = handle->get_image();
    size_t data_size = img_data.size();
    if (buff_size < data_size) {
        return NullSize;
    }
    memcpy(buff, img_data.data(), data_size * sizeof(decltype(img_data)::value_type));
    return data_size;
}

AsstSize AsstGetUUID(AsstHandle handle, char* buff, AsstSize buff_size)
{
    if (!inited() || handle == nullptr || buff == nullptr) {
        return NullSize;
    }
    auto uuid = handle->get_uuid();
    size_t data_size = uuid.size();
    if (buff_size < data_size) {
        return NullSize;
    }
    memcpy(buff, uuid.data(), data_size * sizeof(decltype(uuid)::value_type));
    return data_size;
}

AsstSize AsstGetTasksList(AsstHandle handle, AsstTaskId* buff, AsstSize buff_size)
{
    if (!inited() || handle == nullptr || buff == nullptr) {
        return NullSize;
    }
    auto tasks = handle->get_tasks_list();
    size_t data_size = tasks.size();
    if (buff_size < data_size) {
        return NullSize;
    }
    memcpy(buff, tasks.data(), data_size * sizeof(decltype(tasks)::value_type));
    return data_size;
}

AsstSize AsstGetNullSize()
{
    return NullSize;
}

const char* AsstGetVersion()
{
    return asst::Version;
}

void AsstLog(const char* level, const char* message)
{
    if (asst::UserDir.empty()) {
        std::cerr << __FUNCTION__ << " | User Dir not set" << std::endl;
        return;
    }
    asst::Log.log(asst::Logger::level(level), message);
}
