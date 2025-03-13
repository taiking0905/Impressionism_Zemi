#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <iomanip>
#include <sstream>
#include <utility>
#include <string>
#include <fstream>

using namespace std;
using namespace cv;

// opencvをダウンロードしたファイルの名前。
#ifdef _DEBUG
#pragma comment(lib, "opencv_world490d.lib")
#else
#pragma comment(lib, "opencv_world490.lib")
#endif

// CSVファイルを書き込む関数
void saveToCSV(const string& filename, const vector<pair<int, string>>& result) {
    ofstream file(filename);

    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    for (const auto& pair : result) {
        file << pair.first << "," << pair.second << endl;
    }

    file.close();
    if (!file) {
        cerr << "Failed to close file: " << filename << endl;
    }
}

int main()
{
    // 表示するときのウィンドウ名
    const char* windowName = "image";
    // 画像の場所
    Mat img = imread("C:\\Users\\T122115\\Desktop\\to_photo5.png");
    string CSV = "to_photo5.csv";

    // エラー処理
    if (img.empty())
    {
        cerr << "画像の読み込み失敗" << endl;
        return -1;
    }

    // 画像を1次元配列に変換する
    size_t numPixels = static_cast<size_t>(img.rows) * static_cast<size_t>(img.cols);
    vector<string> data(numPixels);

    int k = 0;
    for (int i = 0; i < img.rows; i++) {
        for (int j = 0; j < img.cols; j++) {
            // i,jのBGR全てを取得する
            int blue = img.at<Vec3b>(i, j)[0];
            int green = img.at<Vec3b>(i, j)[1];
            int red = img.at<Vec3b>(i, j)[2];
            // BGRを一つにまとめる
            stringstream ss;
            ss << hex << setw(2) << setfill('0') << (blue & 0xff)
                << hex << setw(2) << setfill('0') << (green & 0xff)
                << hex << setw(2) << setfill('0') << (red & 0xff);
            // 配列に入れる
            data[k] = ss.str();
            k++;
        }
    }

    // dataを昇順にソート[赤、赤、青、青]みたいに色の文字列をソートした
    sort(data.begin(), data.end());

    // result[int,string]という配列をpairを使い作る　文字と数字だから
    vector<pair<int, string>> result;
    // 初期値を代入
    result.push_back(make_pair(1, data[0]));

    // resultベクターの更新
    for (int i = 1; i < img.rows * img.cols; ++i) {
        // dataとさっき入れた値のsecond[色があるところ]と比べて同じなら　例　赤、赤
        if (data[i] == result.back().second) {
            // secondをインクリメント
            result.back().first++;
        }
        else {
            // 違ったら、新しく追加する　例　赤、青
            result.push_back(make_pair(1,data[i]));
        }
    }
    sort(result.rbegin(), result.rend());//ソートこれ色の割合がで大きい順になっている
 
    // 結果の出力
    for (const auto& pair : result) {
        cout << "Count: " << pair.first << ", Color: " << pair.second << endl;
    }

    // CSVファイルに結果を保存
    saveToCSV(CSV, result);

    // 画像を表示
    imshow(windowName, img);
    waitKey(0);

    return 0;
}


