#include <iostream>
#include <cstring>
#include <cstdint>
#include <arpa/inet.h>
#include <sys/socket.h>

#define ERR_socket_create "UDP socket init failed!"
#define ERR_socket_reuse "Failed to reuse same socket!"
#define ERR_socket_interface "Failed to init interface for the socket!"
#define ERR_socket_bind "Failed to bind socket!"
#define ERR_loop_back "Failed to disable loop backs!"
#define ERR_multicast_group "Failed to init multicast group!"

#define SSDP_MULTICAST_ADDRESS "239.255.255.250"
#define SSDP_PORT 1900
#define SSDP_DISCOVERY_STRING \
"M-SEARCH * HTTP/1.1\r\n" \
"HOST: 239.255.255.250:1900\r\n"\
"MAN: \"ssdp:discover\"\r\n"\
"MX: 2\r\n"\
"ST: ssdp:all\r\n\r\n"

#define BUFFER_SIZE 1000


enum InitResponse{
    SocketCreateErr = 1,
    SocketReuseErr = 2,
    LoopBackErr = 3,
    SocketInterfaceErr = 4,
    SocketBindErr = 5,
    MulticastGroupErr = 6,
    Ok = 0
};

struct ConnectionInfo{
    struct sockaddr_in deviceAddress {}; // used for receiving messages from devices
    struct sockaddr_in groupAddress {};  // init of the multicast group
    struct sockaddr_in localAddress {};  // port & ip the socket will be bound on
    struct in_addr localInterface {};    // local interface the socket will send its datagrams on
    struct ip_mreq group {};             // joining the multicast group (ip of the group + ip of the interface)
    int udpSocket{};
    uint32_t timeout{};
    char* inetAddress{};
};

/***
 *  This function initializes everything needed for our connection
 * @param config
 * @return
 */
InitResponse initConnection(struct ConnectionInfo &config){

    //INIT UDP socket
    config.udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(config.udpSocket == -1){
        return InitResponse::SocketCreateErr;
    }

    int reuse = 1;
    //reuse same packets
    int sockRes = setsockopt(config.udpSocket, SOL_SOCKET, SO_REUSEADDR,
                             reinterpret_cast<char *>(&reuse), sizeof(reuse));
    if(sockRes == -1){
        return InitResponse::SocketReuseErr;
    }

    char loopBack = 0;
    //disable loop backs -> filter our packets
    sockRes = setsockopt(config.udpSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
                         reinterpret_cast<char *>(&loopBack), sizeof(loopBack));
    if(sockRes == -1){
        return InitResponse::LoopBackErr;
    }

    //INIT Addresses
    config.groupAddress.sin_family = AF_INET;
    config.groupAddress.sin_addr.s_addr = inet_addr(SSDP_MULTICAST_ADDRESS);
    config.groupAddress.sin_port = htons(SSDP_PORT);

    config.localInterface.s_addr = inet_addr(config.inetAddress);

    config.localAddress.sin_family = AF_INET;
    config.localAddress.sin_port = htons(SSDP_PORT);
    config.localAddress.sin_addr.s_addr = INADDR_ANY;

    // Tell the socket which interface to send its multicast packets on.
    sockRes = setsockopt(config.udpSocket, IPPROTO_IP, IP_MULTICAST_IF,
                         reinterpret_cast<char *>(&(config.localInterface)), sizeof(config.localInterface));
    if (sockRes == -1) {
        return InitResponse::SocketInterfaceErr;
    }

    // Bind the UDP socket to a specific IP address & port.
    int bindRes = bind(config.udpSocket, reinterpret_cast<struct sockaddr *>
            (&(config.localAddress)), sizeof(config.localAddress));
    if (bindRes == -1) {
        return InitResponse::SocketBindErr;
    }

    // Join the multicast group on a specified interface.
    config.group.imr_multiaddr.s_addr = inet_addr(SSDP_MULTICAST_ADDRESS);
    config.group.imr_interface.s_addr = inet_addr(config.inetAddress);

    if (setsockopt(config.udpSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<char *>(&(config.group)), sizeof(config.group)) == -1) {
        return InitResponse::MulticastGroupErr;
    }

    return InitResponse::Ok;
}

/***
 * This function sends the SSDP discovery packet
 * @param config
 * @return
 */
int sendDiscoveryPacket(struct ConnectionInfo &config){

    std::cout << "Sending discovery packet!" << std::endl;
    //SSDP discovery packet.
    ssize_t sentBytes = sendto(config.udpSocket, SSDP_DISCOVERY_STRING,
                               strlen(SSDP_DISCOVERY_STRING), 0,
                               reinterpret_cast<struct sockaddr *>(&(config.groupAddress)),
                                       sizeof(config.groupAddress));

    return sentBytes == -1 || sentBytes == 0;
}

int startListening(struct ConnectionInfo &config){
    if(sendDiscoveryPacket(config)){
        std::cout << "Failed to send discovery packet!" << std::endl;
        return 1;
    }

    char buffer[BUFFER_SIZE];
    fd_set sockets;
    FD_ZERO(&sockets);

    socklen_t deviceAddressLength = sizeof(config.deviceAddress);

    time_t start = time(nullptr);

    std::cout << "Starting listening!" << std::endl << std::endl;
    while ((time(nullptr) - start) < config.timeout) {
        FD_SET(config.udpSocket, &sockets);
        timeval socketTimeout{0, 5000};
        if(select(FD_SETSIZE, &sockets, nullptr, nullptr, &socketTimeout) == -1){
            std::cout << "Error occurred during file descriptor selection!" << std::endl;
            return 1;
        }

        // Check if there's data to be read from the socket
        if (FD_ISSET(config.udpSocket, &sockets)) {
            auto recCount = recvfrom(config.udpSocket, buffer, BUFFER_SIZE - 1, 0,
                                     reinterpret_cast<struct sockaddr *>(&config.deviceAddress), &deviceAddressLength);
            if (recCount == -1) {
                std::cout << "Error occurred during message reading!" << std::endl;
                return 1;
            }else{
                std::cout << "Received this message:" << std::endl;
                buffer[recCount] = '\0';

                std::cout << buffer << std::endl << std::endl;
            }
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if(argc < 3){
        std::cout << "Not enough parameters passed!" << std::endl;
        return 1;
    }

    char* inetAddr = argv[1];
    uint32_t timeout = std::stoi(argv[2]);

    if(timeout <= 0){
        std::cout << "Please enter valid timeout!" << std::endl;
        return 1;
    }

    struct ConnectionInfo config {};
    config.inetAddress = inetAddr;
    config.timeout = timeout;

    std::cout << "Configuration begin" << std::endl;
    switch(initConnection(config)){
        case Ok:
            break;
        case SocketCreateErr:
            std::cout << ERR_socket_create << std::endl;
            return 1;
        case SocketReuseErr:
            std::cout << ERR_socket_reuse << std::endl;
            return 1;
        case LoopBackErr:
            std::cout << ERR_loop_back << std::endl;
            return 1;
        case SocketInterfaceErr:
            std::cout << ERR_socket_interface << std::endl;
            return 1;
        case SocketBindErr:
            std::cout << ERR_socket_bind << std::endl;
            return 1;
        case MulticastGroupErr:
            std::cout << ERR_multicast_group << std::endl;
            return 1;
    };

    std::cout << "Config done" << std::endl << std::endl;

    return startListening(config);
}
