#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <string>
#include <iostream>
#include <set>
#include <array>

#ifdef __linux__
#include <dlfcn.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#define SOCKET_ERROR (-1)
#elif _WIN32
#include <winsock2.h>
#include <mswsock.h>
#endif

namespace Protocol {

constexpr size_t BUFSIZE = 1024;
constexpr size_t MAX_MESSAGE_SIZE = sizeof (uint16_t);

using byte = uint8_t;

template< typename T > std::array< byte, sizeof(T) >  to_bytes( const T& object )
{
    std::array< byte, sizeof(T) > bytes ;

    const byte* begin = reinterpret_cast< const byte* >( std::addressof(object) ) ;
    const byte* end = begin + sizeof(T) ;
    std::copy( begin, end, std::begin(bytes) ) ;

    return bytes ;
}

template< typename T >
T& from_bytes( const std::array< byte, sizeof(T) >& bytes, T& object )
{
    // http://en.cppreference.com/w/cpp/types/is_trivially_copyable
    static_assert( std::is_trivially_copyable<T>::value, "not a TriviallyCopyable type" ) ;

    byte* begin_object = reinterpret_cast< byte* >( std::addressof(object) ) ;
    std::copy( std::begin(bytes), std::end(bytes), begin_object ) ;

    return object ;
}

    enum : uint8_t {
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
        CL_SND_PART = 103,
        SRV_SND_COMMAND = 120,
        SRV_SND_TEXT = 121,
        SRV_SND_PART = 122,
        USR_CMD = 123,
    };

    enum class ERRORS : uint8_t {SUCCESS = 0,
                 WRONG_COMMUNICATION,
                 WRONG_MESSAGE,
                };

    inline std::string createRequest(uint8_t head, const std::string& body = "") {
        std::string msg;
        msg += SOH;
        msg += head;

        uint16_t size = body.size();
        auto serialized = to_bytes(size);
        msg += serialized[0];
        msg += serialized[1];
        msg += STX;
        msg += body;
        return msg;
    }

    inline ERRORS writeMessage(uint16_t socket, uint8_t head, std::string body = "") {
        std::string msg;

        msg = createRequest(head, body);

        int res = send(socket, msg.c_str(), msg.size(), 0);
        if (res == 0 || res == SOCKET_ERROR) {
            return ERRORS::WRONG_COMMUNICATION;
        }
        return ERRORS::SUCCESS;
    }

    inline ERRORS readMessage(uint16_t socket, std::pair<uint8_t, std::string>& result) {

        //Message structure SOH(1)_HEAD(1)_BODY_SIZE(2)_STX(1)_BODY

        char buf[Protocol::BUFSIZE + 1];
        std::string message = "";

        int res = 0;
        //Read HEAD, per byte
        for (size_t i = 0; i < 5; ++i) {
            char tmp[2];
            res = recv(socket, tmp, 1, 0);
            if (res == 0 || res == SOCKET_ERROR) {
                return ERRORS::WRONG_COMMUNICATION;
            }
            buf[i] = tmp[0];
        }
        buf[5] = '\0';

        //Check read and head structure
        if (buf[0] != Protocol::SOH || buf[4] != Protocol::STX) {
            return ERRORS::WRONG_MESSAGE;
        }

        //Parse head, get body length
        uint16_t bodySize;
        std::array<uint8_t, 2> serialized;

        serialized[0] = buf[2];
        serialized[1] = buf[3];
        from_bytes(serialized, bodySize);

        result.first = static_cast<uint8_t>(buf[1]);

        //Read body
        if (bodySize != 0) {
            uint16_t bytesLeft = bodySize;
            while (bytesLeft > 0) {
                res = 0;
                if (bytesLeft > Protocol::BUFSIZE)
                    res = recv(socket, buf, BUFSIZE, 0);
                else
                    res = recv(socket, buf, bytesLeft, 0);

                if (res == 0 || res == SOCKET_ERROR)
                    return ERRORS::WRONG_COMMUNICATION;

                buf[res] = '\0';
                bytesLeft -= res;

                result.second += std::string(buf, 0, res);
            }
        }

        return ERRORS::SUCCESS;
    }
}

#endif //PROTOCOL_H
