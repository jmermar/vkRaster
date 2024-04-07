#pragma once
#include <vector>
#include <string>

namespace vkr {
    class ResourceManagerImp;

    struct ShaderBinary {
        std::vector<uint8_t> data;
    };

    class ResourceManager {
        private:
            ResourceManagerImp* imp;
            ResourceManager();
        public:
            ~ResourceManager();

            ShaderBinary loadShaderBinary(const std::string& path);
    };
};