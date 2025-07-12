#include "chatutils.h"
#include "entities/charentity.h"
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <zmq.hpp>

namespace chatutils
{
    static zmq::context_t context(1);
    static zmq::socket_t  socket(context, zmq::socket_type::pub);

    void initialize()
    {
        try
        {
            socket.bind("tcp://*:5556");
            ShowInfo("ChatLogger ZMQ: Bound to tcp://*:5556\n");
        }
        catch (const zmq::error_t& e)
        {
            ShowError("ChatLogger ZMQ bind failed: %s\n", e.what());
        }
    }

    // Helper to escape JSON strings
    static std::string escapeJson(const std::string_view& input)
    {
        std::ostringstream oss;
        for (char c : input)
        {
            switch (c)
            {
                case '"':
                    oss << "\\\"";
                    break;
                case '\\':
                    oss << "\\\\";
                    break;
                case '\b':
                    oss << "\\b";
                    break;
                case '\f':
                    oss << "\\f";
                    break;
                case '\n':
                    oss << "\\n";
                    break;
                case '\r':
                    oss << "\\r";
                    break;
                case '\t':
                    oss << "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                        oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                    else
                        oss << c;
            }
        }
        return oss.str();
    }

    void log(CCharEntity* PChar, const char* rawMessage, uint8 messageType)
    {
        static std::once_flag initialized;
        std::call_once(initialized, initialize);

        const std::string name    = PChar ? PChar->getName() : "SYSTEM";
        const uint16_t    zoneId  = PChar ? PChar->getZone() : 0;
        const std::string escaped = escapeJson(rawMessage);

        // ShowDebug("ChatLogger [CHAT]: [%s] %s\n", name.c_str(), rawMessage);

        std::ostringstream json;
        json << "{"
             << "\"senderName\":\"" << name << "\","
             << "\"zoneId\":" << zoneId << ","
             << "\"messageType\":" << static_cast<int>(messageType) << ","
             << "\"message\":\"" << escaped << "\""
             << "}";

        const std::string jsonStr = json.str();
        zmq::message_t    msg(jsonStr.begin(), jsonStr.end());

        try
        {
            socket.send(msg, zmq::send_flags::none);
        }
        catch (const zmq::error_t& e)
        {
            ShowError("ChatLogger ZMQ send failed: %s\n", e.what());
        }
    }

    void chatutils::logSystem(CCharEntity* PChar, const std::string& message, CHAT_MESSAGE_TYPE type)
    {
        // Reuse the existing ZMQ logic
        log(PChar, message.c_str(), static_cast<uint8>(type));
    }
} // namespace chatutils
