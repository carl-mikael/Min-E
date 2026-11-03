add_rules("mode.debug", "mode.release")
---@diagnostic disable-next-line
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".zed" })

option("log")
    set_default(false)
    set_showmenu(true)
    set_description("Should log?")
    add_defines("MINE_LOG")
option_end()

if is_mode("debug") or has_config("log") then
    add_defines("MINE_LOG");
end

-- OpenGL Version
local GL_MAJOR_VERSION = 4
local GL_MINOR_VERSION = 6

add_defines("GL_MAJOR_V=" .. GL_MAJOR_VERSION, "GL_MINOR_V=" .. GL_MINOR_VERSION)
add_defines("GLSL_VERSION=" .. GL_MAJOR_VERSION .. GL_MINOR_VERSION .. "0")
add_defines("LOG_SIZE=" .. 512)

-- OpenGL Extensions
local glad_extensions = {
}

-- OpenGL Loader
local gl_api = "gl=" .. GL_MAJOR_VERSION .. "." .. GL_MINOR_VERSION

add_requires("glfw", "glm")

add_requires("glad v0.1.36", { configs = {
    spec = "gl",
    api = gl_api,
    profile = "core",
    extensions = table.concat(glad_extensions, ","),
    generator = "c",
    loader = true,
} })

target("Min-E")
    set_languages("c++20")
    set_kind("binary")
    add_files("src/*.cpp")

    add_packages("glfw", "glad", "glm")
    add_options("log")
    add_defines("GLFW_INCLUDE_NONE")

    after_build( function (target)
        os.cp("$(projectdir)/src/shader", target:targetdir())
    end)


--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--
