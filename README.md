# Embedding-System-Project: Pong game
## Video demo

## Các thiết bị phần cứng
- 1 ESP32
- 1 loa 
- 1 module màn hình OLED SSD1306
- 1 loa $8\Ohm$ 0.5 W
- Điện trở $200\Ohm$
- 4 nút bấm
- Module thẻ nhớ SD/microSD

### Sơ đồ nối mạch


### mô phỏng trên wokwi


## Code
### Thư viện
- Thư viện đồ họa Adafruit_GFX
- Thư viện điều khiển OLED Adafruit_SSD1306
- Thư viện Wire cho kết nối I2C
- Thư viện SD đọc thẻ nhớ

### Các thành phần
- Bóng (ball): 1 pixel trên màn hình
- 2 paddle tương ứng của 2 người chơi: 2 đoạn thẳng dọc ở 2 cột x = 12 và x = 115
- Sân (court): hình chữ nhật lớn, kích thước 128*54 pixel

### Di chuyển của các thành phần
Ở mỗi thời điểm, nếu đến lượt update vị trí, độ thay đổi của các vật thể sẽ được tính:
- Độ thay đổi của bóng: luân phiên đổi dấu khi đập vào tường hoặc paddle
- Độ thay đổi của paddle: phụ thuộc vào giá trị đọc vào từ các nút bấm

### Vòng lặp game
Trong 1 vòng lặp, vi xử lý sẽ thực hiện:
- Kiểm tra bóng có đập vào tường (ngang, dọc) hay paddle không để tính độ thay đổi
- Cập nhật vị trí của bóng theo độ thay đổi
- Đọc trạng thái của các nút bấm để tính độ thay đổi của paddle
- Cập nhật vị trí của paddle theo độ thay đổi
- Cập nhật điểm nếu cần thiết


### Loa
Loa sẽ phát các âm thanh khác nhau trong các trường hợp:
- Bóng đập tường
- Bóng đập paddle
- Ghi điểm
- Kết thúc game (có 1 người giành chiến thắng)

Lựa chọn đổi nhạc kết thúc game bằng cách nhấn giữ cả 4 nút trong 2s, ấn nút lên của người chơi 2 để đổi nhạc và nút xuống để chọn.