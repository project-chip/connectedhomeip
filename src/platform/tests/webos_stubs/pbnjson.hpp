/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace pbnjson {

class JValue
{
public:
    enum class Type
    {
        kNull,
        kObject,
        kArray,
        kString,
        kBool,
        kNumber,
    };

    JValue() = default;
    explicit JValue(Type type) : mType(type) {}
    JValue(const char * value) : mType(Type::kString), mString(value != nullptr ? value : "") {}
    JValue(const std::string & value) : mType(Type::kString), mString(value) {}
    JValue(bool value) : mType(Type::kBool), mBool(value) {}
    JValue(int32_t value) : mType(Type::kNumber), mNumber(value) {}

    bool hasKey(const char * key) const { return mType == Type::kObject && key != nullptr && mObject.find(key) != mObject.end(); }

    void put(const char * key, const JValue & value)
    {
        EnsureObject();
        if (key != nullptr)
        {
            mObject[key] = value;
        }
    }

    void put(const char * key, const char * value) { put(key, JValue(value)); }
    void put(const char * key, const std::string & value) { put(key, JValue(value)); }
    void put(const char * key, bool value) { put(key, JValue(value)); }
    void put(const char * key, int32_t value) { put(key, JValue(value)); }

    void remove(const char * key)
    {
        if (mType == Type::kObject && key != nullptr)
        {
            mObject.erase(key);
        }
    }

    void append(const JValue & value)
    {
        EnsureArray();
        mArray.push_back(value);
    }

    void append(const char * value) { append(JValue(value)); }
    void append(const std::string & value) { append(JValue(value)); }
    void append(int32_t value) { append(JValue(value)); }
    void append(bool value) { append(JValue(value)); }

    JValue operator[](const char * key) const
    {
        if (mType != Type::kObject || key == nullptr)
        {
            return JValue();
        }

        auto item = mObject.find(key);
        return item == mObject.end() ? JValue() : item->second;
    }

    JValue operator[](int index) const
    {
        if (mType != Type::kArray || index < 0 || static_cast<size_t>(index) >= mArray.size())
        {
            return JValue();
        }

        return mArray[static_cast<size_t>(index)];
    }

    ssize_t arraySize() const { return mType == Type::kArray ? static_cast<ssize_t>(mArray.size()) : 0; }

    std::string asString() const
    {
        if (mType == Type::kString)
        {
            return mString;
        }
        if (mType == Type::kBool)
        {
            return mBool ? "true" : "false";
        }
        if (mType == Type::kNumber)
        {
            return std::to_string(mNumber);
        }
        return "";
    }

    bool asBool() const { return mType == Type::kBool ? mBool : false; }

    template <typename T>
    T asNumber() const
    {
        return static_cast<T>(mType == Type::kNumber ? mNumber : 0);
    }

    template <typename T>
    void asNumber(T & value) const
    {
        value = asNumber<T>();
    }

    std::string stringify() const
    {
        switch (mType)
        {
        case Type::kObject:
            return StringifyObject();
        case Type::kArray:
            return StringifyArray();
        case Type::kString:
            return "\"" + Escape(mString) + "\"";
        case Type::kBool:
            return mBool ? "true" : "false";
        case Type::kNumber:
            return std::to_string(mNumber);
        case Type::kNull:
            return "null";
        }
        return "null";
    }

private:
    void EnsureObject()
    {
        if (mType != Type::kObject)
        {
            mType = Type::kObject;
            mObject.clear();
            mArray.clear();
            mString.clear();
        }
    }

    void EnsureArray()
    {
        if (mType != Type::kArray)
        {
            mType = Type::kArray;
            mObject.clear();
            mArray.clear();
            mString.clear();
        }
    }

    static std::string Escape(const std::string & input)
    {
        std::string escaped;
        escaped.reserve(input.size());
        for (char c : input)
        {
            if (c == '"' || c == '\\')
            {
                escaped.push_back('\\');
            }
            escaped.push_back(c);
        }
        return escaped;
    }

    std::string StringifyObject() const
    {
        std::string output = "{";
        bool first         = true;
        for (const auto & item : mObject)
        {
            if (!first)
            {
                output += ",";
            }
            first = false;
            output += "\"" + Escape(item.first) + "\":" + item.second.stringify();
        }
        output += "}";
        return output;
    }

    std::string StringifyArray() const
    {
        std::string output = "[";
        bool first         = true;
        for (const auto & item : mArray)
        {
            if (!first)
            {
                output += ",";
            }
            first = false;
            output += item.stringify();
        }
        output += "]";
        return output;
    }

    Type mType = Type::kNull;
    std::map<std::string, JValue> mObject;
    std::vector<JValue> mArray;
    std::string mString;
    bool mBool      = false;
    int64_t mNumber = 0;
};

using JValueArrayElement = JValue;

inline JValue JObject()
{
    return JValue(JValue::Type::kObject);
}

inline JValue JArray()
{
    return JValue(JValue::Type::kArray);
}

class JDomParser
{
public:
    static JValue fromString(const std::string & input)
    {
        JValue value = JObject();
        if (input.find("\"returnValue\"") != std::string::npos && input.find("true") != std::string::npos)
        {
            value.put("returnValue", true);
        }
        return value;
    }

    static JValue fromString(const char * input) { return fromString(std::string(input != nullptr ? input : "")); }
};

} // namespace pbnjson
