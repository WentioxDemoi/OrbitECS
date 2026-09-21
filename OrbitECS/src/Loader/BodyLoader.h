#ifndef BODYLOADER_H
#define BODYLOADER_H

#include "Components/HeavyBodies.h"
#include "Components/LightBodies.h"
#include "BodyMetaData.h"
#include <string_view>
#include <vector>

struct LoadedBodies {
    HeavyBodies heavy;
    LightBodies light;
    std::vector<BodyMetaData> meta;
};

class BodyLoader {
    public: 
        BodyLoader() = delete;

        // We can improve that part as we return the tabs by copy
        static LoadedBodies load(const std::string_view path);
};

#endif