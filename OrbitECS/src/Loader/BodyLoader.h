#ifndef BODYLOADER_H
#define BODYLOADER_H

#include "LoadedBodies.h"
#include <string_view>

namespace BodyLoader {
  LoadedBodies load(const std::string_view path);
};

#endif