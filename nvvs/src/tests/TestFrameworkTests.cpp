/*
 * Copyright (c) 2024, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "DcgmStringHelpers.h"
#include "dcgm_fields.h"
#include "dcgm_structs.h"
#include <catch2/catch_all.hpp>
#include <cstring>
#include <sys/stat.h>

#include <TestFramework.h>

class WrapperTestFramework : protected TestFramework
{
public:
    WrapperTestFramework(std::vector<std::unique_ptr<EntitySet>> &entitySet);
    std::string WrapperGetPluginUsingDriverDir();
    std::string WrapperGetPluginBaseDir();
    std::string WrapperGetPluginCudaDirExtension() const;
    std::string WrapperGetPluginCudalessDir();
};

WrapperTestFramework::WrapperTestFramework(std::vector<std::unique_ptr<EntitySet>> &entitySet)
    : TestFramework(entitySet)
{}

std::string WrapperTestFramework::WrapperGetPluginUsingDriverDir()
{
    return GetPluginUsingDriverDir().value_or("");
}
std::string WrapperTestFramework::WrapperGetPluginBaseDir()
{
    return GetPluginBaseDir();
}
std::string WrapperTestFramework::WrapperGetPluginCudaDirExtension() const
{
    return GetPluginCudaDirExtension().value_or("");
}
std::string WrapperTestFramework::WrapperGetPluginCudalessDir()
{
    return GetPluginCudalessDir();
}

/*
 * This function's behaviour mirrors Linux's /proc/<pid>/exec
 * It follows symlinks and returns the location of the executable
 */
std::string getThisExecsLocation()
{
    char szTmp[64];
    char buf[1024] = { 0 };
    snprintf(szTmp, sizeof(szTmp), "/proc/%d/exe", getpid());
    auto const bytes = readlink(szTmp, buf, sizeof(buf) - 1);
    if (bytes >= 0)
    {
        buf[bytes] = '\0';
    }
    else
    {
        throw std::runtime_error("Test error. Expected bytes >= 0");
    }
    std::string path { buf };
    std::string parentDir = path.substr(0, path.find_last_of("/"));
    return parentDir;
}

SCENARIO("GetPluginBaseDir returns plugin directory relative to current process's location")
{
    const std::string myLocation = getThisExecsLocation();
    const std::string pluginDir  = myLocation + "/plugins";
    std::vector<std::unique_ptr<EntitySet>> entitySet;
    WrapperTestFramework tf(entitySet);

    rmdir(pluginDir.c_str());
    CHECK_THROWS(tf.WrapperGetPluginBaseDir());

    int st = mkdir(pluginDir.c_str(), 770);
    if (st != 0)
    {
        // This has fail at this point. The checks below are only to aid in debugging the failure
        CHECK(std::string { "" } == pluginDir);
        CHECK(std::string { "" } == std::string { strerror(errno) });
    }
    CHECK(tf.WrapperGetPluginBaseDir() == pluginDir);
}

SCENARIO("GetPluginCudalessDir returns cudaless directory in plugin directory")
{
    const std::string myLocation = getThisExecsLocation();
    const std::string pluginDir  = myLocation + "/plugins/cudaless/";
    std::vector<std::unique_ptr<EntitySet>> entitySet;
    WrapperTestFramework tf(entitySet);
    CHECK(tf.WrapperGetPluginCudalessDir() == pluginDir);
}