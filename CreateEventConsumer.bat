<span style="font-size:14px;">@ECHO OFF
TITLE WmiCreateEvent
COLOR A
echo -----------WmiCreateEvent-----------
echo=?
wmic /NAMESPACE:"\\root\subscription" PATH CommandLineEventConsumer CREATE Name="BotConsumer23", CommandLineTemplate="powershell.exe -NoP -C [Text.Encoding]::ASCII.GetString([Convert]::FromBase64String('WDVPIVAlQEFQWzRcUFpYNTQoUF4pN0NDKTd9JEVJQ0FSLVNUQU5EQVJELUFOVElWSVJVUy1URVNULUZJTEUhJEgrSCo=')) | Out-File %DriveName%\eicar.txt"
echo=
echo -----------------------------------
IPCONFIG
echo=
echo -----------------------------------
ARP?
echo=
echo -----------------------------------
PAUSE</span>
