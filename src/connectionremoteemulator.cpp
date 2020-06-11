#include "connectionremoteemulator.h"
#include <Mmdeviceapi.h>
#include <Endpointvolume.h>

ConnectionRemoteEmulator::ConnectionRemoteEmulator(int fd) : Connection(fd)
{

}

void ConnectionRemoteEmulator::processMessage(const std::pair<uint8_t, std::string>& message)
{
    char command         = message.first;
    std::string text     = message.second;

    std::string response;

    switch (command) {
    case Protocol::CL_REQ_COMMAND:
        std::cout << "Client requested command" << std::endl;
        response = popCommand();
        if (!response.size())
            response = "wait";
        Protocol::writeMessage(m_sock, Protocol::SRV_SND_COMMAND, response);
        break;
    case Protocol::CL_SND_ID:
        std::cout << "Client sent ID " << text << std::endl;
        m_id = text;
        Protocol::writeMessage(m_sock, Protocol::ACK);
        break;
    case Protocol::CL_REQ_ID:
        std::cout << "Client requested ID" << std::endl;
        Protocol::writeMessage(m_sock, Protocol::SRV_SND_TEXT, "testId");
        break;
    case Protocol::CL_REQ_FILE:
        std::cout << "Client requested file" << std::endl;
        Protocol::writeMessage(m_sock, Protocol::NAK);
        break;
    case Protocol::CL_SND_PRINT:
        std::cout << "Client sent printable info" << std::endl;
        std::cout << text << std::endl;
        Protocol::writeMessage(m_sock, Protocol::ACK);
        break;
    case Protocol::USR_CMD:
        if (text == "mute") {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = 0;
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
            input.ki.wScan=MapVirtualKey(VK_VOLUME_MUTE,0);
            input.ki.time = 0;
            input.ki.dwExtraInfo = 0;
            input.ki.wVk = VK_VOLUME_MUTE;
            SendInput(1,&input,sizeof(input));
            break;
        } else if (text == "vol_up") {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = 0;
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
            input.ki.wScan=MapVirtualKey(VK_VOLUME_UP,0);
            input.ki.time = 0;
            input.ki.dwExtraInfo = 0;
            input.ki.wVk = VK_VOLUME_UP;
            SendInput(1,&input,sizeof(input));
            break;
        } else if (text == "vol_down") {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = 0;
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
            input.ki.wScan=MapVirtualKey(VK_VOLUME_DOWN,0);
            input.ki.time = 0;
            input.ki.dwExtraInfo = 0;
            input.ki.wVk = VK_VOLUME_DOWN;
            SendInput(1,&input,sizeof(input));
            break;
        } else if (text == "play") {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = 0;
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
            input.ki.wScan=MapVirtualKey(VK_MEDIA_PLAY_PAUSE,0);
            input.ki.time = 0;
            input.ki.dwExtraInfo = 0;
            input.ki.wVk = VK_MEDIA_PLAY_PAUSE;
            SendInput(1,&input,sizeof(input));
            break;
        } else if (text == "space") {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = 0;
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
            input.ki.wScan=MapVirtualKey(VK_SPACE,0);
            input.ki.time = 0;
            input.ki.dwExtraInfo = 0;
            input.ki.wVk = VK_SPACE;
            SendInput(1,&input,sizeof(input));
            break;
        } else if (text.find("volume") != std::string::npos) {
            ULONG volume = 0;
            u_int64 pos = text.find(":");
            if (pos != std::string::npos) {
                volume = std::atoi(text.substr(pos + 1).c_str());
            }
            CoInitialize(NULL);
            changeAudioVolume((float)volume / (float)100);
            CoUninitialize();
            break;
        } else if (text == "sleep") {
            SetSuspendState(true, true, true);
            break;
        } else if (text.find("setMousePos") != std::string::npos) {
            int xPos;
            int yPos;
            uint64_t pos = text.find(":");
            if (pos != std::string::npos) {
                uint64_t posDelimiter = text.find(":", pos + 1);
                if (posDelimiter != std::string::npos) {
                    xPos = std::atoi(text.substr(pos + 1, text.size() - posDelimiter).c_str());
                    yPos = std::atoi(text.substr(posDelimiter + 1).c_str());
                    mouseMove(xPos, yPos);
                }
            }
            break;
        } else if (text == "mouseDown") {
            INPUT Input={0};
            Input.type      = INPUT_MOUSE;
            Input.mi.dwFlags  = MOUSEEVENTF_LEFTDOWN;
            SendInput(1,&Input,sizeof(INPUT));
            break;
        } else if (text == "mouseRelease") {
            INPUT Input={0};
            Input.type      = INPUT_MOUSE;
            Input.mi.dwFlags  = MOUSEEVENTF_LEFTUP;
            SendInput(1,&Input,sizeof(INPUT));
            break;
        } else if (text == "shutdown") {
            shutdown();
            break;
        }
        break;

    default:
        std::cout << "Unknown message" << std::endl;
        Protocol::writeMessage(m_sock, Protocol::NAK);
    }
}

bool ConnectionRemoteEmulator::changeAudioVolume(float level)
{
    HRESULT hr = NULL;
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if(FAILED(hr))
        return FALSE;

    IMMDevice *defaultDevice = NULL;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
    deviceEnumerator->Release();
    if(FAILED(hr))
        return FALSE;

    IAudioEndpointVolume *endpointVolume = NULL;
    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume),
                                 CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
    defaultDevice->Release();
    if(FAILED(hr))
        return FALSE;

    hr = endpointVolume->SetMasterVolumeLevelScalar(level, NULL);
    endpointVolume->Release();

    return SUCCEEDED(hr);
}

bool ConnectionRemoteEmulator::shutdown()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process.

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return( FALSE );

    // Get the LUID for the shutdown privilege.

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
                         &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process.

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                          (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
        return FALSE;

    // Shut down the system and force all applications to close.

    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,
                       SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                       SHTDN_REASON_MINOR_UPGRADE |
                       SHTDN_REASON_FLAG_PLANNED))
        return FALSE;

    //shutdown was successful
    return TRUE;
}

bool ConnectionRemoteEmulator::mouseMove(int x, int y)
{
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi.mouseData = 0;
    input.mi.time = 0;
    input.mi.dx = x / 2;//*(65536/GetSystemMetrics(SM_CXSCREEN));//x being coord in pixels
    input.mi.dy = y / 2;//*(65536/GetSystemMetrics(SM_CYSCREEN));//y being coord in pixels
    input.mi.dwFlags = MOUSEEVENTF_MOVE ;//| MOUSEEVENTF_VIRTUALDESK;// | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &input, sizeof(input));
    return true;
}
