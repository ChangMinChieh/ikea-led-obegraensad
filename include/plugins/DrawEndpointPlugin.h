#pragma once

#include "PluginManager.h"

class DrawEndpointPlugin : public Plugin
{
public:
  void setup() override;
  void teardown() override;
  const char *getName() const override;
  const char *restHook(const char *command, const char *dataJson) override;
};
