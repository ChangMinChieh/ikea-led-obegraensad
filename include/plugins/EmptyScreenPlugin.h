#pragma once

#include "PluginManager.h"

class EmptyScreenPlugin : public Plugin
{
public:
  void setup() override;
  void loop() override;
  const char *getName() const override;
};