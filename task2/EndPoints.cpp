//
// Created by pultak on 05.04.22.
//


#include "EndPoints.h"



void EndPoints::helloWorld(std::string& result){
    result = std::string("Ahoj svete!!!");
}
void EndPoints::getAllEndPoints(std::string& result){
    result = std::string("Poskytuji nekolik pohledu! \n"
                         "Patri mezi ne nasledujici: \n\n"
                         "/hello-world/\n"
                         "/end-points/\n"
                         "/info/\n"
                         "/xxx/");
}
void EndPoints::getInfo(std::string& result){
    result = std::string("Tohle je jednoduchy server odpovidajici pouze na HTTP GET Requesty!");
}
void EndPoints::getTopSecret(std::string& result){
    result = std::string("Zde pornografii nenaleznete!");

}