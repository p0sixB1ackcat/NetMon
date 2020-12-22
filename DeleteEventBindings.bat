<span style="font-size:14px;">@ECHO OFF
TITLE WmiCreateEvent
COLOR A
echo -----------WmiCreateEvent-----------
echo=?
wmic /NAMESPACE:"\\root\subscription" PATH __FilterToConsumerBinding WHERE Filter="__EventFilter.Name='BotFilter82'" DELETE
echo=
echo -----------------------------------
IPCONFIG
echo=
echo -----------------------------------
ARP?
echo=
echo -----------------------------------
PAUSE</span>
