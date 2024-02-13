//// 共通システムコード (予約済み) 127~
#define INS_BEGIN     0x7F // 命令送信ビット
#define DATA_BEGIN    0x80 // データ送信ビット
#define DATA_SEPARATE 0x81 // データ区切り
#define RES_OK        0x82 // 処理正常終了
#define RES_ERROR     0x83 // エラー発生時

//// CTRL送信用の命令コード (0x00と0xffは除外) 63~
//// 例：{0x7F, 0x40, 0x80, 0x01(データサイズ 1~255), 0x04(データ)}
////
//// データサイズは255が最大だが区切りを利用するとさらに多くのデータを送れます。
//// 例：{..., 0x80, 0xff, ..., 0x84, 0xff, ...}
#define DISP_CONNECT     0x3F // 接続開始
#define DISP_SET_PRESET  0x40 // プリセットを設定
#define DISP_SET_SYNTH   0x41 // シンセモード設定
#define DISP_SET_PAN     0x42 // パンを設定
#define DISP_RESET_SYNTH 0x43 // シンセをリセット

//// 共通シンセ演奏状態コード
#define SYNTH_SINGLE 0x00
#define SYNTH_OCTAVE 0x01
#define SYNTH_DUAL 0x02
#define SYNTH_MULTI  0x03

//// 共通シンセパン状態コード
#define LR_PAN_C 0x00
#define LR_PAN_L 0x01
#define LR_PAN_R 0x02