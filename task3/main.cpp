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
#define SS_DATE_FORMAT "%Y-%m-%dT%H:%M:%S"


#define TIMESTAMP_IP "13.79.230.33"
#define TIMESTAMP_HOST "showcase.api.linx.twenty57.net"
#define TIMESTAMP_PORT 80
#define TIMESTAMP_ENDPOINT(timestamp) "/UnixTime/fromunixtimestamp?unixtimestamp=" + timestamp +
#define TIMESTAMP_DATE_FORMAT "%Y-%m-%d %H:%M:%S"

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

    return std::make_tuple(sunset, sunrise);
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


auto stringTime2Long(const char* format, const std::string& stringData){
    std::tm tm = {};
    std::stringstream ss(stringData);
    ss >> std::get_time(&tm, format);
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto epoch = std::chrono::time_point_cast<std::chrono::milliseconds>(tp).time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
}

int main() {
    auto [lat, lng, issTimestamp] = issFetch();
    std::cout << "__________________________________________________________________" << std::endl;
    auto [sunset, sunrise] = ssFetch(lat, lng);
    std::cout << "__________________________________________________________________" << std::endl;
    auto strISSStamp = std::to_string(issTimestamp);
    auto actualTime = tsFetch(strISSStamp);
    std::cout << "__________________________________________________________________" << std::endl;

    std::cout << "Actual time: " << actualTime << std::endl;
    std::cout << "Actual sunrise time: " << sunrise << std::endl
    << "Actual sunset time: " << sunset;

    /*auto actualTime = "2022-04-09 12:52:58";
    auto sunrise = "2022-04-09T00:49:42+00:00";
    auto sunset = "2022-04-09T13:44:29+00:00";*/

    auto sunsetMs = stringTime2Long(SS_DATE_FORMAT, sunset);
    auto sunriseMs = stringTime2Long(SS_DATE_FORMAT, sunrise);
    auto actualMs = stringTime2Long(TIMESTAMP_DATE_FORMAT, actualTime);

    std::cout << std::endl;

    long ms2Sunrise = 0;
    long ms2Sunset = 0;
    bool pastSunriseTime = false;
    if(actualMs > sunriseMs){
        pastSunriseTime = true;
        ms2Sunrise = sunriseMs - (actualMs - 24 * 3600000);
    }else{
        pastSunriseTime = false;
        //add one day
        ms2Sunrise = sunriseMs - actualMs;
    }

    bool pastSunsetTime = false;
    if(actualMs < sunsetMs){
        pastSunsetTime = true;
        ms2Sunset = sunsetMs - (actualMs + 24 * 3600000);
    }else{
        pastSunsetTime = false;
        ms2Sunset = sunsetMs - actualMs;
    }
    auto hoursBeforeSunrise = (double)ms2Sunrise / 3600000.0;
    auto hoursAfterSunset = (double)ms2Sunset / 3600000.0;


    //hoursBeforeSunrise = (hoursBeforeSunrise < 0 ? 24 + hoursBeforeSunrise : hoursBeforeSunrise);
    hoursBeforeSunrise = hoursBeforeSunrise;
    //hoursAfterSunset = (hoursAfterSunset < 0 ? 24 + hoursAfterSunset : hoursAfterSunset);
    hoursAfterSunset = hoursAfterSunset * -1;

    std::cout << "Remaining hours to next sunrise in current place: " << hoursBeforeSunrise << std::endl;
    std::cout << "Elapsed hours from last sunset in current place: " << hoursAfterSunset << std::endl;
    std::cout << std::endl;

    std::cout << "__________________________________________________________________" << std::endl;
    if((hoursAfterSunset > 12) || (hoursBeforeSunrise > 12 && hoursBeforeSunrise < 0)){
        std::cout << "ISS is currently on the illuminated side of the earth.";
    }else{
        std::cout << "ISS is currently on the dark side of the earth.";
    }
    std::cout << std::endl;

    if(hoursAfterSunset <= 2 && hoursAfterSunset >= 1){
        std::cout << "Ideal conditions to observer the ISS (" << hoursAfterSunset << " hours after sunset)";
    }else if(hoursBeforeSunrise <= 2 && hoursBeforeSunrise >= 1){
        std::cout << "Ideal conditions to observer the ISS (" << hoursBeforeSunrise << " hours before sunset)";
    }else{
        std::cout << "Currently there are non ideal conditions to observer ISS";
    }
    std::cout << std::endl;
    std::cout << "__________________________________________________________________";
    std::cout << std::endl;
    return 0;
}
