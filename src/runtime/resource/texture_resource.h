#pragma once

#include "resource.h"
#include "LVK.h"

namespace ash
{
class TextureResource : public Resource
{
  public:
    lvk::Holder<lvk::TextureHandle> texture;
};

using TexturePtr = ResourcePtr<TextureResource>;
} // namespace ash
