#include "mlm-message.h"
#include <fty_log.h>

namespace fty::messagebus::plugin {

zmsg_t* toMalamuteMsg(const Message& msg)
{
    zmsg_t* zmsg = zmsg_new();

    zmsg_addstr(zmsg, "__METADATA_START");
    for (const auto& fld : msg.meta.fields()) {
        if (!fld->hasValue()) {
            continue;
        }
        const pack::IValue* val = dynamic_cast<const pack::IValue*>(fld);
        if (!val) {
            logWarn("Message meta field should be a value");
            continue;
        }

        std::string store = [&]() {
            switch (val->valueType()) {
                case pack::Type::Bool:
                    return fty::convert<std::string>(static_cast<const pack::Bool*>(val)->value());
                case pack::Type::Double:
                    return fty::convert<std::string>(static_cast<const pack::Double*>(val)->value());
                case pack::Type::Float:
                    return fty::convert<std::string>(static_cast<const pack::Float*>(val)->value());
                case pack::Type::String:
                    return fty::convert<std::string>(static_cast<const pack::String*>(val)->value());
                case pack::Type::Int32:
                    return fty::convert<std::string>(static_cast<const pack::Int32*>(val)->value());
                case pack::Type::UInt32:
                    return fty::convert<std::string>(static_cast<const pack::UInt32*>(val)->value());
                case pack::Type::Int64:
                    return fty::convert<std::string>(static_cast<const pack::Int64*>(val)->value());
                case pack::Type::UInt64:
                    return fty::convert<std::string>(static_cast<const pack::UInt64*>(val)->value());
                case pack::Type::UChar:
                    return fty::convert<std::string>(static_cast<const pack::UChar*>(val)->value());
                case pack::Type::Unknown:
                    throw std::runtime_error("Unsupported type to unpack");
            }
            return std::string{};
        }();

        zmsg_addmem(zmsg, fld->key().c_str(), fld->key().size());
        zmsg_addmem(zmsg, store.c_str(), store.size());
    }
    zmsg_addstr(zmsg, "__METADATA_END");

    for (const auto& item : msg.userData) {
        zmsg_addmem(zmsg, item.c_str(), item.size());
    }

    return zmsg;
}

Message fromMalamuteMsg(zmsg_t* msg)
{
    Message   message;
    zframe_t* item;

    const auto metaFlds = message.meta.fields();

    if (zmsg_size(msg)) {
        item = zmsg_pop(msg);
        std::string key(reinterpret_cast<const char*>(zframe_data(item)), zframe_size(item));
        zframe_destroy(&item);
        if (key == "__METADATA_START") {
            while ((item = zmsg_pop(msg))) {
                key = std::string(reinterpret_cast<const char*>(zframe_data(item)), zframe_size(item));
                zframe_destroy(&item);
                if (key == "__METADATA_END") {
                    break;
                }

                zframe_t*   zvalue = zmsg_pop(msg);
                std::string value(reinterpret_cast<const char*>(zframe_data(zvalue)), zframe_size(zvalue));
                zframe_destroy(&zvalue);

                auto it = std::find_if(metaFlds.begin(), metaFlds.end(), [&](const auto* attr) {
                    return attr->key() == key;
                });

                if (it != metaFlds.end()) {
                    auto ival = static_cast<pack::IValue*>(*it);
                    switch (ival->valueType()) {
                    case pack::Type::Bool:
                        static_cast<pack::Bool*>(ival)->setValue(fty::convert<bool>(value));
                        break;
                    case pack::Type::Double:
                        static_cast<pack::Double*>(ival)->setValue(fty::convert<double>(value));
                        break;
                    case pack::Type::Float:
                        static_cast<pack::Float*>(ival)->setValue(fty::convert<float>(value));
                        break;
                    case pack::Type::String:
                        static_cast<pack::String*>(ival)->setValue(fty::convert<std::string>(value));
                        break;
                    case pack::Type::Int32:
                        static_cast<pack::Int32*>(ival)->setValue(fty::convert<int32_t>(value));
                        break;
                    case pack::Type::UInt32:
                        static_cast<pack::UInt32*>(ival)->setValue(fty::convert<uint32_t>(value));
                        break;
                    case pack::Type::Int64:
                        static_cast<pack::Int64*>(ival)->setValue(fty::convert<int64_t>(value));
                        break;
                    case pack::Type::UInt64:
                        static_cast<pack::UInt64*>(ival)->setValue(fty::convert<uint64_t>(value));
                        break;
                    default:
                        throw std::runtime_error("Unsupported type to unpack");
                    }
                } else {
                    logWarn("Not existng field '{}' in meta", key);
                }
            }
        } else {
            message.userData.append(key);
        }

        while ((item = zmsg_pop(msg))) {
            message.userData.append(std::string(reinterpret_cast<const char*>(zframe_data(item)), zframe_size(item)));
            zframe_destroy(&item);
        }
    }
    return message;
}

} // namespace fty::messagebus::plugin
