/* XMRig
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/net/stratum/AutoClient.h"
#include "3rdparty/rapidjson/document.h"
#include "base/io/json/Json.h"
#include "net/JobResult.h"


xmrig::AutoClient::AutoClient(int id, const char *agent, IClientListener *listener) :
    EthStratumClient(id, agent, listener)
{
}


bool xmrig::AutoClient::handleResponse(int64_t id, const rapidjson::Value &result, const rapidjson::Value &error)
{
    if (m_mode == DEFAULT_MODE) {
        return Client::handleResponse(id, result, error); // NOLINT(bugprone-parent-virtual-call)
    }

    return EthStratumClient::handleResponse(id, result, error);
}


bool xmrig::AutoClient::parseLogin(const rapidjson::Value &result, int *code)
{
    if (result.HasMember("job")) {
        return Client::parseLogin(result, code);
    }

    setRpcId(Json::getString(result, "id"));
    if (rpcId().isNull()) {
        *code = 1;
        return false;
    }

    const Algorithm algo(Json::getString(result, "algo"));
    if (algo.family() != Algorithm::KAWPOW && algo.family() != Algorithm::GHOSTRIDER) {
        *code = 6;
        return false;
    }

    try {
        setExtraNonce(Json::getValue(result, "extra_nonce"));
    } catch (const std::exception &ex) {
        *code = 6;
        return false;
    }

    m_mode = ETH_MODE;
    setAlgo(algo);

    return true;
}


int64_t xmrig::AutoClient::submit(const JobResult &result)
{
    if (result.algorithm.family() != Algorithm::KAWPOW || result.algorithm.family() != Algorithm::GHOSTRIDER) {
        return Client::submit(result); // NOLINT(bugprone-parent-virtual-call)
    }

    return EthStratumClient::submit(result);
}


void xmrig::AutoClient::parseNotification(const char *method, const rapidjson::Value &params, const rapidjson::Value &error)
{
    if (strcmp(method, "job") == 0) {
        m_mode = DEFAULT_MODE;
        return Client::parseNotification(method, params, error); // NOLINT(bugprone-parent-virtual-call)
    }

    m_mode = ETH_MODE;
    return EthStratumClient::parseNotification(method, params, error);
}
