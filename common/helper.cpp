#include <chrono>
#include <ctime>
#include <random>
#include <uuid/uuid.h>
#include "helper.h"

namespace fty::messagebus::utils {

const std::string generateUuid()
{
    uuid_t uuid;
    uuid_generate(uuid);
    char uuid_str[UUID_STR_LEN + 1];
    uuid_unparse_lower(uuid, uuid_str);
    return uuid_str;
}

const std::string generateId()
{
    std::random_device                 rd;
    std::mt19937                       rng(rd());
    std::uniform_int_distribution<int> uni(0, RAND_MAX);
    return std::to_string(uni(rng));
}

const std::string getClientId(const std::string& prefix)
{
    std::chrono::milliseconds ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    std::string clientId = prefix + "-" + std::to_string(ms.count());
    return clientId;
}

}
