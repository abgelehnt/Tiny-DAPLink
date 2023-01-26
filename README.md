# Tiny-DAPLink

> 小巧的使用CH552实现的DAPLink

详见[Tiny-DAPLink：开源CH552实现的CMSIS-DAP v2升级版](TODO)

## DAT命令列表
使用任意串口工具发送特定的命令即可对Tiny-DAPLink进行相应的设置。

| 命令        | 描述                                                                                           |
| :---------- |:---------------------------------------------------------------------------------------------|
| DAT+RST     | 重启调试器                                                                                        |
| DAT+IAP     | 进入调试器固件下载模式                                                                                  |
| DAT+KEY?    | 查询电容键盘映射的按键                                                                                  |
| DAT+KEY=XX  | 更改电容键盘映射的按键[USB CDC键盘键值表](https://github.com/abgelehnt/Tiny-DAPLink/blob/main/3.Docs/Keyboard_HID.txt) |
| DAT+KEY=FF  | 关闭电容键盘功能                                                                                     |
| DAT+AUTHOR? | 查询作者信息                                                                                       |
