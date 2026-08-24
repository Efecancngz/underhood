#pragma once

#include <string>

namespace underhood {

struct Parameter {
    std::string name;
    int value;
    int minValue;
    int maxValue;
};

}  // namespace underhood
