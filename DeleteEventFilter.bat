<span style="font-size:14px;">@ECHO OFF
TITLE WmiDeleteEvent
COLOR A
echo -----------WmiDeleteEvent-----------
powershell -command "Get-WMIObject -Namespace root\Subscription -Class __EventFilter -Filter \"Name='BotFilter82'\" | Remove-WmiObject -Verbose"
echo -----------------------------------
IPCONFIG
echo=
echo -----------------------------------
ARP?
echo=
echo -----------------------------------
PAUSE</span>
