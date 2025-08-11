#!/usr/bin/env luajit

-- build.lua - Simple Lua build script

local function execute(cmd)
    print("Running: " .. cmd)
    local result = os.execute(cmd)
    if result ~= 0 then
        print("Command failed!")
        os.exit(1)
    end
end

local function file_exists(path)
    local f = io.open(path, "r")
    if f then
        f:close()
        return true
    end
    return false
end

local function get_mtime(path)
    local handle = io.popen('stat -f %m "' .. path .. '" 2>/dev/null')
    if handle then
        local mtime = handle:read("*n")
        handle:close()
        return mtime or 0
    end
    return 0
end

local function needs_rebuild(target, source)
    if not file_exists(target) then return true end
    return get_mtime(source) > get_mtime(target)
end

-- Configuration
local CC = os.getenv("CC") or "cc"
local CFLAGS = os.getenv("CFLAGS") or "-g -Wall -Wextra -Wpedantic -std=c99"
local PROG = "myapp"
local SRCDIR = "src"
local OBJDIR = "obj"
local LINKS = "-lm"

-- Create obj directory
execute("mkdir -p " .. OBJDIR)

-- Find sources
local sources = {}
local handle = io.popen('find ' .. SRCDIR .. ' -name "*.c"')
if handle then
    for file in handle:lines() do
        table.insert(sources, file)
    end
    handle:close()
end

if #sources == 0 then
    print("No .c files found in " .. SRCDIR)
    os.exit(1)
end

-- Compile sources
local objects = {}
for _, src in ipairs(sources) do
    local obj = src:gsub('%.c$', '.o'):gsub('^' .. SRCDIR .. '/', OBJDIR .. '/')
    table.insert(objects, obj)
    
    if needs_rebuild(obj, src) then
        execute(CC .. " " .. CFLAGS .. " -c " .. src .. " -o " .. obj .. " " .. LINKS)
    else
        print("Up to date: " .. obj)
    end
end

-- Link program
local needs_link = not file_exists(PROG)
if not needs_link then
    local prog_mtime = get_mtime(PROG)
    for _, obj in ipairs(objects) do
        if get_mtime(obj) > prog_mtime then
            needs_link = true
            break
        end
    end
end

if needs_link then
    execute(CC .. " " .. CFLAGS .. " -o " .. PROG .. " " .. table.concat(objects, " ") .. " " .. LINKS )
else
    print("Up to date: " .. PROG)
end

print("Build complete: " .. PROG)
