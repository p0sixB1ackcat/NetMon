<span style="font-size:14px;">@ECHO OFF
TITLE WmiCreateEvent
COLOR A
echo -----------WmiCreateEvent-----------
echo=?
wmic /NAMESPACE:"\\root\subscription" PATH CommandLineEventConsumer WHERE Name="BotConsumer23" DELETE
echo=
echo -----------------------------------
IPCONFIG
echo=
echo -----------------------------------
ARP?
echo=
echo -----------------------------------
PAUSE</span>
