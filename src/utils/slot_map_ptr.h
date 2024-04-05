#pragma once

#include <stdexcept>
#include "slot_map.h"

namespace ash
{
template <typename T>
class SlotMapPtr
{
  public:
    SlotMapPtr() = default;

    SlotMapPtr(stdext::slot_map<T>* slot_map,std::pair<unsigned, unsigned> key) : slot_map(slot_map), key(key)
    {
    }

    [[nodiscard]]
    bool is_valid() const
    {
        return slot_map && slot_map->find(key) != slot_map->end();
    }

    explicit operator bool() const
    {
        return is_valid();
    }

    const T& operator*() const
    {
        if (slot_map)
        {
            return slot_map->at(key);
        }
        throw std::runtime_error("Invalid slot map pointer");
    }

    T& operator*()
    {
        return const_cast<T&>(std::as_const(*this).operator*());
    }

    const T* operator->() const
    {
        if (slot_map)
        {
            auto it = slot_map->find(key);
            return it != slot_map->end() ? &*it : nullptr;
        }
        return nullptr;
    }

    T* operator->()
    {
        return const_cast<T*>(std::as_const(*this).operator->());
    }

    bool operator==(const SlotMapPtr& other) const
    {
        return slot_map == other.slot_map && key == other.key;
    }

    bool operator!=(const SlotMapPtr& other) const
    {
        return slot_map != other.slot_map || key != other.key;
    }
    
    [[nodiscard]] std::pair<unsigned, unsigned> get_key() const
    {
        return key;
    }

  private:
    stdext::slot_map<T>* slot_map = nullptr;
    std::pair<unsigned, unsigned> key;
};
} // namespace ash
