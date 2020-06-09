#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <string>
#include <set>

namespace Protocol {
    enum : unsigned char {
        SOH = 1,        //START OF HEADING
        STX = 2,        //START OF TEXT
        ETX = 3,        //END OF TEXT
        EOT = 4,        //END OF TRANSMISSION
        ENQ = 5,        //ENQUIRY
        ACK = 6,        //ACKNOWLEDGE
        NAK = 21,       //NEGATIVE ACKNOWLEDGE
        SYN = 22,       //SYNCHRONOUS IDLE
        ETB = 23,       //END OF TRANSMISSION BLOCK
        LF = 10,        //LINE FEED
        CR = 13,        //CARRIAGE RETURN
        DONERECV = 18,  //DONE RECEIVING
        DONETRANS = 19, //DONE TRANSMISSION
        CL_REQ_ID = 50,
        CL_REQ_AUTH = 51,
        CL_REQ_FILE = 52,
        CL_REQ_PARAM = 53,
        CL_REQ_COMMAND = 54,
        CL_SND_PARAM = 100,
        CL_SND_ID = 101,
        CL_SND_PRINT = 102,
        SRV_SND_COMMAND = 120,
        SRV_SND_TEXT = 121,
        SRV_SND_PART = 122,
        USR_CMD = 123,
    };

    const int BUFSIZE = 1024;

    inline std::string createRequest(unsigned char head, const std::string& payload = "") {
        std::string result;
        result += SOH;
        result += head;
        result += STX;
        result += payload;
        return result;
    }

    inline bool parseResponse(const std::string& message, std::pair<char, std::string>& result) {
        size_t startHead = message.find_first_of(SOH);
        size_t startText = message.find_first_of(STX);

        if (startHead == std::string::npos || startText == std::string::npos)
            return false;

        //Parts of message
        result.first = message.substr(startHead + 1, startHead + 2).at(0);
        result.second = message.substr(startText + 1);

        return true;
    }
}


#endif //PROTOCOL_H
