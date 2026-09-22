#ifndef BODYLOADER_H
#define BODYLOADER_H

#include "LoadedBodies.h"
#include <string_view>

class BodyLoader {
public:
  BodyLoader() = delete;

  static LoadedBodies load(const std::string_view path) {};
};

#endif