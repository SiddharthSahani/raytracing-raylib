
#pragma once

#include "src/compiled-scene.h"


std::unique_ptr<rt::CompiledScene> createRandomScene(int numSpheres, int numMats);

std::unique_ptr<rt::CompiledScene> createScene_1();

std::unique_ptr<rt::CompiledScene> createScene_2();
