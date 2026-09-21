#include "BodyMetaData.h"
#include "BufferExchange.h"
#include "Components/HeavyBodies.h"
#include "Components/LightBodies.h"
#include <vector>

class BackManager {
    public:
        explicit BackManager(HeavyBodies heavy, LightBodies light, BufferExchange *exchange);
};