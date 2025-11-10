#include <utils/Systems.hpp>

#include <iostream>

void System::clear() {
#ifdef WIN32
  system("cls");
#else
  system("clear");
#endif
}
