#include "base.h"
#include "RumiConfig.h"
#include <iostream>

void printInfo() {
  std::cout << "Rumi Version " << Rumi_VERSION_MAJOR << "."
            << Rumi_VERSION_MINOR << std::endl;
}
