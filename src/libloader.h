/*  ========================================================================================================================================
   message.h - Common message bus wrapper

   Copyright (C) 2014 - 2020 Eaton

   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
==========================================================================================================================================*/

#pragma once

#include <memory>
#include <fty/expected.h>

namespace fty {

/// Loads a library and resolve some special functions
class LibLoader
{
public:
    LibLoader(const std::string& libName);

    ~LibLoader();

    template <typename Plugin, typename... Args>
    Expected<Plugin*> load(const Args&... args)
    {
        using Func = Plugin* (*)(Args...);

        auto plugFunc = resolve();
        if (plugFunc) {
            return reinterpret_cast<Func>(*plugFunc)(args...);
        } else {
            return fty::unexpected(plugFunc.error());
        }
    }

    void unload();

private:
    Expected<void*> resolve();

    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace fty
