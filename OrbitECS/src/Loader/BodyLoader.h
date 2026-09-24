#ifndef BODYLOADER_H
#define BODYLOADER_H

#include "LoadedBodies.h"
#include <string_view>

namespace BodyLoader {
  LoadedBodies load(std::string_view heavyPath, std::string_view lightPath, int nb_asteroids);
};

#endif