
local function build_launcher()
    set_kind("binary")
    set_group("Client")
    set_symbols("debug", "hidden")
    add_ldflags(
        "/IGNORE:4254",
        "/DYNAMICBASE:NO",
        "/SAFESEH:NO",
        "/LARGEADDRESSAWARE",
        "/INCREMENTAL:NO",
        "/MANIFEST:NO",
        "/LAST:.zdata",
        "/SUBSYSTEM:WINDOWS",
        "/ENTRY:mainCRTStartup", { force = true })
    add_includedirs(
        ".",
        "../",
        "../../Libraries/")
    add_headerfiles("**.h")
    add_files(
        "**.cpp",
        "resources/launcher.rc")
    add_deps(
        "TiltedReverse",
        "TiltedHooks",
        "TiltedUi",
        "ImGuiImpl",
        "Common")
    add_links("ntdll_x64")
    add_linkdirs(".")
    add_syslinks(
        "user32",
        "shell32",
        "comdlg32",
        "bcrypt",
        "ole32",
        "dxgi",
        "d3d11",
        "gdi32")
    add_packages(
        "tiltedcore",
        "spdlog",
        "minhook",
        "hopscotch-map",
        "cryptopp",
        "glm",
        "cef",
        "mem")
end

target("SkyrimImmersiveLauncher")
    set_basename("SkyrimTogether")
    add_defines(
        "TARGET_ST",
        "TARGET_PREFIX=\"st\"")
    add_deps("SkyrimTogetherClient")
    add_ldflags("/WHOLEARCHIVE:SkyrimTogetherClient", { force = true })
    build_launcher()

--[[target("FalloutImmersiveLauncher")
    set_basename("FalloutTogether")
    add_defines(
        "TARGET_FT",
        "TARGET_PREFIX=\"ft\"")
    add_deps("FalloutTogetherClient")
    add_ldflags("/WHOLEARCHIVE:FalloutTogetherClient", { force = true })
    build_launcher()
    ]]--

