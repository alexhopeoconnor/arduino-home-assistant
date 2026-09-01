
#include <Arduino.h>

#include "HASerializerArray.h"
#include "HADictionary.h"
#include "HAJson.h"

HASerializerArray::HASerializerArray(const uint8_t size, const bool progmemItems) :
    _progmemItems(progmemItems),
    _size(size),
    _itemsNb(0),
    _items(new ItemType[size])
{

}

HASerializerArray::~HASerializerArray()
{
    delete[] _items;
}

bool HASerializerArray::add(ItemType item)
{
    if (_itemsNb >= _size) {
        return false;
    }

    _items[_itemsNb++] = item;
    return true;
}

const char* HASerializerArray::getItem(const uint8_t index) const
{
    if (index >= _itemsNb) {
        return nullptr;
    }

    return _items[index];
}

uint16_t HASerializerArray::calculateSize() const
{
    uint32_t size =
        strlen_P(HASerializerJsonArrayPrefix) +
        strlen_P(HASerializerJsonArraySuffix);

    if (_itemsNb == 0) {
        return static_cast<uint16_t>(size);
    }

    // separators between elements
    size += (_itemsNb - 1) * strlen_P(HASerializerJsonPropertiesSeparator);

    for (uint8_t i = 0; i < _itemsNb; i++) {
        if (!_items[i]) {
            return 0;
        }

        const uint16_t itemSize = _progmemItems
            ? HAJson::calculateEscapedProgmemStringSize(_items[i])
            : HAJson::calculateEscapedStringSize(_items[i]);
        if (itemSize == 0) {
            return 0;
        }

        size += itemSize;
        if (size > UINT16_MAX) {
            return 0;
        }
    }

    return static_cast<uint16_t>(size);
}

bool HASerializerArray::serialize(char* output) const
{
    if (!output) {
        return false;
    }

    const uint16_t size = calculateSize();
    if (size == 0) {
        return false;
    }

    char* cursor = output;
    char* const end = output + size;
    *cursor++ = '[';
    *cursor = 0;

    for (uint8_t i = 0; i < _itemsNb; i++) {
        if (i > 0) {
            *cursor++ = ',';
            *cursor = 0;
        }

        const bool serialized = _progmemItems
            ? HAJson::appendEscapedProgmemString(cursor, end, _items[i])
            : HAJson::appendEscapedString(cursor, end, _items[i]);
        if (!serialized) {
            return false;
        }
    }

    *cursor++ = ']';
    *cursor = 0;
    return static_cast<uint16_t>(cursor - output) == size;
}

void HASerializerArray::clear()
{
    _itemsNb = 0;
}
