#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath> 

using namespace std;

int main() {
    //読み込み　これは毎回変える必要ある
    ifstream file("to_photo1.csv");
    string line;
    vector<int> result;  
    int sum = 0;

    //ファイル読み込みで1行目のみ取得、色はいらない
    if (file.is_open()) {
        while (getline(file, line)) {
            stringstream ss(line);
            string temp;
            int number;

            if (getline(ss, temp, ',')) {
                number = stoi(temp);
                sum += number;
                result.push_back(number);  
            }
        }
        file.close();
    }
    else {
        cout << "Unable to open file" << endl;
        return 1; 
    }

    cout << "Sum of numbers: " << sum << endl;
    double entropy = 0.0;

    // エントロピーの計算
    for (const auto& num : result) {
        //割合計算
        double probability = static_cast<double>(num) / sum;
        if (probability > 0) {//エラー消し
            entropy -= probability * log2(probability);
        }
    }

    cout << "Entropy: " << entropy << endl;

    return 0;
}
