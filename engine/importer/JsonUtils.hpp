#include "json.hpp"

namespace MFA::JsonUtils
{
    template<typename T>
    static T TryGetValue(
        nlohmann::json const & jsonObject,
        char const * keyName,
        T defaultValue
    )
    {
        T result = defaultValue;
        const auto findResult = jsonObject.find(keyName);
        if (findResult != jsonObject.end())
        {
            result = jsonObject.value<T>(keyName, defaultValue);
        }
        return result;
    }
}