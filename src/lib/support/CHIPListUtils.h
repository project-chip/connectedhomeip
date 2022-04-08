/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#ifndef CHIP_LISTUTILS_INTERNAL_H
#define CHIP_LISTUTILS_INTERNAL_H

#include <app/data-model/List.h>
#include <set>
#include <type_traits>

// TODO move to chip namespace
template <typename T>
struct ListMemberTypeGetter
{
};
template <typename T>
struct ListMemberTypeGetter<chip::app::DataModel::List<T>>
{
    // We use List<const ...> in data model data structures, so consumers can
    // use const data.  Just grab the type with the const stripped off.
    using Type = std::remove_const_t<T>;
};

struct ListHolderBase
{
    // Just here so we can delete an instance to trigger the subclass destructor.
    virtual ~ListHolderBase() {}
};

template <typename T>
struct ListHolder : ListHolderBase
{
    ListHolder(size_t N) { mList = new T[N]; }
    ~ListHolder() { delete[] mList; }
    T * mList;
};

struct ListFreer
{
    ~ListFreer()
    {
        for (auto listHolder : mListHolders)
        {
            delete listHolder;
        }
    }

    void add(ListHolderBase * listHolder) { mListHolders.insert(listHolder); }

    std::set<ListHolderBase *> mListHolders;
};

#endif /* CHIP_LISTUTILS_INTERNAL_H */
