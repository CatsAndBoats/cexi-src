#include "packets/chat_message.h"
#include "zone.h"
#include <cstdint>

class CCharEntity;

namespace chatutils
{
    void log(CCharEntity* PChar, const char* rawMessage, uint8_t messageType);
    void sendChatMessage(CCharEntity* PChar, uint8_t chatType, std::string_view message, uint8_t range = CHAR_INRANGE);
    void logSystem(CCharEntity* PChar, const std::string& message, CHAT_MESSAGE_TYPE type);
} // namespace chatutils
