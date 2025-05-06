#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <cctype>
#include <algorithm>
#include <map>
#include <iomanip>
#include <regex>

using std::string, std::vector, std::map;

bool IsDigit(string& var, int& amount) {
    for (char c : var) {
        if (!std::isdigit(c)) {
            return 1;
        }
    }
    amount = std::stoi(var);
    return 0;
}

bool IsTime(std::chrono::minutes& time, std::string& time_start, int& hours, int& minutes) {
    char colon;
    std::regex time_format(R"(^([0-9]{2}):([0-9]{2})$)");
    std::smatch match;


    if (std::regex_match(time_start, match, time_format)) {
        hours = std::stoi(match[1].str());
        minutes = std::stoi(match[2].str());

        if (hours >= 0 && hours < 24 && minutes >= 0 && minutes < 60) {
            time = std::chrono::hours(hours) + std::chrono::minutes(minutes);
        }
    }
    else {
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {

     if (argc < 2) {
         std::cout << "Add a file\n";
         return 1;
     }


    std::ifstream file(argv[1]);
    if(!file.is_open()){
        std::cout << "Bad file\n";
    }
    vector<string>res;
    /*std::ifstream file("file_2.txt");*/

    int amount_of_tables, money_per_h;
    string line;
    std::getline(file, line);

    if (IsDigit(line, amount_of_tables)) {
        std::cout << line;
        return 2;
    }

    std::string time_start_gl, time_end_gl;
    int hours, minutes;
    char colon;
    std::getline(file, line);
    std::istringstream line_stream(line);

    line_stream >> time_start_gl >> time_end_gl;
    if (line_stream.peek() != EOF) {
        std::cout << line;
        return 11;
    }

    std::chrono::minutes start, end;


    if (IsTime(start, time_start_gl, hours, minutes) || IsTime(end, time_end_gl, hours, minutes)) {
        std::cout << line;
        return 3;
    }


    std::getline(file, line);
    if (IsDigit(line, money_per_h)) {
        std::cout << line;
        return 2;
    }


    string clientName, ID, tableNumber;
    std::string time_start, time_end;
    std::chrono::minutes request_time;
    map<string, int> clientBase;
    map<int, bool> placesBase;
    std::map< size_t, string> waiters;
    vector<string> visitors;
    size_t waiter_num = 0;
    vector<int>pc_salary(amount_of_tables, 0);
    vector<int>pc_wasted_time(amount_of_tables, 0);
    vector<std::chrono::minutes>pc_working_time(amount_of_tables);


    res.emplace_back(time_start_gl);
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream line_stream(line);
        line_stream >> time_start >> ID >> clientName;


        if (ID == "2") line_stream >> tableNumber;
        if (line_stream.peek() != EOF) {
            std::cout << line << "\n";
            return 8;
        }
        int real_ID, real_tableNum;

        if (IsTime(request_time, time_start, hours, minutes) || IsDigit(ID, real_ID) || ID == "2" && IsDigit(tableNumber, real_tableNum)) {
            std::cout << line << "\n";
            return 4;
        }

        res.emplace_back(line);




        if (real_ID == 2 && real_tableNum > amount_of_tables) {
            return 10;
        }
        //std::cout << "\n";

        if (real_ID == 1) {
            if (request_time < start || request_time>end) {
                res.emplace_back(time_start + " 13 NotOpenYet");
                //std::cout << time_start << " 13 NotOpenYet\n";
                continue;
            }


            else if (clientBase.find(clientName) != clientBase.end()) {
                res.emplace_back(time_start + " 13 YouShallNotPass");
                //std::cout << time_start << " 13 YouShallNotPass\n";
                continue;
            }
            visitors.push_back(clientName);
        }
        else if (real_ID == 2) {

            if (request_time > end) {
                break;
            }

            if (placesBase.contains(real_tableNum) && placesBase[real_tableNum]) {
                res.emplace_back(time_start + " 13 PlaceIsBusy");
                // std::cout << time_start << " 13 PlaceIsBusy\n";
            }
            else if (std::find(visitors.begin(), visitors.end(), clientName) == visitors.end() && placesBase.contains(real_tableNum)) {
                if (!placesBase.contains(real_tableNum)) {
                    placesBase[clientBase[clientName]] = false;
                }

                auto time_diff = request_time - pc_working_time[clientBase[clientName] - 1];
                pc_wasted_time[clientBase[clientName] - 1] += static_cast<int>(time_diff.count());
                pc_working_time[clientBase[clientName] - 1] = request_time;

                pc_salary[clientBase[clientName] - 1] += static_cast<int>((std::ceil(time_diff.count() / 60.0)) * money_per_h);

                clientBase[clientName] = real_tableNum;
                placesBase[clientBase[clientName]] = true;

            }
            else if (std::find(visitors.begin(), visitors.end(), clientName) != visitors.end()) {
                if (placesBase.contains(real_tableNum) && placesBase[real_tableNum]) res.emplace_back(time_start + " 13 ClientUnknown");
                // std::cout << time_start << " 13 ClientUnknown\n";
                else {
                    if (std::find(visitors.begin(), visitors.end(), clientName) == visitors.end()) {
                        visitors.emplace_back(clientName);
                    }
                    clientBase[clientName] = real_tableNum;
                    placesBase[real_tableNum] = true;
                    pc_working_time[real_tableNum - 1] = request_time;
                }
            }
            else {
                clientBase[clientName] = real_tableNum;
                placesBase[real_tableNum] = true;
                pc_working_time[real_tableNum - 1] = request_time;
            }
        }
        else if (real_ID == 3) {
            if (request_time > end) {
                break;
            }
            bool targetValue = 0;
            auto it = std::ranges::find_if(placesBase,
                [&targetValue](const auto& pair) {
                    return pair.second == targetValue;
                });

            if (it != placesBase.end() || placesBase.size() < amount_of_tables) {
                res.emplace_back(time_start + " 13 ICanWaitNoLonger!");
                // std::cout<<time_start <<" 13 ICanWaitNoLonger!\n";
            }
            else {
                if (waiters.size() < amount_of_tables) {
                    waiters[waiter_num] = clientName;
                }
                else {
                    res.emplace_back(time_start + "11 " + clientName);
                    //std::cout << time_start << " 11 " << clientName << "\n";
                }
            }
        }
        else if (real_ID == 4) {
            if (request_time > end) {
                break;
            }
            if (clientBase.find(clientName) == clientBase.end()) {
                res.emplace_back(time_start + " 13 ClientUnknown");
                //std::cout << time_start << " 13 ClientUnknown\n";
            }
            else {
                visitors.erase(std::remove(visitors.begin(), visitors.end(), clientName), visitors.end());

                auto time_diff = request_time - pc_working_time[clientBase[clientName] - 1];
                pc_wasted_time[clientBase[clientName] - 1] += static_cast<int>(time_diff.count());


                pc_salary[clientBase[clientName] - 1] += static_cast<int>((std::ceil(time_diff.count() / 60.0)) * money_per_h);

                if (waiters.empty()) {
                    placesBase[clientBase[clientName]] = false;
                }
                else {
                    clientBase.emplace(waiters[waiter_num], clientBase[clientName]);



                    pc_working_time[clientBase[clientName] - 1] = request_time;
                    res.emplace_back(time_start + " 12 " + waiters.at(waiter_num) + " " + std::to_string(clientBase.at(clientName)));
                    //std::cout << time_start << " 12 " <<  waiters[waiter_num] << " " <<clientBase[clientName] << "\n";
                    waiters.erase(waiter_num);
                    ++waiter_num;

                }
                clientBase.erase(clientName);
            }
        }

        else {
            std::cout << line;
            return 6;
        }


    }
    for (const auto& it : res)
    {
        std::cout << it << "\n";
    }
    for (const auto& it : visitors) {
        std::cout << time_end_gl << " 11 " << it << "\n";
    }
    for (const auto& it : clientBase) {
        int kk = it.second;
        clientName = it.first;
        auto time_diff = end - pc_working_time[kk - 1];
        pc_wasted_time[clientBase[clientName] - 1] += static_cast<int>(time_diff.count());
        pc_salary[clientBase[clientName] - 1] += static_cast<int>((std::ceil(time_diff.count() / 60.0)) * money_per_h);
    }
    std::cout << time_end_gl;


    for (int i = 0; i < pc_wasted_time.size(); ++i)
    {
        int hours = pc_wasted_time[i] / 60;
        int minutes = pc_wasted_time[i] % 60;
        std::cout << "\n" << i +1 << " "<< pc_salary[i] << " "<<std::setw(2) << std::setfill('0')<<hours<<":"<<std::setw(2)<<std::setfill('0')<<minutes;
    }
    file.close();
    return 0;
}