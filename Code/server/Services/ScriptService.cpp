#include <stdafx.h>

#include <Services/CalendarService.h>
#include <Services/ScriptService.h>
#include <World.h>

#include <Events/PlayerEnterWorldEvent.h>
#include <Events/UpdateEvent.h>

#include <Components.h>
#include <GameServer.h>
#include <TiltedCore/Filesystem.hpp>

#include <resources/ResourceCollection.h>

namespace Script
{
void CreateScriptBindings(sol::state& aState);
}

ScriptService::ScriptService(World& aWorld, entt::dispatcher& aDispatcher)
    : m_world(aWorld), m_updateConnection(aDispatcher.sink<UpdateEvent>().connect<&ScriptService::OnUpdate>(this)),
      m_playerEnterWorldConnection(
          aDispatcher.sink<PlayerEnterWorldEvent>().connect<&ScriptService::OnPlayerEnterWorld>(this))
{
}

ScriptService::~ScriptService()
{
    auto &lockedState = m_lua.Lock().Get();
    for (auto& m : m_sandboxes)
    {
        m.clear();
    }
 
    lockedState.collect_garbage();
}

void ScriptService::Initialize(Resources::ResourceCollection& aCollection) noexcept
{
    auto lua = m_lua.Lock();
    auto& luaVm = lua.Get();
    m_globals = luaVm.globals();

    luaVm.open_libraries(sol::lib::base, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::package,
                         sol::lib::os, sol::lib::table, sol::lib::bit32);

    // make sure to set package path to current directory scope
    // as this could get overriden by LUA_PATH environment variable
    luaVm["package"]["path"] = "./?.lua";

    BindInbuiltFunctions();

    aCollection.ForEachManifest([&](const Resources::Manifest001& aManifest) {
        if (aManifest.EntryPoint.empty()) // just a dependency?
            return;
        auto entryPointPath = aCollection.GetResourceFolderPath() / aManifest.FolderName / aManifest.EntryPoint;
        if (!std::filesystem::exists(entryPointPath))
        {
            spdlog::warn("Script entry point {} does not exist", entryPointPath.string());
            return;
        }
        LoadScript(entryPointPath);
    });
}

bool ScriptService::LoadScript(const std::filesystem::path& aPath)
{
    try
    {
        auto lua = m_lua.Lock();
        auto& luaVm = lua.Get();

        auto &env = m_sandboxes.emplace_back(luaVm, sol::create, luaVm.globals());
        
        const sol::protected_function_result result = luaVm.script_file(aPath.string(), env);
        if (!result.valid())
        {
            const sol::error error = result;
            spdlog::error("Failed to load script file: {}", error.what());
            return false;
        }
    }
    catch (std::exception& exception)
    {
        spdlog::error("Failed to load script file: {}", exception.what());
        return false;
    }

    return true;
}

void ScriptService::BindInbuiltFunctions()
{
    auto lua = m_lua.Lock();
    auto& luaVm = lua.Get();
    Script::CreateScriptBindings(luaVm);

    {
        luaVm.set_function("addEventHandler", [this](std::string acName, sol::function aFunction) {
            AddEventHandler(std::move(acName), std::move(aFunction));
        });
        luaVm.set_function("cancelEvent", [this](std::string acReason) { CancelEvent(std::move(acReason)); });
    }
}

std::tuple<bool, String> ScriptService::HandleCharacterMove(const entt::entity aNpc) noexcept
{
    return CallCancelableEvent("onCharacterMove", aNpc);
}

std::tuple<bool, String> ScriptService::HandleCharacterSpawn(const entt::entity aNpc) noexcept
{
    return CallCancelableEvent("onCharacterSpawn", aNpc);
}

std::tuple<bool, String> ScriptService::HandleCharacterDestoy(const entt::entity aNpc) noexcept
{
    return CallCancelableEvent("onCharacterDestroy", aNpc);
}

std::tuple<bool, String> ScriptService::HandlePlayerJoin(const ConnectionId_t aPlayer) noexcept
{
    return CallCancelableEvent("onPlayerJoin", aPlayer);
}

std::tuple<bool, String> ScriptService::HandleChatMessage(const entt::entity aSender, const String& aMessage) noexcept
{
    return CallCancelableEvent("onChatMessage", aSender, aMessage);
}

void ScriptService::HandlePlayerQuit(ConnectionId_t aConnectionId, Server::EDisconnectReason aReason) noexcept
{
    std::string reason;

    switch (aReason)
    {
    case Server::EDisconnectReason::Quit:
        reason = "Quit";
        break;
    case Server::EDisconnectReason::Kicked:
        reason = "Kicked";
        break;
    case Server::EDisconnectReason::Banned:
        reason = "Banned";
        break;
    case Server::EDisconnectReason::BadConnection:
        reason = "Connection lost";
        break;
    case Server::EDisconnectReason::TimedOut:
        reason = "Timed out";
        break;
    case Server::EDisconnectReason::Unknown:
    default:
        reason = "Unknown";
        break;
    }

    CallEvent("onPlayerQuit", aConnectionId, reason);
}

#if 0
void ScriptService::RegisterExtensions(ScriptContext& aContext)
{
    ScriptStore::RegisterExtensions(aContext);

    BindTypes(aContext);
    BindStaticFunctions(aContext);
}
#endif

void ScriptService::OnUpdate(const UpdateEvent& acEvent) noexcept
{
    if (m_sandboxes.size() == 0)
        return;

    try
    {
        CallEvent("onUpdate", acEvent.Delta);
    }
    catch (std::exception& exception)
    {
        spdlog::error("Script execution failure: {}", exception.what());
    }
}

void ScriptService::OnPlayerEnterWorld(const PlayerEnterWorldEvent& acEvent) noexcept
{
    /* const Script::Player cPlayer(acEvent.Entity, m_world);

    CallEvent("onPlayerEnterWorld", cPlayer);*/
}

void ScriptService::AddEventHandler(const std::string acName, const sol::function acFunction) noexcept
{
    m_callbacks[String(acName)].push_back(acFunction);
}

void ScriptService::CancelEvent(const std::string aReason) noexcept
{
    m_eventCanceled = true;
    m_cancelReason = aReason;
}
