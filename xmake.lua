set_project("small-test")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")
set_languages("c++20")
set_warnings("allextra")

if is_plat("android") then
    package("benchmark")
        set_kind("library")
        add_deps("cmake")
        set_urls("https://github.com/google/benchmark.git")
        add_versions("v1.9.0", "12235e24652fc7f809373e7c11a5f73c5763fc4c")
        
        -- Add description and homepage for better package management
        set_description("A microbenchmark support library")
        set_homepage("https://github.com/google/benchmark")

        on_install(function(package)
            local configs = {
                "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"),
                "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"),
                "-DBENCHMARK_DOWNLOAD_DEPENDENCIES=on",
                "-DHAVE_THREAD_SAFETY_ATTRIBUTES=0"
            }
            import("package.tools.cmake").install(package, configs)
        end)
    package_end()
end

-- Application requires
add_requires("glm 1.0.*", {alias = "glm"})
add_requires("spdlog")

-- Benchmark and testing requires
add_requires("gtest 1.15.*", {alias = "gtest"})
add_requires("benchmark 1.9.*", {alias = "benchmark"})


-- Vulkan requires
add_requires("vulkan-headers", "volk")
add_requires("vulkan-validationlayers")
add_requires("vulkan-memory-allocator")

local ANDROID_CONFIG = {
    ignored_devices = {"ZY22FLDDK7"},
    remote_base_path = "/data/local/tmp"  -- Base directory for all executables
}

-- Android deployment helper function
function run_on_android(target)
    local exec_path = target:targetfile()
    local target_name = target:name()
    local remote_path = ANDROID_CONFIG.remote_base_path  -- .. "/" .. target_name

    if not os.isfile(exec_path) then
        raise("Executable not found at: " .. exec_path)
    end

    -- Get connected devices
    local devices_output = try { function() 
        return os.iorun("adb devices")
    end}

    if not devices_output then
        raise("Failed to get device list from adb")
    end

    -- Parse device list
    local devices = {}
    for line in devices_output:gmatch("[^\r\n]+") do
        if line:find("%s*device$") then
            local device_id = line:match("(%S+)%s+device")
            if device_id and not table.contains(ANDROID_CONFIG.ignored_devices, device_id) then
                table.insert(devices, device_id)
            end
        end
    end

    if #devices == 0 then
        raise("No connected devices found!")
    end

    -- Run on each device
    import("core.base.option")
    local args = option.get("arguments") or {}

    for i, device_id in ipairs(devices) do
        print(string.format("[%d/%d] Running %s on device: %s", i, #devices, target_name, device_id))
        
        -- Deploy and execute
        local adb_commands = {
            {"-s", device_id, "push", exec_path, remote_path .. "/" .. target_name},
            {"-s", device_id, "shell", "chmod", "+x", remote_path .. "/" .. target_name},
        }

        -- Copy shaders
        local shader_dir = "./ppl/vulkan/shaders/compiled_shaders"
        for _, file in ipairs(os.files(shader_dir .. "/*.spv")) do
            table.insert(adb_commands, {"-s", device_id, "push", file, remote_path})
        end
        
        -- -- -- Execute commands

        for _, cmd in ipairs(adb_commands) do
            if os.execv("adb", cmd) ~= 0 then
                -- raise("Failed to execute adb command")
                print(string.format("Warning: Failed to execute adb command on device %s", device_id))
            end
        end

        -- Run the binary with arguments
        local run_command = {"-s", device_id, "shell", remote_path .. "/" .. target_name}

        table.join2(run_command, args, {"--device=" .. device_id})
        if os.execv("adb", run_command) ~= 0 then
            -- raise(string.format("Failed to run %s", target_name))
            print(string.format("Warning: Failed to run %s on device %s", target_name, device_id))
        end

        print()
    end
end

--

includes("ppl")
includes("tests")
includes("demos")
includes("benchmarks")
