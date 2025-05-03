#include "plugins/EmptyScreenPlugin.h"

void EmptyScreenPlugin::setup()
{
  Screen.clear();
}

void EmptyScreenPlugin::loop()
{
}

const char *EmptyScreenPlugin::getName() const
{
  return "Empty Screen";
}
