#include "BodyMetaData.h"
#include "BufferExchange.h"
#include <vector>

class FrontManager {
    public:
        explicit FrontManager(BufferExchange &exchange, std::vector<BodyMetaData> metaData) {};
};