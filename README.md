# Tiny-DAPLink

> 小巧的使用CH552实现的DAPLink

详见[Tiny-DAPLink：开源CH552实现的CMSIS-DAP v2升级版](https://whycan.com/t_9365.html)

## 硬件设计

1. 下载按键需要超级小的两脚小龟按键，如果不重复下载调试，可以不焊
2. 整体需要包热缩管进行绝缘，不然手汗会造成数据异常

## DAT命令列表

使用任意串口工具发送特定的命令即可对Tiny-DAPLink进行相应的设置。

| 命令            | 描述                                                 |
|:--------------|:---------------------------------------------------|
| DAT+RST       | 重启调试器                                              |
| DAT+IAP       | 进入调试器固件下载模式                                        |
| DAT+KEY?      | 查询电容键盘映射的按键                                        |
| DAT+KEY=XX    | 更改电容键盘映射的按键[USB CDC键盘键值表](3.Docs/Keyboard_HID.txt) |
| DAT+KEY=FF    | 关闭电容键盘功能                                           |
| DAT+AUTHOR?   | 查询作者信息                                             |
| DAT+KEY_FLASH | 刷新电容按键参数                                           |
| DAT+KEY_VALUE | 查看电容按键参数                                           |

## 注意事项

1. 一些不合格的数据线会使Type C接口外壳对地短路，使电容按键失效