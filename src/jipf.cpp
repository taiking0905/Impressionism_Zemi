#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <limits>
#include <boost/math/special_functions/gamma.hpp>
#include <chrono>

using namespace std;
using namespace chrono;

//補助関数使用回数
int globalCount = 0;

//主関数　べき乗分布と指数分布を組み合わせた形
double model(double s_i, double A, double b, double T) {
    return A / (pow(s_i, b)) * exp(-s_i / T);
}

//補助関数　不完全ガンマ関数を採用　iの値が大きいと、誤差が大きくなってしまうからこれを使うらしい
double model_M(double s_i, double A, double b, double T) {
    return (A / s_i) * boost::math::tgamma(1 - b, s_i / T);
}
//lossの計算
double loss(const vector<double>& s, double A, double b, double T, const vector<double>& N_s) {
    double sum = 0.0;
    double best_N = 0.0;
    double best_M = 0.0;

    for (size_t i = 0; i < s.size(); i++) {
        best_N = model(s[i], A, b, T);
        best_M = model_M(s[i], A, b, T);

        //主か補助かどっちを使うか
        if (pow(N_s[i] - best_N, 2) > pow(N_s[i] - best_M, 2)) {
            best_N = best_M;
            globalCount++;
        }

        //最小二乗法でlossを計算
        sum += pow(N_s[i] - best_N, 2);
    }
    return sum;
}
// lossみたいに結果1つの結果の指標MAPE(平均絶対パーセント誤差)
double MAPE(const vector<double>& s, double A, double b, double T, const vector<double>& N_s) {
    double sum = 0.0;
    double best_N = 0.0;

    for (size_t i = 0; i < s.size(); i++) {
        //ベストな値で実行
        best_N = model(s[i], A, b, T);
        //その時のMAPEを計算
        sum += abs((N_s[i] - best_N) / N_s[i]);
    }
    return  sum / s.size() * 100;
}

int main() {
    //これを毎回変える
    string realcsv = "to_photo1.csv";
    //こっちは変えない。best値があるファイルを作成しておく
    string bestcsv = "bestcsv.csv";

    ifstream file(realcsv);
    string line;
    vector<double> s;
    vector<double> N_s;
    int index = 1;

    double current_loss;
    //初期値を大きくしておく
    double min_loss = numeric_limits<double>::max();
    vector<double> best_params(3);

    //realcsv読み込み　1行めのみ
    if (file.is_open()) {
        while (getline(file, line)) {
            stringstream ss(line);
            string temp;
            int number;

            if (getline(ss, temp, ',')) {
                number = stoi(temp);
                N_s.push_back(number);
                s.push_back(index++);
            }
        }
        file.close();
    }
    else {//例外処理
        cout << "Unable to open file" << endl;
        return 1;
    }

    //パラメータの初期値　外れ値があるときT_maxがmaxになっている。
    //Aは最大値の値に近い。ので N_s[0]を使っている
    //bは傾きで、bが1.0以上はエラーが出るので、注意
    double T_min = 10.0, T_max = 500.0;
    double A_min = 10.0, A_max = N_s[0] + 10000;
    double b_min = 0.01, b_max = 1.0;
    int steps = 4;

    for (int step = 0; step < steps; step++) {
        auto start = high_resolution_clock::now();
        //学習開始　繰り返しの回数は、予測回数が上がるごとに大きくなる用設定した
        for (double b = b_min; b <= b_max; b += (b_max - b_min + 1) / (50 + 10 * step)) {
            for (double A = A_min; A <= A_max; A += (A_max - A_min + 1) / (50 + 10 * step)) {
                for (double T = T_min; T <= T_max; T += (T_max - T_min + 1) / (50 + 10 * step)) {
                    current_loss = loss(s, A, b, T, N_s);
                    if (current_loss < min_loss) {
                        min_loss = current_loss;
                        best_params = { A, b, T };
                    }
                }
            }
        }
        //時間測定
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);

        cout << "Best Parameters step:" << step + 1 << endl
            << "A = " << best_params[0]
            << ", b = " << best_params[1]
            << ", T = " << best_params[2]
            << ", Loss = " << min_loss
            << ", Duration = " << duration.count() << " ms" << endl;

        //パラメータの更新　予測回数が上がるごとに初期値を小さくすることで、深い数値まで計算する
        T_min = best_params[2] * (0.5 + (double)(step - 1) / 10);
        T_max = best_params[2] * (1.5 - (double)(step - 1) / 10);
        A_min = best_params[0] * (0.5 + (double)(step - 1) / 10);
        A_max = best_params[0] * (1.5 - (double)(step - 1) / 10);
        b_min = best_params[1] * (0.5 + (double)(step - 1) / 10);
        b_max = best_params[1] * (1.5 - (double)(step - 1) / 10);

        //エラー回避
        if (b_max > 1) {
            b_max = 1;
        }

        //計算がうまくいっているか表示
        //cout <<"T_max:" << T_max << ",T_min:" << T_min << ",A_max:" << A_max << ",A_min:" << A_min << ",b_max:" << b_max << ",b_min:" << b_min << ",補助関数使用回数:" << globalCount << endl;
        globalCount = 0;
    }

    double test = MAPE(s, best_params[0], best_params[1], best_params[2], N_s);

    cout << endl << "MAPE:" << test<<endl;

    // bestcsv.csvに記入
    ofstream outputFile("bestcsv.csv");
    if (outputFile.is_open()) {
        for (size_t i = 0; i < s.size(); i++) {
            double predicted_N = model(s[i], best_params[0], best_params[1], best_params[2]);
            outputFile  << predicted_N << endl;
        }
        outputFile.close();
        cout << "Best model predictions written to bsxtcsv.csv" << endl;
    }
    else {//例外処理
        cout << "Unable to open bestcsv.csv for writing." << endl;
    }

    cout << "Best Parameters: A = " << best_params[0]
        << ", b = " << best_params[1]
        << ", T = " << best_params[2]
        << ", Loss = " << min_loss << endl;

    // GNUplotを起動
    FILE* gnuplotPipe = _popen("gnuplot -persist", "w");
    if (!gnuplotPipe) {
        std::cerr << "Error: Could not open pipe to GNUplot." << std::endl;
        return 1;
    }

    // ファイル名をGNUplotコマンドに埋め込む
    fprintf(gnuplotPipe, "set datafile separator ','\n");
    fprintf(gnuplotPipe, "set terminal qt size 1500, 800\n");
    fprintf(gnuplotPipe, "set multiplot layout 1,2 title 'Data Visualization with Linear and Log-Log Scales'\n");

    // 線形スケールプロット
    fprintf(gnuplotPipe, "set title 'Linear Scale'\n");
    fprintf(gnuplotPipe, "set xlabel 'Row Number (Line Index)'\n");
    fprintf(gnuplotPipe, "set ylabel 'Values in Column 1'\n");
    fprintf(gnuplotPipe, "unset logscale\n");
    fprintf(gnuplotPipe, "plot '%s' using ($0+1):1 with points pointtype 7 pointsize 1 lc rgb 'blue' title 'Column 1 Data',\\\n", realcsv.c_str());
    fprintf(gnuplotPipe, "     '%s' using ($0+1):1 with lines linewidth 2 linecolor rgb 'red' title 'Column'\n", bestcsv.c_str());

    // 対数スケールプロット
    fprintf(gnuplotPipe, "set logscale x\n");
    fprintf(gnuplotPipe, "set logscale y\n");
    fprintf(gnuplotPipe, "set title 'Log-Log Scale'\n");
    fprintf(gnuplotPipe, "set xlabel 'log(Row Number)'\n");
    fprintf(gnuplotPipe, "set ylabel 'log(Values in Column 1)'\n");
    fprintf(gnuplotPipe, "plot '%s' using ($0+1):1 with points pointtype 7 pointsize 1 lc rgb 'blue' title 'Column 1 Data (Log-Log)',\\\n", realcsv.c_str());
    fprintf(gnuplotPipe, "     '%s' using ($0+1):1 with lines linewidth 2 linecolor rgb 'red' title 'Column'\n", bestcsv.c_str());

    // マルチプロット終了
    fprintf(gnuplotPipe, "unset multiplot\n");

    // PNG保存
    fprintf(gnuplotPipe, "set output '%s.png'\n", realcsv.c_str());
    fprintf(gnuplotPipe, "set terminal pngcairo size 1200, 600\n");
    fprintf(gnuplotPipe, "replot\n");

    // 画面表示のために端末を戻す
    fprintf(gnuplotPipe, "set terminal qt size 1200, 600\n");

    fflush(gnuplotPipe); // コマンドを送信
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get(); // プログラム終了を待機
    _pclose(gnuplotPipe); // パイプを閉じる

    return 0;
}
