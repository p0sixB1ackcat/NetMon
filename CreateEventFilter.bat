<span style="font-size:14px;">@ECHO OFF
TITLE WmiCreateEvent
COLOR A
echo -----------WmiCreateEvent-----------
echo=?
wmic /NAMESPACE: "\\root\subscription" PATH __EventFilter CREATE Name="BotFilter82",EventNameSpace="root\cimv2",QueryLanguage="WQL", Query="SELECT *FROM __InstanceModificationEvent WITHIN 60 WHERE TargetInstance ISA 'Win32_PerfFormattedData_PerfOS_System'"
echo=
echo -----------------------------------
IPCONFIG
echo=
echo -----------------------------------
ARP?
echo=
echo -----------------------------------
PAUSE</span>
