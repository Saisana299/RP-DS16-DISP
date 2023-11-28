import cv2
import numpy as np

isMaintain = True

def convert_to_binary(frame, maintain_aspect_ratio=True):
    # ターゲットサイズの設定
    target_size = (128, 64)
    h, w, _ = frame.shape

    if maintain_aspect_ratio:
        # アスペクト比を維持してリサイズ
        aspect_ratio = w / h
        resized_width = int(target_size[1] * aspect_ratio)
        resized_frame = cv2.resize(frame, (resized_width, target_size[1]))
        padding = target_size[0] - resized_width
        left_padding = padding // 2
        right_padding = padding - left_padding
        padded_frame = cv2.copyMakeBorder(resized_frame, 0, 0, left_padding, right_padding, cv2.BORDER_CONSTANT, value=[0, 0, 0])
    else:
        # アスペクト比を無視してリサイズ
        padded_frame = cv2.resize(frame, (target_size[0], target_size[1]))

    # グレースケールに変換
    gray_frame = cv2.cvtColor(padded_frame, cv2.COLOR_BGR2GRAY)

    # 2値化（白=1、黒=0）
    _, binary_frame = cv2.threshold(gray_frame, 128, 1, cv2.THRESH_BINARY)

    return binary_frame.flatten()

def run_length_encode(data):
    encoded_data = []
    current_value = data[0]
    count = 1

    for value in data[1:]:
        if value == current_value:
            count += 1
        else:
            if count > 1:
                encoded_data.append(f"{current_value}:{count}")
            else:
                encoded_data.append(str(current_value))
            current_value = value
            count = 1

    if count > 1:
        encoded_data.append(f"{current_value}:{count}")
    else:
        encoded_data.append(str(current_value))

    return encoded_data

def process_image(input_path, output_path):
    # 画像の読み込み
    frame = cv2.imread(input_path)

    # 画像を2値データに変換
    binary_frame = convert_to_binary(frame, maintain_aspect_ratio=isMaintain)

    # ランレングス圧縮
    compressed_data = run_length_encode(binary_frame)

    # 2値データをCSVに保存
    np.savetxt(output_path, [compressed_data], delimiter=",", fmt="%s")

if __name__ == "__main__":
    # 入力画像と出力CSVのパス指定
    input_image_path = "convert/input_image.png"
    output_csv_path = "convert/output_data.csv"
    
    # 画像処理の実行
    process_image(input_image_path, output_csv_path)
