#include <iostream>
#include "json.hpp"
#include "apifetcher.h"

#define ISS_IP "138.68.39.196"
#define ISS_HOST "api.open-notify.org"
#define ISS_PORT 80
#define ISS_ENDPOINT "/iss-now.json"

#define SS_IP "45.33.59.78"
#define SS_HOST "api.sunrise-sunset.org"
#define SS_PORT 80
#define SS_ENDPOINT(latitude, longitude) "/json?lat=" + latitude + "&lng=" + longitude + "&formatted=0"

#define TIMESTAMP_IP "13.79.230.33"
#define TIMESTAMP_HOST "showcase.api.linx.twenty57.net"
#define TIMESTAMP_PORT 80
#define TIMESTAMP_ENDPOINT(timestamp) "/UnixTime/fromunixtimestamp?unixtimestamp=" + timestamp +


#define GET_REQUEST(ENDPOINT, HOST) \
"GET " ENDPOINT " HTTP/1.1\n" \
"Host: "  HOST "\n" \
"Connection: close\n" \
"Upgrade-Insecure-Requests: 1\n\n"


std::string tsFetch(const std::string& issStamp){
    auto tsGet = std::string(GET_REQUEST(TIMESTAMP_ENDPOINT(issStamp), TIMESTAMP_HOST));
    ApiFetcher ts{TIMESTAMP_PORT, TIMESTAMP_IP, tsGet};
    std::string tsResult{};
    if(!ts.fetchData(tsResult)){
        std::cout << "TS fetch failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Received from Timestamp API:" << std::endl;
    std::cout << tsResult << std::endl;
    std::cout << std::endl << std::endl;

    auto tsJson = nlohmann::json::parse(tsResult);
    std::string datetime = tsJson["Datetime"];
    return datetime;
}

std::tuple<std::string, std::string> ssFetch(const std::string& lat, const std::string& lng){

    auto ssGet = std::string(GET_REQUEST(SS_ENDPOINT(lat, lng), SS_HOST));
    ApiFetcher ss{SS_PORT, SS_IP, ssGet};
    std::string ssResult{};
    if(!ss.fetchData(ssResult)){
        std::cout << "SS fetch failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Received from Sunrise Sunset:" << std::endl;
    std::cout << ssResult << std::endl;
    std::cout << std::endl << std::endl;

    if (ssResult.empty()) {
        std::cout << "No data received from SS server\n";
        exit(EXIT_FAILURE);
    }
    auto ssJson = nlohmann::json::parse(ssResult);

    std::string sunrise = ssJson["results"]["sunrise"];
    std::string sunset = ssJson["results"]["sunset"];

    //TODO simplify?

    return std::make_tuple(sunrise, sunset);
}

std::tuple<std::string, std::string, long> issFetch(){
    auto issGet = std::string(GET_REQUEST(ISS_ENDPOINT, ISS_HOST));
    ApiFetcher iss{ISS_PORT, ISS_IP, issGet};
    std::string issResult{};
    if(!iss.fetchData(issResult)){
        std::cout << "ISS fetch failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Received from International Space Station :" << std::endl;
    std::cout << issResult << std::endl;
    std::cout << std::endl << std::endl;

    if (issResult.empty()) {
        std::cout << "No data received from ISS server!\n";
        exit(EXIT_FAILURE);
    }
    auto issJson = nlohmann::json::parse(issResult);

    std::string lat = issJson["iss_position"]["latitude"];
    std::string lng = issJson["iss_position"]["longitude"];
    long timestamp = issJson["timestamp"];
    return std::make_tuple(lat, lng, timestamp);
}


int main() {
    auto [lat, lng, issTimestamp] = issFetch();
    auto [sunset, sunrise] = ssFetch(lat, lng);
    auto strISSStamp = std::to_string(issTimestamp);
    auto dateTime = tsFetch(strISSStamp);

    return 0;
}
