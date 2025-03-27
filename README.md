# 概要

これは、**後期印象派の色彩情報について解析**（docs内にある）を参考に、印象派と非印象派の違いを分析するアプリを開発し、実行解析を行ったものです。    

具体的には、**情報エントロピー**と**Jipf法則**を用いて、印象派と非印象派の作品における色彩情報の特徴的な違いを探りました。  
さらに、**CycleGAN** を利用して、モネの画像を学習させ、絵画を再現するモデルを作成しました。    

Gitに共有することを前提にして作っていないです。もうしわけない。また、コード内のパスなど私の環境になっています。

## ファイル構成

```
Impressionism_Zemi/ 
│── CycleGAN_data/                       # CycleGAN で使用したテストデータ
│ ├──Generated picture                   # 生成した画像
│ ├──picture                             # 生成する前の画像
│── data/                                # 画像データを解析し、CSV に変換したもの 
│── data_picture/                        # 実験で取得した元の画像データ 
│── docs/                                # 参考論文や関連資料
│ ├──ゼミ研最終発表_T122115.pptx          # Jipfメインで発表
│ ├──ゼミ研中間発表_T122115.pptx          # エントロピーメインで発表
│ ├──後期印象派の色彩情報について解析.pdf   # 参考論文
│ ├──解析データ.xlsx                      # 実験の詳細な結果 
│── jipf_result/                         # Jipf 法則解析の結果画像
│── python/                              # Python を用いた解析スクリプト
│ ├──cyclegan.ipynb
│ ├──jikkouGAN.ipynb
│── src/                                 # C++ で実装された解析コード 
│ ├── get.cpp                            # 色データ抽出 
│ ├── entoropi.cpp                       # 情報エントロピー解析 
│ ├── jipf.cpp                           # Jipf 法則解析 

│── README.md                            # 本ドキュメント
```

## セットアップ方法
### 1. インストールする外部ライブラリー
このコードを動かすにあたって、以下の外部ライブラリが必要です。
- **OpenCV**  
画像から、データを取得するのに使います
- **Boost**  
解析をする計算に使います

### 2.画像の取得方法
パブリックドメインの絵画を扱っているサイト。ここからダウンロードする。  
[https://www.wikiart.org](https://www.wikiart.org/)

### 2. 画像の取得と減色処理

解析のために、画像の色数を **256色** に減らす必要があります。  
以下の手順で **GIMP** を使用して減色を行います。

#### **GIMP を使った減色方法**
1. **GIMP** で画像を開く  
2. 上部メニューから  
   **`画像` → `モード` → `インデックスカラー`** を選択  
3. **最大色数を 256** に設定  
4. **OK を押して適用**  
5. **`ファイル` → `エクスポート` で保存**

これで、解析に適した256色の画像が得られます。  
実験で取得した画像はdata_pictureにある。

### 3. 画像からデータを取得

画像から色データを抽出し、使用頻度の高い順にCSVファイルへ保存します。  
この処理は、`src/get.cpp` を実行することで行えます。  
実験で使用した画像はdataにて全てcsvになっている

## 解析手順

### 1. 情報エントロピー解析

この解析では、画像の色の分布を基に **どれだけ複雑な配色になっているか** を調べます。  
エントロピーの値 **0 < N < 8** の範囲で、**8 に近いほど色が複雑である** ことを示します。  
この処理は、`src/entoropi.cpp` を実行することで行えます。

### 2. Jipf法則解析

この解析では、画像の色の分布がどれだけ法則性になっているか調べます。  
この解析では**モデルフィット**をして、どれだけJipf法則での**理論値**に沿っているかを調べます。  
より法則的な物は、よりモデルにフィットします。  
この処理は、`src/jipf.cpp` を実行することで行えます。  
この解析結果は、jipf_resultに例を載せてあります。  

解析データ.xlsxに全ての実験結果があります。ご覧ください。  
ただし、これは公開用に作られていないので見にくいです。  
**発表用のパワーポイント**では、詳しくより分かりやすく説明されているので、ぜひそちらを

## 考察(CycleGAN)

### 1. 実行方法
参考サイト
このサイトは、CycleGANを理解するために、作られたものです。  
馬をゼブラにするコードを変更して、モネと画像に変えた。  
データは、参考サイトから取得していて、学習データを自分のgooglecloudに保存する。  
そうすることで、学習を複数回にして行った。  
[https://colab.research.google.com/github/tensorflow/docs-l10n/blob/master/site/ja/tutorials/generative/cyclegan.ipynb](https://colab.research.google.com/github/tensorflow/docs-l10n/blob/master/site/ja/tutorials/generative/cyclegan.ipynb)

結果はCycleGAN_dataここにあります。

### 2. 結果

この CycleGAN を使うことで、見た目はモネのような画風になったと思う。  
色が鮮やかになり、輪郭が滑らかになったなどの変化が見られた。  

今回行った解析をすることで、数値的に、印象派と非印象派では違った傾向が表れた。  
では CycleGAN で変化した画像には、印象派傾向が表れるのだろうか？  
もし傾向が表れるなら、印象派を生成した画像の完成度を表す指標になるのではないか。    

<div align="center">
  <img src="https://github.com/user-attachments/assets/420a6742-431b-4ca5-a7a8-e1ecfcbf3b3f" width="70%">
</div>

この結果から、やはり印象派風に変換すると、画像の数値が印象派の傾向に似た方向になった。  
このことから、**印象派を生成した画像の完成度を表す指標になり得る可能性がある** といえる。


