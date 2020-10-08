# battery_metter

R1 = 4.7k
R2 = 1k

 ==> 1/(4.7+1) = 1/ 5.7

VDC = adc_value* 3.3 / 4095 / *5.7

Start serial port 115200

Setting alarm max vaulue

setting-max
15.0 

setting-min
11.5


functions:
 Nut nhan B0 ==> Chọn chế độ manual / auto
* Chế độ auto , 
 khi que đo vào 12VDC thì chip sẽ tự động nhận biết và đọc trong 1s và gửi kết quả lên máy tính
 Nhấn que đo ra để có thể đo lần tiếp theo
 * Chế độ manual
 Mỗi khi nhấn nút B1 thì thiết bị sẽ thực hiện một lần đo và gửi kết quả lên máy tính