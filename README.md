# NetMon
1、AntiyMonDrv
进程监控驱动，涉及到商业化，并没有太多东西。

2、AntiyMonitor-MFC
(1) 网络监控部分代码，实现方案是通过应用层注册etw事件回调。基本还原了Sysmon的逻辑。
(2) wmi监控

3、AddToResourceTool
编译期脚本运行程序，实现将AntiyMonDrv.sys添加到应用程序的资源节中。
