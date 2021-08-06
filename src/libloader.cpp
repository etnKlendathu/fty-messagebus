#include "libloader.h"
#include <dlfcn.h>
#include <filesystem>
#include <fty_log.h>
#include <functional>
#include <unistd.h>
#include <vector>

namespace fty {

class LibLoader::Impl
{
    friend class LibLoader;

public:
    Impl(const std::string& name)
        : m_libName(name)
    {
    }

    void unload()
    {
        if (!m_pLibHandler)
            return;

        logDebug("Unload library {}", m_libName);

        if (dlclose(m_pLibHandler)) {
            logWarn("Cannot unload library {}", m_libName);
        }

        m_pLibHandler = nullptr;
    }

    Expected<void*> resolveFunc(const char* symbol)
    {
        void* func = dlsym(m_pLibHandler, symbol);
        if (!func) {
            return unexpected(dlerror());
        }
        return func;
    }

    Expected<void> preload()
    {
        if (auto ret = findFile(m_libName); !ret) {
            return unexpected(ret.error());
        } else {
            logDebug("Load library {}", *ret);
            if ((m_pLibHandler = dlopen(ret->c_str(), RTLD_LAZY)) == 0) {
                return unexpected(dlerror());
            }
            return {};
        }
    }

private:
    Expected<std::string> findFile(const std::string& name) const
    {
        static std::vector<std::filesystem::path> paths = {"/usr/lib/messagebus", "messagebus", "plugins"};

        for (const auto& path : paths) {
            auto check = path / name;
            if (std::filesystem::exists(check) && access(check.c_str(), X_OK) == 0) {
                return (path / name).string();
            }
        }

        return unexpected("File {} was not found", name);
    }

    void*       m_pLibHandler = 0;
    std::string m_libName;
};

LibLoader::LibLoader(const std::string& libName)
    : m_impl(new Impl(libName))
{
}

LibLoader::~LibLoader()
{
    m_impl->unload();
}

Expected<void*> LibLoader::resolve()
{
    if (!m_impl->m_pLibHandler) {
        if (!m_impl->preload())
            return 0;
    }
    auto plugFunc = m_impl->resolveFunc("pluginInstance");
    if (!plugFunc) {
        return unexpected("Cannot resolve function 'pluginInstance' from {}", m_impl->m_libName);
    }
    return *plugFunc;
}

} // namespace fty
